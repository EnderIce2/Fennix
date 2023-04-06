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

        std::shared_ptr<VirtualFileSystem::File> ExFile = vfs->Open(Path);

        if (ExFile->Status == VirtualFileSystem::FileStatus::OK)
        {
            if (ExFile->node->Flags != VirtualFileSystem::NodeFlags::FILE)
            {
                ret.Status = ExStatus::InvalidFilePath;
                goto Exit;
            }

            switch (GetBinaryType(Path))
            {
            case BinaryType::BinTypeFex:
            {
                Fex *FexHdr = (Fex *)ExFile->node->Address;
                if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
                {
                    const char *BaseName;
                    cwk_path_get_basename(Path, &BaseName, nullptr);
                    PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), BaseName, TaskTrustLevel::User);

                    void *BaseImage = KernelAllocator.RequestPages(TO_PAGES(ExFile->node->Length));
                    memcpy(BaseImage, (void *)ExFile->node->Address, ExFile->node->Length);

                    Memory::Virtual(Process->PageTable).Map((void *)BaseImage, (void *)BaseImage, ExFile->node->Length, Memory::PTFlag::RW | Memory::PTFlag::US);

                    std::vector<AuxiliaryVector> auxv; // TODO!

                    TCB *Thread = TaskManager->CreateThread(Process,
                                                            (IP)FexHdr->EntryPoint,
                                                            argv, envp, auxv,
                                                            (IPOffset)BaseImage,
                                                            TaskArchitecture::x64,
                                                            TaskCompatibility::Native);
                    ret.Process = Process;
                    ret.Thread = Thread;
                    ret.Status = ExStatus::OK;
                }

                ret.Status = ExStatus::InvalidFileHeader;
                goto Exit;
            }
            case BinaryType::BinTypeELF:
            {
                ELFBaseLoad bl = ELFLoad(Path, argv, envp);
                if (!bl.Success)
                {
                    ret.Status = ExStatus::GenericError;
                    goto Exit;
                }
                ret = bl.sd;
                goto Exit;
            }
            default:
            {
                ret.Status = ExStatus::Unsupported;
                goto Exit;
            }
            }
        }
        else if (ExFile->Status == VirtualFileSystem::FileStatus::NotFound)
            ret.Status = ExStatus::InvalidFilePath;
        else
            ret.Status = ExStatus::InvalidFile;

    Exit:
        vfs->Close(ExFile);
        return ret;
    }
}
