#ifndef __FENNIX_KERNEL_FILE_EXECUTE_H__
#define __FENNIX_KERNEL_FILE_EXECUTE_H__

#include <types.h>

namespace Execute
{
    enum ExStatus
    {
        OK,
        Unknown,
        InvalidFile,
        InvalidFileFormat,
        InvalidFileHeader,
        InvalidFileData,
        InvalidFileEntryPoint,
        InvalidFilePath
    };

    ExStatus Spawn(char *Path, uint64_t Arg0, uint64_t Arg1);
}

#endif // !__FENNIX_KERNEL_FILE_EXECUTE_H__
