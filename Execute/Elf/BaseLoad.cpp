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

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;
using VirtualFileSystem::File;
using VirtualFileSystem::FileStatus;
using VirtualFileSystem::NodeFlags;

namespace Execute
{
    struct InterpreterIPCDataLibrary
    {
        char Name[128];
    };

    typedef struct
    {
        char Path[256];
        void *ElfFile;
        void *MemoryImage;
        struct InterpreterIPCDataLibrary Libraries[64];
    } InterpreterIPCData;

    /* Passing arguments as a sanity check and debugging. */
    void ELFInterpreterIPCThread(PCB *Process, char *Path, void *MemoryImage, void *ElfFile, std::vector<const char *> NeededLibraries)
    {
        debug("Interpreter thread started for %s", Path);
        // Interpreter will create an IPC with token "LOAD".
        char UniqueToken[16] = {'L', 'O', 'A', 'D', '\0'};
        InterProcessCommunication::IPCHandle *Handle = nullptr;
        while (Handle == nullptr)
        {
            debug("Searching for IPC with token %s", UniqueToken);
            Handle = Process->IPC->SearchByToken(UniqueToken);
            if (Handle == nullptr)
            {
                debug("Failed");
            }

            TaskManager->Sleep(200);
            if (Handle == nullptr)
            {
                debug("Retrying...");
            }
        }
        debug("IPC found, sending data...");
        InterpreterIPCData *TmpBuffer = new InterpreterIPCData;
        strcpy(TmpBuffer->Path, Path);
        TmpBuffer->ElfFile = ElfFile;
        TmpBuffer->MemoryImage = MemoryImage;
        if (NeededLibraries.size() > 256)
            warn("Too many libraries! (max 256)");
        for (size_t i = 0; i < NeededLibraries.size(); i++)
        {
            strcpy(TmpBuffer->Libraries[i].Name, NeededLibraries[i]);
        }

#ifdef DEBUG
        debug("OUTSIDE DATA");
        debug("Path: %s", Path);
        debug("ElfFile: %p", ElfFile);
        debug("MemoryImage: %p", MemoryImage);
        for (size_t i = 0; i < NeededLibraries.size(); i++)
        {
            debug("Library: %s", NeededLibraries[i]);
        }
        debug("INSIDE DATA");
        debug("Path: %s", TmpBuffer->Path);
        debug("ElfFile: %p", TmpBuffer->ElfFile);
        debug("MemoryImage: %p", TmpBuffer->MemoryImage);
        for (size_t i = 0; i < NeededLibraries.size(); i++)
        {
            debug("Library: %s", TmpBuffer->Libraries[i].Name);
        }
#endif

    RetryIPCWrite:
        InterProcessCommunication::IPCErrorCode ret = Process->IPC->Write(Handle->ID, TmpBuffer, sizeof(InterpreterIPCData));
        debug("Write returned %d", ret);
        if (ret == InterProcessCommunication::IPCErrorCode::IPCNotListening)
        {
            debug("IPC not listening, retrying...");
            TaskManager->Sleep(100);
            goto RetryIPCWrite;
        }
        delete TmpBuffer;
        /* Prevent race condition, maybe a
           better idea is to watch when the
           IPC is destroyed. */
        TaskManager->Sleep(5000);
        TEXIT(0);
    }

    PCB *InterpreterTargetProcess;
    std::string *InterpreterTargetPath; /* We can't have String as a constructor :( */
    void *InterpreterMemoryImage;
    void *InterpreterElfFile;
    std::vector<const char *> InterpreterNeededLibraries;
    void ELFInterpreterThreadWrapper()
    {
        ELFInterpreterIPCThread(InterpreterTargetProcess, (char *)InterpreterTargetPath->c_str(), InterpreterMemoryImage, InterpreterElfFile, InterpreterNeededLibraries);
        delete InterpreterTargetPath, InterpreterTargetPath = nullptr;
    }

    ELFBaseLoad ELFLoad(char *Path, const char **argv, const char **envp, Tasking::TaskCompatibility Compatibility)
    {
        /* We get the base name ("app.elf") */
        const char *BaseName;
        cwk_path_get_basename(Path, &BaseName, nullptr);
        TaskArchitecture Arch = TaskArchitecture::UnknownArchitecture;

        File ExFile = vfs->Open(Path);

        if (ExFile.Status != FileStatus::OK)
        {
            vfs->Close(ExFile);
            error("Failed to open file: %s", Path);
            return {};
        }
        else
        {
            if (ExFile.node->Flags != NodeFlags::FILE)
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

        size_t ExFileSize = ExFile.node->Length;

        /* Allocate elf in memory */
        void *ElfFile = KernelAllocator.RequestPages(TO_PAGES(ExFileSize + 1));
        /* Copy the file to the allocated memory */
        memcpy(ElfFile, (void *)ExFile.node->Address, ExFileSize);
        debug("Elf file: %#lx - %#lx (length: %ld)", ElfFile, (uintptr_t)ElfFile + ExFileSize, ExFileSize);

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
        Process->SetWorkingDirectory(vfs->GetNodeFromPath(Path)->Parent);
        Memory::Virtual pV = Memory::Virtual(Process->PageTable);
        for (size_t i = 0; i < TO_PAGES(ExFileSize); i++)
            pV.Remap((void *)((uintptr_t)ElfFile + (i * PAGE_SIZE)), (void *)((uintptr_t)ElfFile + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

        // for (size_t i = 0; i < TO_PAGES(ElfLazyResolverSize); i++)
        // pV.Remap((void *)((uintptr_t)ElfLazyResolver + (i * PAGE_SIZE)), (void *)((uintptr_t)ElfLazyResolver + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

        ELFBaseLoad bl;

        switch (ELFHeader->e_type)
        {
        case ET_REL:
            bl = ELFLoadRel(ElfFile, ExFile, Process);
            break;
        case ET_EXEC:
            bl = ELFLoadExec(ElfFile, ExFile, Process);
            break;
        case ET_DYN:
            bl = ELFLoadDyn(ElfFile, ExFile, Process);
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

        if (bl.Interpreter)
        {
            debug("ElfFile: %p ELFHeader: %p", ElfFile, ELFHeader);

            InterpreterTargetProcess = Process;
            InterpreterTargetPath = new std::string(Path); /* We store in a String because Path may get changed while outside ELFLoad(). */
            InterpreterMemoryImage = bl.VirtualMemoryImage;
            InterpreterElfFile = ElfFile;
            InterpreterNeededLibraries = bl.NeededLibraries;
            __sync;
            TCB *InterpreterIPCThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)ELFInterpreterThreadWrapper);
            InterpreterIPCThread->Rename("ELF Interpreter IPC Thread");
            InterpreterIPCThread->SetPriority(TaskPriority::Low);
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
        delete bl.TmpMem, bl.TmpMem = nullptr;

        bl.sd.Process = Process;
        bl.sd.Thread = Thread;
        bl.sd.Status = ExStatus::OK;
        vfs->Close(ExFile);
        return bl;
    }
}
