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
        return;

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

    SharedLibraries *AddLibrary(char *Identifier, void *LibraryImage, size_t Length)
    {
        SmartLock(ExecuteServiceLock);
        SharedLibraries sl;

        strcpy(sl.Identifier, Identifier);
        sl.Timeout = TimeManager->CalculateTarget(600000); /* 10 minutes */
        sl.RefCount = 0;

        void *BaseLibImage = mem->RequestPages(TO_PAGES(Length));
        memcpy(BaseLibImage, (void *)LibraryImage, Length);
        sl.Address = BaseLibImage;
        sl.Length = Length;

        debug("Library %s loaded at %#lx", Identifier, BaseLibImage);

        Libs.push_back(sl);
        return &Libs[Libs.size() - 1];
    }

    void SearchLibrary(char *Identifier)
    {
        SmartLock(ExecuteServiceLock);
    }

    void AttachLibrary(SharedLibraries *Lib, void *BaseImage)
    {
        SmartLock(ExecuteServiceLock);

        BinaryType Type = GetBinaryType(BaseImage);
        switch (Type)
        {
        case BinaryType::BinTypeFex:
        {
            fixme("Fex is not supported yet");
            return;
        }
        case BinaryType::BinTypeELF:
        {
            Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)BaseImage;
            uintptr_t BaseAddress = UINTPTR_MAX;
            size_t ElfAppSize = 0;
            Elf64_Phdr ItrProgramHeader;

            Elf64_Shdr *ElfSections = (Elf64_Shdr *)((uintptr_t)BaseImage + ELFHeader->e_shoff);
            Elf64_Shdr *Dynamic = nullptr;
            Elf64_Shdr *DynamicSymbol = nullptr;
            Elf64_Shdr *DynamicString = nullptr;
            Elf64_Shdr *SymbolTable = nullptr;
            Elf64_Shdr *StringTable = nullptr;
            Elf64_Shdr *RelaPlt = nullptr;
            Elf64_Shdr *GotPlt = nullptr;
            size_t SymbolCount = 0;

            size_t GOTSize = 0;
            Elf64_Addr *GOTEntry = 0;

            uintptr_t RelaOffset = 0;
            uint64_t RelaEnt = 0;
            size_t RelaSize = 0;

            for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
            {
                memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
                BaseAddress = MIN(BaseAddress, ItrProgramHeader.p_vaddr);
            }

            for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
            {
                memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
                uintptr_t SegmentEnd;
                SegmentEnd = ItrProgramHeader.p_vaddr - BaseAddress + ItrProgramHeader.p_memsz;
                ElfAppSize = MAX(ElfAppSize, SegmentEnd);

                for (Elf64_Half i = 0; i < ELFHeader->e_shnum; i++)
                {
                    char *DynamicStringTable = (char *)((uintptr_t)BaseImage + ElfSections[ELFHeader->e_shstrndx].sh_offset + ElfSections[i].sh_name);

                    if (strcmp(DynamicStringTable, ".dynamic") == 0)
                    {
                        Dynamic = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".dynsym") == 0)
                    {
                        DynamicSymbol = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".dynstr") == 0)
                    {
                        DynamicString = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".strtab") == 0)
                    {
                        StringTable = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".rela.plt") == 0)
                    {
                        RelaPlt = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".got.plt") == 0)
                    {
                        GotPlt = &ElfSections[i];
                    }
                    else if (strcmp(DynamicStringTable, ".symtab") == 0)
                    {
                        SymbolTable = &ElfSections[i];
                    }
                }

                if (ItrProgramHeader.p_type == PT_DYNAMIC)
                {
                    Elf64_Dyn *Dynamic = (Elf64_Dyn *)((uint8_t *)BaseImage + ItrProgramHeader.p_offset);

                    for (uint64_t i = 0; i < ItrProgramHeader.p_filesz / sizeof(Elf64_Dyn); i++)
                    {
                        switch (Dynamic[i].d_tag)
                        {
                        case DT_PLTRELSZ:
                        {
                            GOTSize = Dynamic[i].d_un.d_val;
                            break;
                        }
                        case DT_PLTGOT:
                        {
                            GOTEntry = (Elf64_Addr *)Dynamic[i].d_un.d_ptr;
                            break;
                        }
                        case DT_RELA:
                        {
                            RelaOffset = Dynamic[i].d_un.d_ptr;
                            break;
                        }
                        case DT_RELASZ:
                        {
                            RelaSize = Dynamic[i].d_un.d_val;
                            break;
                        }
                        case DT_RELAENT:
                        {
                            RelaEnt = Dynamic[i].d_un.d_val;
                            break;
                        }
                        default:
                            break;
                        }

                        if (Dynamic[i].d_tag == DT_NULL)
                            break;
                    }
                    break;
                }
            }

            debug("BaseAddress: %#lx Size: %ld", BaseAddress, ElfAppSize);

            if (RelaOffset != 0)
            {
                if (RelaEnt != sizeof(Elf64_Rela))
                {
                    error("RelaEnt != sizeof(Elf64_Rela)");
                    /* I should exit here I guess... */
                }
                else
                {
                    for (size_t RelaOffsetItr = 0; RelaOffsetItr < RelaSize; RelaOffsetItr += RelaEnt)
                    {
                        Elf64_Rela *Rela = (Elf64_Rela *)(((char *)BaseImage) + RelaOffset + RelaOffsetItr);

                        switch (Rela->r_info)
                        {
                        case R_X86_64_RELATIVE:
                        {
                            uintptr_t *Ptr = (uintptr_t *)((uintptr_t)BaseImage + Rela->r_offset);
                            *Ptr = (uintptr_t)Lib->Address + Rela->r_addend;
                            break;
                        }
                        default:
                            fixme("Rela: %ld", Rela->r_info);
                            break;
                        }
                    }
                }
            }
            else
                debug("No Rela");

            if (DynamicSymbol != nullptr)
                SymbolCount = DynamicSymbol->sh_size / sizeof(Elf64_Sym);
            else if (SymbolTable != nullptr)
                SymbolCount = SymbolTable->sh_size / sizeof(Elf64_Sym);

            debug("GOT Address %#lx Size %#lx Entry %#lx",
                  GOTEntry, GOTSize, GOTEntry ? GOTEntry : 0);

