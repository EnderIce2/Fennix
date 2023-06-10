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
    ELFBaseLoad ELFLoad(char *Path,
                        const char **argv, const char **envp,
                        TaskCompatibility Compatibility)
    {
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
        else if (ExFile.GetFlags() != NodeFlags::FILE)
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

        size_t ExFileSize = ExFile.GetLength();

        void *ElfFile = KernelAllocator.RequestPages(TO_PAGES(ExFileSize + 1));
        vfs->Read(ExFile, (uint8_t *)ElfFile, ExFileSize);
        debug("Loaded elf %s at %#lx with the length of %ld",
              Path, ElfFile, ExFileSize);

        Elf32_Ehdr *ELFHeader = (Elf32_Ehdr *)ElfFile;

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
            fixme("32-bit ELF");
        else if (ELFHeader->e_ident[EI_CLASS] == ELFCLASS64)
            fixme("64-bit ELF");
        else
            fixme("Unknown class %d", ELFHeader->e_ident[EI_CLASS]);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        if (ELFHeader->e_ident[EI_DATA] != ELFDATA2LSB)
        {
            fixme("ELF32 LSB expected, got %d", ELFHeader->e_ident[EI_DATA]);
        }
#else
        if (ELFHeader->e_ident[EI_DATA] != ELFDATA2MSB)
        {
            fixme("ELF32 MSB expected, got %d", ELFHeader->e_ident[EI_DATA]);
        }
#endif

        /* ------------------------------------------------------------------------------------------------------------------------------ */

        PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(),
                                                  BaseName, TaskTrustLevel::User, ElfFile);
        Process->SetWorkingDirectory(vfs->GetNodeFromPath(Path)->Parent);
        Process->Info.Compatibility = TaskCompatibility::Native;
        Process->Info.Architecture = TaskArchitecture::x64;

        ELFBaseLoad bl;

        ELFObject *obj = new ELFObject(Path, Process);
        if (!obj->IsValid())
        {
            error("Failed to load ELF object");
            vfs->Close(ExFile);
            TaskManager->RevertProcessCreation(Process);
            return {};
        }

        bl = obj->GetBaseLoadInfo();

        /* TODO: Keep only the necessary headers */
        Memory::Virtual vmm = Memory::Virtual(Process->PageTable);
        for (size_t i = 0; i < TO_PAGES(ExFileSize); i++)
        {
            void *AddressToMap = (void *)((uintptr_t)ElfFile + (i * PAGE_SIZE));
            vmm.Remap(AddressToMap, AddressToMap, Memory::RW | Memory::US);
        }

        if (bl.Interpreter)
        {
            debug("Keeping ElfFile at %p", ElfFile);

            TCB *InterIPCThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(),
                                                            (IP)ELFInterpreterIPCThread,
                                                            nullptr,
                                                            nullptr,
                                                            std::vector<AuxiliaryVector>(),
                                                            TaskArchitecture::x64,
                                                            TaskCompatibility::Native,
                                                            true);

            std::vector<const char *> *tmp_needed_libs =
                new std::vector<const char *>(bl.NeededLibraries);

            InterIPCThread->SYSV_ABI_Call((uintptr_t)Process,
                                          (uintptr_t) new std::string(Path),
                                          (uintptr_t)bl.VirtualMemoryImage,
                                          (uintptr_t)tmp_needed_libs);

            InterIPCThread->Rename("ELF Interpreter IPC Thread");
            InterIPCThread->SetPriority(TaskPriority::Low);
            InterIPCThread->Status = TaskStatus::Ready;
        }

        TCB *Thread = TaskManager->CreateThread(Process,
                                                bl.InstructionPointer,
                                                argv, envp, bl.auxv,
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
