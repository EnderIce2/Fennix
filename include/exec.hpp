#ifndef __FENNIX_KERNEL_FILE_EXECUTE_H__
#define __FENNIX_KERNEL_FILE_EXECUTE_H__

#include <types.h>

#include <task.hpp>

namespace Execute
{
    enum BinaryType
    {
        BinTypeInvalid,
        BinTypeFex,
        BinTypeElf,
        BinTypePE,
        BinTypeUnknown
    };

    enum ExStatus
    {
        OK,
        Unknown,
        Unsupported,
        InvalidFile,
        InvalidFileFormat,
        InvalidFileHeader,
        InvalidFileData,
        InvalidFileEntryPoint,
        InvalidFilePath
    };

    struct SpawnData
    {
        ExStatus Status;
        Tasking::PCB *Process;
        Tasking::TCB *Thread;
    };

    BinaryType GetBinaryType(char *Path);
    SpawnData Spawn(char *Path, uint64_t Arg0, uint64_t Arg1);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