#ifdef DEBUG
            DumpData("Old GOT", (void *)GOTEntry, GOTSize);

            if (DynamicSymbol && DynamicString)
                for (size_t i = 0; i < SymbolCount; i++)
                {
                    Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)BaseImage + DynamicSymbol->sh_offset + i * sizeof(Elf64_Sym));
                    char *SymbolName = (char *)((uintptr_t)BaseImage + DynamicString->sh_offset + Symbol->st_name);
                    if (GOTEntry)
                        if (GOTEntry[i])
                        {
                            uintptr_t SymbolAddress = GOTEntry[i];
                            debug("New GOTEntry[%d] - Symbol %s Address %#lx", i, SymbolName, SymbolAddress);
                        }
                }

            for (size_t i = 0; i < GOTSize; i++)
                if (GOTEntry)
                    if (GOTEntry[i])
                        debug("GOTEntry[%d] = %#lx", i, GOTEntry[i]);
#endif

            GOTEntry[1] = (uintptr_t)BaseImage;
            GOTEntry[2] = (uintptr_t)ElfLazyResolver;

            if (DynamicSymbol && DynamicString && GOTEntry)
                for (size_t i = 0; i < SymbolCount; i++)
                {
                    Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)BaseImage + DynamicSymbol->sh_offset + i * sizeof(Elf64_Sym));
                    char *SymbolName = (char *)((uintptr_t)BaseImage + DynamicString->sh_offset + Symbol->st_name);

                    switch (ELF64_ST_TYPE(Symbol->st_info))
                    {
                    case STT_OBJECT:
                        fixme("STT_OBJECT");
                    case STT_FUNC:
                    {
                        uintptr_t SymbolAddress = (uintptr_t)ELFLookupSymbol((Elf64_Ehdr *)Lib->Address, SymbolName);
                        if (SymbolAddress == 0)
                        {
                            error("Symbol %s not found", SymbolName);
                            continue;
                        }
                        GOTEntry[i] = (uintptr_t)Lib->Address + SymbolAddress;
                        debug("%d %#lx Symbol %s at %#lx (%#lx)", i, &GOTEntry[i], SymbolName, SymbolAddress, (uintptr_t)Lib->Address + SymbolAddress);
                        break;
                    }
                    case STT_NOTYPE:
                        break;
                    default:
                        error("Unsupported symbol type %d", ELF64_ST_TYPE(Symbol->st_info));
                        break;
                    }
                }

#ifdef DEBUG
            DumpData("New GOT", (void *)GOTEntry, GOTSize);

            if (DynamicSymbol && DynamicString)
                for (size_t i = 0; i < SymbolCount; i++)
                {
                    Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)BaseImage + DynamicSymbol->sh_offset + i * sizeof(Elf64_Sym));
                    char *SymbolName = (char *)((uintptr_t)BaseImage + DynamicString->sh_offset + Symbol->st_name);
                    if (GOTEntry)
                        if (GOTEntry[i])
                        {
                            uintptr_t SymbolAddress = GOTEntry[i];
                            debug("New GOTEntry[%d] - Symbol %s Address %#lx", i, SymbolName, SymbolAddress);
                        }
                }

            for (size_t i = 0; i < GOTSize; i++)
                if (GOTEntry)
                    if (GOTEntry[i])
                        debug("GOTEntry[%d] = %#lx", i, GOTEntry[i]);
#endif

            break;
        }
        default:
        {
            fixme("Unsupported binary type %d", Type);
            return;
        }
        }

        Lib->RefCount++;
        debug("Attached library %s", Lib->Identifier);
    }
}
