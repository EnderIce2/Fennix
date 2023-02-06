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
    Vector<SharedLibraries> Libs;

    void StartExecuteService()
    {
        mem = new Memory::MemMgr;
        // return;

        while (true)
        {
            ExecuteServiceLock.Lock(__FUNCTION__);
            foreach (auto &Lib in Libs)
            {
                if (Lib.RefCount > 0)
                {
                    Lib.Timeout = TimeManager->CalculateTarget(600000);
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
            ExecuteServiceLock.Unlock();
            TaskManager->Sleep(10000);
        }
    }

    SharedLibraries *AddLibrary(char *Identifier, void *ElfImage, size_t Length, const Memory::Virtual &pV)
    {
        SmartLock(ExecuteServiceLock);
        SharedLibraries sl;

        strcpy(sl.Identifier, Identifier);
        sl.Timeout = TimeManager->CalculateTarget(600000); /* 10 minutes */
        sl.RefCount = 0;

        void *LibFile = mem->RequestPages(TO_PAGES(Length), true);
        memcpy(LibFile, (void *)ElfImage, Length);

        Memory::Virtual ncpV = pV;
        sl.MemoryImage = ELFCreateMemoryImage(mem, ncpV, LibFile, Length);

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
        }

        sl.Address = LibFile;
        sl.Length = Length;

        debug("Library %s loaded at %#lx (full file: %#lx)", Identifier, sl.MemoryImage, LibFile);

        Libs.push_back(sl);
        return &Libs[Libs.size() - 1];
    }

    void SearchLibrary(char *Identifier)
    {
        SmartLock(ExecuteServiceLock);
    }
}
