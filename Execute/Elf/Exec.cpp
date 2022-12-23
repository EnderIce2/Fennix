#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;

namespace Execute
{
    void ELFLoadExec(void *BaseImage,
                     Elf64_Ehdr *ELFHeader,
                     Memory::Virtual &pva,
                     SpawnData *ret,
                     char *Path,
                     Tasking::PCB *Process,
                     const char **argv,
                     const char **envp,
                     Tasking::TaskArchitecture Arch,
                     Tasking::TaskCompatibility Comp)
    {
        trace("Executable");
        Elf64_Phdr *ProgramHeader = (Elf64_Phdr *)(((char *)BaseImage) + ELFHeader->e_phoff);
        debug("p_paddr: %#lx | p_vaddr: %#lx | p_filesz: %#lx | p_memsz: %#lx | p_offset: %#lx", ProgramHeader->p_paddr, ProgramHeader->p_vaddr, ProgramHeader->p_filesz, ProgramHeader->p_memsz, ProgramHeader->p_offset);

        uintptr_t BaseAddress = UINTPTR_MAX;
        uint64_t ElfAppSize = 0;

        Elf64_Phdr ItrProgramHeader;
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
            BaseAddress = MIN(BaseAddress, ItrProgramHeader.p_vaddr);
        }
        debug("BaseAddress %#lx", BaseAddress);

        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
            uintptr_t SegmentEnd;
            SegmentEnd = ItrProgramHeader.p_vaddr - BaseAddress + ItrProgramHeader.p_memsz;
            ElfAppSize = MAX(ElfAppSize, SegmentEnd);
        }
        debug("ElfAppSize %ld", ElfAppSize);

        uint8_t *MemoryImage = nullptr;

        // check for TEXTREL
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
            if (ItrProgramHeader.p_type == DT_TEXTREL)
            {
                warn("TEXTREL ELF is not fully tested yet!");
                MemoryImage = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(ElfAppSize));
                memset(MemoryImage, 0, ElfAppSize);
                for (uint64_t i = 0; i < TO_PAGES(ElfAppSize); i++)
                {
                    pva.Remap((void *)((uintptr_t)MemoryImage + (i * PAGE_SIZE)), (void *)((uintptr_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
                    debug("Mapping: %#lx -> %#lx", (uintptr_t)MemoryImage + (i * PAGE_SIZE), (uintptr_t)MemoryImage + (i * PAGE_SIZE));
                }
                break;
            }
        }

        if (!MemoryImage)
        {
            debug("Allocating %ld pages for image", TO_PAGES(ElfAppSize));
            MemoryImage = (uint8_t *)KernelAllocator.RequestPages(TO_PAGES(ElfAppSize));
            memset(MemoryImage, 0, ElfAppSize);
            for (uint64_t i = 0; i < TO_PAGES(ElfAppSize); i++)
            {
                uintptr_t Address = (uintptr_t)ProgramHeader->p_vaddr;
                Address &= 0xFFFFFFFFFFFFF000;
                pva.Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uintptr_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
                debug("Mapping: %#lx -> %#lx", (uintptr_t)Address + (i * PAGE_SIZE), (uintptr_t)MemoryImage + (i * PAGE_SIZE));
            }
        }

        debug("BaseAddress: %#lx | ElfAppSize: %#lx (%ld, %ld KB)", BaseAddress, ElfAppSize, ElfAppSize, TO_KB(ElfAppSize));

        debug("Solving symbols for address: %#llx", (uintptr_t)BaseImage);
        Elf64_Shdr *ElfSections = (Elf64_Shdr *)((uintptr_t)BaseImage + ELFHeader->e_shoff);
        Elf64_Shdr *Dynamic = nullptr;
        Elf64_Shdr *DynamicSymbol = nullptr;
        Elf64_Shdr *DynamicString = nullptr;
        Elf64_Shdr *SymbolTable = nullptr;
        Elf64_Shdr *StringTable = nullptr;
        Elf64_Shdr *RelaPlt = nullptr;

        for (Elf64_Half i = 0; i < ELFHeader->e_shnum; i++)
        {
            char *DynamicStringTable = (char *)((uintptr_t)BaseImage + ElfSections[ELFHeader->e_shstrndx].sh_offset + ElfSections[i].sh_name);

            if (strcmp(DynamicStringTable, ".dynamic") == 0)
            {
                Dynamic = &ElfSections[i];
                debug("Found .dynamic");
            }
            else if (strcmp(DynamicStringTable, ".dynsym") == 0)
            {
                DynamicSymbol = &ElfSections[i];
                debug("Found .dynsym");
            }
            else if (strcmp(DynamicStringTable, ".dynstr") == 0)
            {
                DynamicString = &ElfSections[i];
                debug("Found .dynstr");
            }
            else if (strcmp(DynamicStringTable, ".strtab") == 0)
            {
                StringTable = &ElfSections[i];
                debug("Found .strtab");
            }
            else if (strcmp(DynamicStringTable, ".rela.plt") == 0)
            {
                RelaPlt = &ElfSections[i];
                debug("Found .rela.plt");
            }
            else if (strcmp(DynamicStringTable, ".symtab") == 0)
            {
                SymbolTable = &ElfSections[i];
                debug("Found .symtab");
            }
            else
            {
                debug("Unknown section: %s", DynamicStringTable);
            }
        }

        UNUSED(Dynamic);
        UNUSED(DynamicSymbol);
        UNUSED(SymbolTable);
        UNUSED(RelaPlt);

        if (!DynamicString)
            DynamicString = StringTable;

        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
            uintptr_t MAddr;

            switch (ItrProgramHeader.p_type)
            {
            case PT_NULL:
                fixme("PT_NULL");
                break;
            case PT_LOAD:
            {
                debug("PT_LOAD - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrProgramHeader.p_offset, ItrProgramHeader.p_vaddr,
                      ItrProgramHeader.p_filesz, ItrProgramHeader.p_memsz, ItrProgramHeader.p_align);
                MAddr = (ItrProgramHeader.p_vaddr - BaseAddress) + (uintptr_t)MemoryImage;
                debug("MAddr: %#lx", MAddr);

                memcpy((void *)MAddr, (uint8_t *)BaseImage + ItrProgramHeader.p_offset, ItrProgramHeader.p_filesz);
                debug("memcpy operation: %#lx to %#lx for length %ld", (uint8_t *)BaseImage + ItrProgramHeader.p_offset, MemoryImage + MAddr, ItrProgramHeader.p_filesz);
                break;
            }
            case PT_DYNAMIC:
            {
                debug("PT_DYNAMIC - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrProgramHeader.p_offset, ItrProgramHeader.p_vaddr,
                      ItrProgramHeader.p_filesz, ItrProgramHeader.p_memsz, ItrProgramHeader.p_align);

                Elf64_Dyn *Dynamic = (Elf64_Dyn *)((uint8_t *)BaseImage + ItrProgramHeader.p_offset);

                char *NeededLibraries[256];
                uint64_t InitAddress = 0;
                uint64_t FiniAddress = 0;

                for (uint64_t i = 0; i < ItrProgramHeader.p_filesz / sizeof(Elf64_Dyn); i++)
                {
                    switch (Dynamic[i].d_tag)
                    {
                    case DT_NULL:
                        debug("DT_NULL");
                        break;
                    case DT_NEEDED:
                    {
                        if (!DynamicString)
                        {
                            error("DynamicString is null");
                            break;
                        }

                        debug("DT_NEEDED - Name[%ld]: %s", i, (uintptr_t)BaseImage + DynamicString->sh_offset + Dynamic[i].d_un.d_ptr);
                        NeededLibraries[i] = (char *)((uintptr_t)BaseImage + DynamicString->sh_offset + Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_PLTRELSZ:
                    {
                        fixme("DT_PLTRELSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_PLTGOT:
                    {
                        fixme("DT_PLTGOT - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_HASH:
                    {
                        fixme("DT_HASH - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_STRTAB:
                    {
                        fixme("DT_STRTAB - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_SYMTAB:
                    {
                        fixme("DT_SYMTAB - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_RELA:
                    {
                        fixme("DT_RELA - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_RELASZ:
                    {
                        fixme("DT_RELASZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_RELAENT:
                    {
                        fixme("DT_RELAENT - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_STRSZ:
                    {
                        fixme("DT_STRSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_SYMENT:
                    {
                        fixme("DT_SYMENT - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_INIT:
                    {
                        debug("DT_INIT - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        InitAddress = Dynamic[i].d_un.d_ptr;
                        break;
                    }
                    case DT_FINI:
                    {
                        debug("DT_FINI - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        FiniAddress = Dynamic[i].d_un.d_ptr;
                        break;
                    }
                    case DT_SONAME:
                    {
                        fixme("DT_SONAME - Name: %s", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_RPATH:
                    {
                        fixme("DT_RPATH - Name: %s", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_SYMBOLIC:
                    {
                        fixme("DT_SYMBOLIC - Name: %s", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_REL:
                    {
                        fixme("DT_REL - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_RELSZ:
                    {
                        fixme("DT_RELSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_RELENT:
                    {
                        fixme("DT_RELENT - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_PLTREL:
                    {
                        fixme("DT_PLTREL - Type: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_DEBUG:
                    {
                        fixme("DT_DEBUG - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_TEXTREL:
                    {
                        fixme("DT_TEXTREL - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_JMPREL:
                    {
                        fixme("DT_JMPREL - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_BIND_NOW:
                    {
                        fixme("DT_BIND_NOW - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_INIT_ARRAY:
                    {
                        fixme("DT_INIT_ARRAY - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_FINI_ARRAY:
                    {
                        fixme("DT_FINI_ARRAY - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_INIT_ARRAYSZ:
                    {
                        fixme("DT_INIT_ARRAYSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_FINI_ARRAYSZ:
                    {
                        fixme("DT_FINI_ARRAYSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_RUNPATH:
                    {
                        fixme("DT_RUNPATH - Name: %s", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_FLAGS:
                    {
                        fixme("DT_FLAGS - Flags: %#lx", Dynamic[i].d_un.d_val);
                        break;
                    }
                    case DT_PREINIT_ARRAY:
                    {
                        fixme("DT_PREINIT_ARRAY - Address: %#lx", Dynamic[i].d_un.d_ptr);
                        break;
                    }
                    case DT_PREINIT_ARRAYSZ:
                    {
                        fixme("DT_PREINIT_ARRAYSZ - Size: %ld", Dynamic[i].d_un.d_val);
                        break;
                    }
                    /* ... */
                    default:
                        fixme("DT: %ld", Dynamic[i].d_tag);
                        break;
                    }

                    if (Dynamic[i].d_tag == DT_NULL)
                        break;
                }

                break;
            }
            case PT_INTERP: // Do I have to do anything here?
            {
                debug("PT_INTERP - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrProgramHeader.p_offset, ItrProgramHeader.p_vaddr,
                      ItrProgramHeader.p_filesz, ItrProgramHeader.p_memsz, ItrProgramHeader.p_align);

                char InterpreterPath[256];
                memcpy((void *)InterpreterPath, (uint8_t *)BaseImage + ItrProgramHeader.p_offset, 256);
                fixme("Interpreter: %s", InterpreterPath);
                FileSystem::FILE *InterpreterFile = vfs->Open(InterpreterPath);
                if (InterpreterFile->Status != FileSystem::FileStatus::OK)
                {
                    error("Failed to open interpreter file: %s", InterpreterPath);
                }
                else
                {
                }
                break;
            }
            /* ... */
            case PT_PHDR:
            {
                debug("PT_PHDR - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrProgramHeader.p_offset, ItrProgramHeader.p_vaddr,
                      ItrProgramHeader.p_filesz, ItrProgramHeader.p_memsz, ItrProgramHeader.p_align);
                break;
            }
            default:
            {
                warn("Unknown or unsupported program header type: %d", ItrProgramHeader.p_type);
                break;
            }
            }
        }

        debug("Entry Point: %#lx", ELFHeader->e_entry);

        Vector<AuxiliaryVector> auxv;

        auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
        auxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)Path}}});
        auxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t) "x86_64"}}});
        auxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)ELFHeader->e_entry}}});
        auxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)MemoryImage}}});
        auxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
        auxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader->e_phnum}}});
        auxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader->e_phentsize}}});
        auxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFHeader->e_phoff}}});

        TCB *Thread = TaskManager->CreateThread(Process,
                                                (IP)ELFHeader->e_entry,
                                                argv, envp, auxv,
                                                (IPOffset)0 /* ProgramHeader->p_offset */, // I guess I don't need this
                                                Arch,
                                                Comp);
        ret->Process = Process;
        ret->Thread = Thread;
        ret->Status = ExStatus::OK;
    }
}
