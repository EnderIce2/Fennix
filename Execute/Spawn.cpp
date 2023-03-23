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

                    Memory::Virtual pva = Memory::Virtual(Process->PageTable);
                    for (size_t i = 0; i < TO_PAGES(ExFile->node->Length); i++)
                        pva.Map((void *)((uintptr_t)BaseImage + (i * PAGE_SIZE)), (void *)((uintptr_t)BaseImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

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
