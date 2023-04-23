/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#ifdef DEBUG
#include <dumper.hpp>
#endif

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;

NewLock(ExecuteServiceLock);

namespace Execute
{
    Memory::MemMgr *mem = nullptr;
    std::vector<SharedLibraries> Libs;

    void StartExecuteService()
    {
        mem = new Memory::MemMgr;

        while (true)
        {
            {
                SmartLock(ExecuteServiceLock);
                foreach (auto &Lib in Libs)
                {
                    if (Lib.RefCount > 0)
                    {
                        Lib.Timeout = TimeManager->CalculateTarget(10, Time::Units::Minutes);
                        debug("Reset timeout for %s", Lib.Identifier);
                        continue;
                    }
                    if (Lib.Timeout < TimeManager->GetCounter())
                    {
                        // TODO: Remove
                        fixme("Removed library %s because of timeout", Lib.Identifier);
                    }
                    else
                        debug("Timeout for %s is %ld", Lib.Identifier, Lib.Timeout);
                }
                debug("Waiting 10 seconds...");
            }
            TaskManager->Sleep(10000);
        }
    }

    bool AddLibrary(char *Identifier, void *ElfImage, size_t Length, const Memory::Virtual &pV)
    {
        SmartLock(ExecuteServiceLock);
        SharedLibraries sl;

        foreach (auto lib in Libs)
        {
            if (strcmp(lib.Identifier, Identifier) == 0)
            {
                debug("Library %s already loaded", Identifier);
                lib.RefCount++;
                return true;
            }
        }

        strcpy(sl.Identifier, Identifier);
        sl.Timeout = TimeManager->CalculateTarget(10, Time::Units::Minutes);
        sl.RefCount = 0;

        void *LibFile = mem->RequestPages(TO_PAGES(Length + 1), true);
        debug("LibFile: %#lx", LibFile);
        memcpy(LibFile, (void *)ElfImage, Length);
        Memory::Virtual().Map(LibFile, LibFile, Length, Memory::RW | Memory::US | Memory::G);

        Memory::Virtual ncpV = pV;
        sl.MemoryImage = r_cst(uint64_t, ELFCreateMemoryImage(mem, ncpV, LibFile, Length).Phyiscal);
        debug("MemoryImage: %#lx", sl.MemoryImage);

        {
            uintptr_t BaseAddress = UINTPTR_MAX;
            Elf64_Phdr ItrProgramHeader;

            for (Elf64_Half i = 0; i < ((Elf64_Ehdr *)LibFile)->e_phnum; i++)
            {
                memcpy(&ItrProgramHeader, (uint8_t *)LibFile + ((Elf64_Ehdr *)LibFile)->e_phoff + ((Elf64_Ehdr *)LibFile)->e_phentsize * i, sizeof(Elf64_Phdr));
                BaseAddress = MIN(BaseAddress, ItrProgramHeader.p_vaddr);
            }

            for (Elf64_Half i = 0; i < ((Elf64_Ehdr *)LibFile)->e_phnum; i++)
            {
                memcpy(&ItrProgramHeader, (uint8_t *)LibFile + ((Elf64_Ehdr *)LibFile)->e_phoff + ((Elf64_Ehdr *)LibFile)->e_phentsize * i, sizeof(Elf64_Phdr));
                if (ItrProgramHeader.p_type != PT_LOAD)
                    continue;

                debug("PT_LOAD - Offset: %#lx, VirtAddr: %#lx, FileSiz: %ld, MemSiz: %ld, Align: %#lx",
                      ItrProgramHeader.p_offset, ItrProgramHeader.p_vaddr,
                      ItrProgramHeader.p_filesz, ItrProgramHeader.p_memsz, ItrProgramHeader.p_align);
                uintptr_t MAddr = (ItrProgramHeader.p_vaddr - BaseAddress) + (uintptr_t)sl.MemoryImage;
                fixme("Address: %#lx %s%s%s", MAddr,
                      (ItrProgramHeader.p_flags & PF_R) ? "R" : "",
                      (ItrProgramHeader.p_flags & PF_W) ? "W" : "",
                      (ItrProgramHeader.p_flags & PF_X) ? "X" : "");

                memcpy((void *)MAddr, (uint8_t *)LibFile + ItrProgramHeader.p_offset, ItrProgramHeader.p_filesz);
                debug("memcpy: %#lx => %#lx (%ld bytes)", (uint8_t *)LibFile + ItrProgramHeader.p_offset, (uintptr_t)MAddr, ItrProgramHeader.p_filesz);
                break;
            }

            struct Elf64_Dyn *JmpRel = ELFGetDynamicTag((void *)LibFile, DT_JMPREL);
            struct Elf64_Dyn *SymTab = ELFGetDynamicTag((void *)LibFile, DT_SYMTAB);
            struct Elf64_Dyn *StrTab = ELFGetDynamicTag((void *)LibFile, DT_STRTAB);

            if (!JmpRel)
            {
                debug("No DT_JMPREL");
            }

            if (!SymTab)
            {
                debug("No DT_SYMTAB");
            }

            if (!StrTab)
            {
                debug("No DT_STRTAB");
            }

            if (JmpRel && SymTab && StrTab)
            {
                debug("JmpRel: %#lx, SymTab: %#lx, StrTab: %#lx", JmpRel->d_un.d_ptr, SymTab->d_un.d_ptr, StrTab->d_un.d_ptr);
                Elf64_Rela *_JmpRel = (Elf64_Rela *)(sl.MemoryImage + (JmpRel->d_un.d_ptr - BaseAddress));
                Elf64_Sym *_SymTab = (Elf64_Sym *)(sl.MemoryImage + (SymTab->d_un.d_ptr - BaseAddress));

                char *_DynStr = (char *)(sl.MemoryImage + (StrTab->d_un.d_ptr - BaseAddress));

                Elf64_Shdr *gotSection = nullptr;
                for (Elf64_Half i = 0; i < ((Elf64_Ehdr *)LibFile)->e_shnum; i++)
                {
                    Elf64_Shdr *shdr = (Elf64_Shdr *)((uint8_t *)LibFile + ((Elf64_Ehdr *)LibFile)->e_shoff + i * sizeof(Elf64_Shdr));
                    if (shdr->sh_type == SHT_PROGBITS && (shdr->sh_flags & SHF_WRITE) && (shdr->sh_flags & SHF_ALLOC))
                    {
                        gotSection = shdr;
                        break;
                    }
                }

                debug("LIB_DBG");

                if (gotSection)
                {
                    Elf64_Xword numEntries = gotSection->sh_size / sizeof(Elf64_Addr);
                    for (Elf64_Xword i = 0; i < numEntries - 3; i++)
                    {
                        Elf64_Rela *Rel = _JmpRel + i;
                        Elf64_Addr *GOTEntry = (Elf64_Addr *)(Rel->r_offset + sl.MemoryImage);

                        Elf64_Xword RelType = ELF64_R_TYPE(Rel->r_info);
                        debug("r_offset: %#lx RelType: %d", Rel->r_offset, RelType);

                        switch (RelType)
                        {
                        case R_X86_64_NONE:
                            break;
                        case R_X86_64_JUMP_SLOT:
                        {
                            Elf64_Xword SymIndex = ELF64_R_SYM(Rel->r_info);
                            Elf64_Sym *Sym = _SymTab + SymIndex;

                            if (Sym->st_name)
                            {
                                char *SymName = _DynStr + Sym->st_name;
                                debug("SymName: %s", SymName);

                                Elf64_Sym *LibSym = ELFLookupSymbol((Elf64_Ehdr *)LibFile, SymName);

                                if (LibSym)
                                {
                                    *GOTEntry = (Elf64_Addr)(sl.MemoryImage + LibSym->st_value);
                                    debug("GOT[%ld]: %#lx + %#lx = %#lx", i, sl.MemoryImage, LibSym->st_value, *GOTEntry);
                                }
                            }
                            break;
                        }
                        default:
                        {
                            fixme("RelType %d not supported", RelType);
                            break;
                        }
                        }

                        debug("GOT[%ld](%#lx): %#lx", i, GOTEntry, *GOTEntry);
                    }
                }
                else
                    debug("GOT section not found");
            }
        }

        sl.Address = r_cst(uint64_t, LibFile);
        debug("Casted LibFile %#lx -> %#lx", LibFile, sl.Address);
        sl.Length = Length;

        debug("Library %s loaded at %#lx (full file: %#lx)", Identifier, sl.MemoryImage, LibFile);

        Libs.push_back(sl);
        return true;
    }

    void SearchLibrary(char *Identifier)
    {
        SmartLock(ExecuteServiceLock);
    }

    SharedLibraries GetLibrary(char *Identifier)
    {
        SmartLock(ExecuteServiceLock);
        foreach (auto Lib in Libs)
        {
            if (strcmp(Lib.Identifier, Identifier) == 0)
            {
                Lib.RefCount++;
                debug("Library %s found (%#lx %#lx)", Identifier, Lib.Address, Lib.MemoryImage);
                return Lib;
            }
        }
        // throw std::runtime_error("Library not found");
        return SharedLibraries();
    }
}
