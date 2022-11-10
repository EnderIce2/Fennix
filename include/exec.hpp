#ifndef __FENNIX_KERNEL_FILE_EXECUTE_H__
#define __FENNIX_KERNEL_FILE_EXECUTE_H__

#include <types.h>

#include <task.hpp>
#include <elf.h>

namespace Execute
{
    enum BinaryType
    {
        BinTypeInvalid,
        BinTypeFex,
        BinTypeELF,
        BinTypePE,
        BinTypeNE,
        BinTypeMZ,
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
    SpawnData Spawn(char *Path, Vector<const char *> &argv, Vector<const char *> &envp);

    void *ELFLoadRel(Elf64_Ehdr *Header);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
