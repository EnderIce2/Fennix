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
using VirtualFileSystem::File;
using VirtualFileSystem::FileStatus;
using VirtualFileSystem::NodeFlags;

namespace Execute
{
    ELFBaseLoad ELFLoad(char *Path, const char **argv, const char **envp, Tasking::TaskCompatibility Compatibility)
    {
        /* We get the base name ("app.elf") */
        const char *BaseName;
        cwk_path_get_basename(Path, &BaseName, nullptr);
        TaskArchitecture Arch = TaskArchitecture::UnknownArchitecture;

        shared_ptr<File> ExFile = vfs->Open(Path);

        if (ExFile->Status != FileStatus::OK)
        {
            vfs->Close(ExFile);
            error("Failed to open file: %s", Path);
            return {};
        }
        else
        {
            if (ExFile->node->Flags != NodeFlags::FILE)
            {
                vfs->Close(ExFile);
                error("Invalid file path: %s", Path);
                return {};
            }
            else if (GetBinaryType(Path) != BinaryType::BinTypeELF)
            {
                vfs->Close(ExFile);
                error("Invalid file type: %s", Path);
                return {};
            }
        }

        size_t ExFileSize = ExFile->node->Length;

        /* Allocate elf in memory */
        void *ElfFile = KernelAllocator.RequestPages(TO_PAGES(ExFileSize));
        /* Copy the file to the allocated memory */
        memcpy(ElfFile, (void *)ExFile->node->Address, ExFileSize);
        debug("Image Size: %#lx - %#lx (length: %ld)", ElfFile, (uintptr_t)ElfFile + ExFileSize, ExFileSize);

        Elf64_Ehdr *ELFHeader = (Elf64_Ehdr *)ElfFile;

        switch (ELFHeader->e_machine)
        {
        case EM_386:
            Arch = TaskArchitecture::x32;
            break;
        case EM_X86_64:
            Arch = TaskArchitecture::x64;
            break;
        case EM_ARM:
            Arch = TaskArchitecture::ARM32;
            break;
        case EM_AARCH64:
            Arch = TaskArchitecture::ARM64;
            break;
        default:
            break;
        }

        // TODO: This shouldn't be ignored
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

        /* ------------------------------------------------------------------------------------------------------------------------------ */

        PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), BaseName, TaskTrustLevel::User, ElfFile);
        Memory::Virtual pV = Memory::Virtual(Process->PageTable);
        for (size_t i = 0; i < TO_PAGES(ExFileSize); i++)
            pV.Remap((void *)((uintptr_t)ElfFile + (i * PAGE_SIZE)), (void *)((uintptr_t)ElfFile + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

        // for (size_t i = 0; i < TO_PAGES(ElfLazyResolverSize); i++)
        // pV.Remap((void *)((uintptr_t)ElfLazyResolver + (i * PAGE_SIZE)), (void *)((uintptr_t)ElfLazyResolver + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

        /* We prepare the ELF for execution (allocate memory, etc...) */
        ELFBaseLoad bl;

        switch (ELFHeader->e_type)
        {
        case ET_REL:
            bl = ELFLoadRel(ElfFile, ExFile.Get(), Process);
            break;
        case ET_EXEC:
            bl = ELFLoadExec(ElfFile, ExFile.Get(), Process);
            break;
        case ET_DYN:
            bl = ELFLoadDyn(ElfFile, ExFile.Get(), Process);
            break;
        case ET_CORE:
        {
            fixme("ET_CORE not implemented");
            TaskManager->RevertProcessCreation(Process);
            vfs->Close(ExFile);
            return {};
        }
        case ET_NONE:
        default:
        {
            error("Unknown ELF Type: %d", ELFHeader->e_type);
            vfs->Close(ExFile);
            TaskManager->RevertProcessCreation(Process);
            return {};
        }
        }

        TCB *Thread = TaskManager->CreateThread(Process,
                                                bl.InstructionPointer,
                                                argv, envp, bl.auxv,
                                                (IPOffset)0 /* ProgramHeader->p_offset */, // I guess I don't need this
                                                Arch,
                                                Compatibility);

        foreach (Memory::MemMgr::AllocatedPages p in bl.TmpMem->GetAllocatedPagesList())
        {
            Thread->Memory->Add(p.Address, p.PageCount);
            bl.TmpMem->DetachAddress(p.Address);
        }
        delete bl.TmpMem;

        bl.sd.Process = Process;
        bl.sd.Thread = Thread;
        bl.sd.Status = ExStatus::OK;
        vfs->Close(ExFile);
        return bl;
    }
}
