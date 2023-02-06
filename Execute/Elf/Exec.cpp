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
    ELFBaseLoad ELFLoadExec(void *ElfFile,
                            VirtualFileSystem::File *ExFile,
                            Tasking::PCB *Process)
    {
        debug("Executable");
        ELFBaseLoad ELFBase = {};
        /* This should be deleted inside BaseLoad.cpp */
        ELFBase.TmpMem = new Memory::MemMgr(Process->PageTable);

        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)ElfFile;
        Memory::Virtual pV(Process->PageTable);

        uintptr_t BaseAddress = UINTPTR_MAX;
        uint64_t ElfAppSize = 0;
        uintptr_t EntryPoint = ELFHeader->e_entry;

        Elf64_Phdr ItrPhdr;

        /* Get base address */
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)ElfFile + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            BaseAddress = MIN(BaseAddress, ItrPhdr.p_vaddr);
        }

        /* Get size */
        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)ElfFile + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            uintptr_t SegmentEnd;
            SegmentEnd = ItrPhdr.p_vaddr - BaseAddress + ItrPhdr.p_memsz;
            ElfAppSize = MAX(ElfAppSize, SegmentEnd);
        }
        debug("BaseAddress: %#lx | ElfAppSize: %#lx (%ld, %ld KB)", BaseAddress, ElfAppSize, ElfAppSize, TO_KB(ElfAppSize));

        /* If required, MemoryImage will be at virtual address. (unless has PIE)
         *
         * tl;dr this is where the code is stored. */
        void *MemoryImage = ELFCreateMemoryImage(ELFBase.TmpMem, pV, ElfFile, ElfAppSize);

        debug("Solving symbols for address: %#llx", (uintptr_t)ElfFile);
        Elf64_Shdr *ElfSections = (Elf64_Shdr *)((uintptr_t)ElfFile + ELFHeader->e_shoff);
        Elf64_Shdr *DynamicString = nullptr;
        Elf64_Shdr *StringTable = nullptr;

        for (Elf64_Half i = 0; i < ELFHeader->e_shnum; i++)
        {
            char *DynamicStringTable = (char *)((uintptr_t)ElfFile + ElfSections[ELFHeader->e_shstrndx].sh_offset + ElfSections[i].sh_name);

            if (strcmp(DynamicStringTable, ".dynstr") == 0)
            {
                DynamicString = &ElfSections[i];
                debug("Found .dynstr");
            }
            else if (strcmp(DynamicStringTable, ".strtab") == 0)
            {
                StringTable = &ElfSections[i];
                debug("Found .strtab");
            }
        }

        Vector<char *> NeededLibraries;

        if (!DynamicString)
            DynamicString = StringTable;

        /* Calculate entry point */
        memcpy(&ItrPhdr, (uint8_t *)ElfFile + ELFHeader->e_phoff, sizeof(Elf64_Phdr));
        if (ItrPhdr.p_vaddr == 0)
            EntryPoint += (uintptr_t)MemoryImage;

        char InterpreterPath[256];

        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
        {
            memcpy(&ItrPhdr,
                   (uint8_t *)ElfFile + ELFHeader->e_phoff + ELFHeader->e_phentsize * i,
                   sizeof(Elf64_Phdr));

            switch (ItrPhdr.p_type)
            {
            case PT_NULL:
                fixme("PT_NULL");
                break;
            case PT_LOAD:
            {
                debug("PT_LOAD - Offset: %#lx, VirtAddr: %#lx, FileSiz: %ld, MemSiz: %ld, Align: %#lx",
                      ItrPhdr.p_offset, ItrPhdr.p_vaddr,
                      ItrPhdr.p_filesz, ItrPhdr.p_memsz, ItrPhdr.p_align);
                uintptr_t MAddr = (ItrPhdr.p_vaddr - BaseAddress) + (uintptr_t)MemoryImage;
                fixme("Address: %#lx %s%s%s", MAddr,
                      (ItrPhdr.p_flags & PF_R) ? "R" : "",
                      (ItrPhdr.p_flags & PF_W) ? "W" : "",
                      (ItrPhdr.p_flags & PF_X) ? "X" : "");

                memcpy((void *)MAddr, (uint8_t *)ElfFile + ItrPhdr.p_offset, ItrPhdr.p_filesz);
                debug("memcpy: %#lx => %#lx (%ld bytes)", (uint8_t *)ElfFile + ItrPhdr.p_offset, MAddr, ItrPhdr.p_filesz);
                break;
            }
            case PT_DYNAMIC:
            {
                debug("PT_DYNAMIC - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrPhdr.p_offset, ItrPhdr.p_vaddr,
                      ItrPhdr.p_filesz, ItrPhdr.p_memsz, ItrPhdr.p_align);

                Elf64_Dyn *Dynamic = (Elf64_Dyn *)((uint8_t *)ElfFile + ItrPhdr.p_offset);

                for (size_t i = 0; i < ItrPhdr.p_filesz / sizeof(Elf64_Dyn); i++)
                {
                    if (Dynamic[i].d_tag == DT_NEEDED)
                    {
                        if (!DynamicString)
                        {
                            error("DynamicString is null");
                            break;
                        }

                        char *ReqLib = (char *)kmalloc(256);
                        strcpy(ReqLib, (char *)((uintptr_t)ElfFile + DynamicString->sh_offset + Dynamic[i].d_un.d_ptr));
                        debug("DT_NEEDED - Name[%ld]: %s", i, ReqLib);
                        NeededLibraries.push_back(ReqLib);
                    }
                    else if (Dynamic[i].d_tag == DT_NULL)
                        break;
                }
                break;
            }
            case PT_INTERP:
            {
                debug("PT_INTERP - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrPhdr.p_offset, ItrPhdr.p_vaddr,
                      ItrPhdr.p_filesz, ItrPhdr.p_memsz, ItrPhdr.p_align);

                memcpy((void *)InterpreterPath, (uint8_t *)ElfFile + ItrPhdr.p_offset, 256);
                debug("Interpreter: %s", InterpreterPath);

                shared_ptr<VirtualFileSystem::File> InterpreterFile = vfs->Open(InterpreterPath);
                if (InterpreterFile->Status != VirtualFileSystem::FileStatus::OK)
                    warn("Failed to open interpreter file: %s", InterpreterPath);

                vfs->Close(InterpreterFile);
                break;
            }
            /* ... */
            case PT_PHDR:
            {
                debug("PT_PHDR - Offset: %#lx VirtAddr: %#lx FileSiz: %ld MemSiz: %ld Align: %#lx",
                      ItrPhdr.p_offset, ItrPhdr.p_vaddr,
                      ItrPhdr.p_filesz, ItrPhdr.p_memsz, ItrPhdr.p_align);
                break;
            }
            default:
            {
                warn("Unknown or unsupported program header type: %d", ItrPhdr.p_type);
                break;
            }
            }
        }

        EntryPoint = LoadELFInterpreter(ELFBase.TmpMem, pV, InterpreterPath);

        debug("Entry Point: %#lx", EntryPoint);

        char *aux_platform = (char *)ELFBase.TmpMem->RequestPages(1, true);
        strcpy(aux_platform, "x86_64");

        ELFBase.auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)vfs->GetPathFromNode(ExFile->node).Get()}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t)aux_platform}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)EntryPoint}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)MemoryImage}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader->e_phnum}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader->e_phentsize}}});
        ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFHeader->e_phoff}}});

        ELFBase.InstructionPointer = EntryPoint;

        foreach (auto var in NeededLibraries)
            kfree(var);

        ELFBase.Success = true;
        return ELFBase;
    }
}
