#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../kernel.h"
#include "../Fex.hpp"

using namespace Tasking;

namespace Execute
{
    SpawnData Spawn(char *Path, const char **argv, const char **envp)
    {
        SpawnData ret = {.Status = ExStatus::Unknown,
                         .Process = nullptr,
                         .Thread = nullptr};
        FileSystem::FILE *ExFile = vfs->Open(Path);
        if (ExFile->Status == FileSystem::FileStatus::OK)
        {
            if (ExFile->Node->Flags == FileSystem::NodeFlags::FS_FILE)
            {
                BinaryType Type = GetBinaryType(Path);
                switch (Type)
                {
                case BinaryType::BinTypeFex:
                {
#if defined(__amd64__)

                    Fex *FexHdr = (Fex *)ExFile->Node->Address;
                    if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
                    {
                        const char *BaseName;
                        cwk_path_get_basename(Path, &BaseName, nullptr);
                        PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), BaseName, TaskTrustLevel::User);

                        void *BaseImage = KernelAllocator.RequestPages(TO_PAGES(ExFile->Node->Length));
                        memcpy(BaseImage, (void *)ExFile->Node->Address, ExFile->Node->Length);

                        Memory::Virtual pva = Memory::Virtual(Process->PageTable);
                        for (uint64_t i = 0; i < TO_PAGES(ExFile->Node->Length); i++)
                            pva.Map((void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), (void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                        Vector<AuxiliaryVector> auxv; // TODO!

                        TCB *Thread = TaskManager->CreateThread(Process,
                                                                (IP)FexHdr->Pointer,
                                                                argv, envp, auxv,
                                                                (IPOffset)BaseImage,
                                                                TaskArchitecture::x64,
                                                                TaskCompatibility::Native);
                        ret.Process = Process;
                        ret.Thread = Thread;
                        ret.Status = ExStatus::OK;
#elif defined(__i386__)
                    if (1)
                    {
#elif defined(__aarch64__)
                    if (1)
                    {
#endif
                        goto Exit;
                    }
                    ret.Status = ExStatus::InvalidFileHeader;
                    goto Exit;
                }
                case BinaryType::BinTypeELF:
                {
#if defined(__amd64__)
                    const char *BaseName;
                    cwk_path_get_basename(Path, &BaseName, nullptr);

                    void *BaseImage = KernelAllocator.RequestPages(TO_PAGES(ExFile->Node->Length));
                    memcpy(BaseImage, (void *)ExFile->Node->Address, ExFile->Node->Length);
                    debug("Image Size: %#lx - %#lx (length: %ld)", BaseImage, (uint64_t)BaseImage + ExFile->Node->Length, ExFile->Node->Length);

                    PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), BaseName, TaskTrustLevel::User, BaseImage);

                    Memory::Virtual pva = Memory::Virtual(Process->PageTable);
                    for (uint64_t i = 0; i < TO_PAGES(ExFile->Node->Length); i++)
                        pva.Remap((void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), (void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                    Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)BaseImage;

                    TaskArchitecture Arch = TaskArchitecture::x64;
                    TaskCompatibility Comp = TaskCompatibility::Native;
                    if (ELFHeader->e_machine == EM_386)
                        Arch = TaskArchitecture::x32;
                    else if (ELFHeader->e_machine == EM_AMD64)
                        Arch = TaskArchitecture::x64;
                    else if (ELFHeader->e_machine == EM_AARCH64)
                        Arch = TaskArchitecture::ARM64;
                    else
                        Arch = TaskArchitecture::UnknownArchitecture;

                    // TODO: Should I care about this?
                    if (ELFHeader->e_ident[EI_CLASS] == ELFCLASS32)
                    {
                        if (ELFHeader->e_ident[EI_DATA] == ELFDATA2LSB)
                            fixme("ELF32 LSB");
                        else if (ELFHeader->e_ident[EI_DATA] == ELFDATA2MSB)
                            fixme("ELF32 MSB");
                        else
                            fixme("ELF32 Unknown");
                    }
                    else if (ELFHeader->e_ident[EI_CLASS] == ELFCLASS64)
                    {
                        if (ELFHeader->e_ident[EI_DATA] == ELFDATA2LSB)
                            fixme("ELF64 LSB");
                        else if (ELFHeader->e_ident[EI_DATA] == ELFDATA2MSB)
                            fixme("ELF64 MSB");
                        else
                            fixme("ELF64 Unknown");
                    }
                    else
                        fixme("Unknown ELF");

                    if (ELFHeader->e_type == ET_EXEC)
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
                                    pva.Remap((void *)((uint64_t)MemoryImage + (i * PAGE_SIZE)), (void *)((uint64_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
                                    debug("Mapping: %#lx -> %#lx", (uint64_t)MemoryImage + (i * PAGE_SIZE), (uint64_t)MemoryImage + (i * PAGE_SIZE));
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
                                pva.Remap((void *)((uint64_t)ProgramHeader->p_vaddr + (i * PAGE_SIZE)), (void *)((uint64_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
                                debug("Mapping: %#lx -> %#lx", (uint64_t)ProgramHeader->p_vaddr + (i * PAGE_SIZE), (uint64_t)MemoryImage + (i * PAGE_SIZE));
                            }
                        }

                        debug("BaseAddress: %#lx | ElfAppSize: %#lx (%ld, %ld KB)", BaseAddress, ElfAppSize, ElfAppSize, TO_KB(ElfAppSize));

                        for (Elf64_Half i = 0; i < ELFHeader->e_phnum; i++)
                        {
                            memcpy(&ItrProgramHeader, (uint8_t *)BaseImage + ELFHeader->e_phoff + ELFHeader->e_phentsize * i, sizeof(Elf64_Phdr));
                            uintptr_t MAddr;

                            if (ItrProgramHeader.p_type == PT_LOAD)
                            {
                                debug("PT_LOAD");
                                MAddr = (ItrProgramHeader.p_vaddr - BaseAddress) + (uintptr_t)MemoryImage;

                                memcpy(MemoryImage, (uint8_t *)BaseImage + ItrProgramHeader.p_offset, ItrProgramHeader.p_filesz);
                                debug("MemoryImage: %#lx", MemoryImage);
                                debug("MAddr: %#lx", MAddr);
                                debug("memset operation: 0 to %#lx for length %ld", MemoryImage + MAddr, ItrProgramHeader.p_memsz);
                                debug("memcpy operation: %#lx to %#lx for length %ld", (uint8_t *)BaseImage + ItrProgramHeader.p_offset, MemoryImage + MAddr, ItrProgramHeader.p_filesz);
                            }
                            else
                            {
                                fixme("Not PT_LOAD (%ld)", ItrProgramHeader.p_type);
                            }
                        }

                        debug("Entry Point: %#lx", ELFHeader->e_entry);

                        Vector<AuxiliaryVector> auxv;

                        auxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFHeader->e_phoff}}});
                        auxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader->e_phentsize}}});
                        auxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader->e_phnum}}});
                        auxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
                        auxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)MemoryImage}}});
                        auxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)ELFHeader->e_entry + (uint64_t)ProgramHeader->p_offset}}});
                        auxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t) "x86_64"}}});
                        auxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)Path}}});

                        TCB *Thread = TaskManager->CreateThread(Process,
                                                                (IP)ELFHeader->e_entry,
                                                                argv, envp, auxv,
                                                                (IPOffset)ProgramHeader->p_offset,
                                                                Arch,
                                                                Comp);
                        ret.Process = Process;
                        ret.Thread = Thread;
                        ret.Status = ExStatus::OK;
                        goto Exit;
                    }
                    else if (ELFHeader->e_type == ET_DYN)
                    {
                        fixme("Shared Object");
                    }
                    else if (ELFHeader->e_type == ET_REL)
                    {
                        trace("Relocatable");
                        void *EP = ELFLoadRel(ELFHeader);
                        if (EP == (void *)0xdeadbeef || EP == 0x0)
                        {
                            ret.Status = ExStatus::InvalidFileEntryPoint;
                            goto Exit;
                        }

                        Vector<AuxiliaryVector> auxv;
                        fixme("auxv");

                        TCB *Thread = TaskManager->CreateThread(Process,
                                                                (IP)EP,
                                                                argv, envp, auxv,
                                                                (IPOffset)BaseImage,
                                                                Arch,
                                                                Comp);
                        ret.Process = Process;
                        ret.Thread = Thread;
                        ret.Status = ExStatus::OK;
                        goto Exit;
                    }
                    else if (ELFHeader->e_type == ET_CORE)
                    {
                        fixme("Core");
                    }
                    else
                    {
                        fixme("Unknown");
                    }
                    ret.Status = ExStatus::InvalidFileHeader;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
                    goto Exit;
                }
                default:
                    ret.Status = ExStatus::Unsupported;
                    goto Exit;
                }
                goto Exit;
            }
        }
        else if (ExFile->Status == FileSystem::FileStatus::NOT_FOUND)
        {
            ret.Status = ExStatus::InvalidFilePath;
            goto Exit;
        }
        else
        {
            ret.Status = ExStatus::InvalidFile;
            goto Exit;
        }

    Exit:
        if (ret.Status != ExStatus::OK)
            if (ret.Process)
                ret.Process->Status = TaskStatus::Terminated;
        vfs->Close(ExFile);
        return ret;
    }
}
