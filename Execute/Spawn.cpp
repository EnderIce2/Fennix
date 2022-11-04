#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <cwalk.h>

#include "../kernel.h"
#include "../Fex.hpp"

using namespace Tasking;

namespace Execute
{
    SpawnData Spawn(char *Path, uint64_t Arg0, uint64_t Arg1)
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
                    Fex *FexHdr = (Fex *)ExFile->Node->Address;
                    if (FexHdr->Magic[0] == 'F' && FexHdr->Magic[1] == 'E' && FexHdr->Magic[2] == 'X' && FexHdr->Magic[3] == '\0')
                    {
                        if (FexHdr->Type == FexFormatType::FexFormatType_Executable)
                        {
                            const char *BaseName;
                            cwk_path_get_basename(Path, &BaseName, nullptr);
                            PCB *Process = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), BaseName, TaskTrustLevel::User);

                            void *BaseImage = KernelAllocator.RequestPages(TO_PAGES(ExFile->Node->Length));
                            memcpy(BaseImage, (void *)ExFile->Node->Address, ExFile->Node->Length);

                            for (uint64_t i = 0; i < TO_PAGES(ExFile->Node->Length); i++)
                                Memory::Virtual(Process->PageTable).Map((void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), (void *)((uint64_t)BaseImage + (i * PAGE_SIZE)), Memory::PTFlag::US);

                            TCB *Thread = TaskManager->CreateThread(Process,
                                                                    (IP)FexHdr->Pointer,
                                                                    Arg0, Arg1,
                                                                    (IPOffset)BaseImage,
                                                                    TaskArchitecture::x64,
                                                                    TaskCompatibility::Native);
                            ret.Process = Process;
                            ret.Thread = Thread;
                            ret.Status = ExStatus::OK;
                            goto Exit;
                        }
                    }
                    ret.Status = ExStatus::InvalidFileHeader;
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
        vfs->Close(ExFile);
        return ret;
    }
}
