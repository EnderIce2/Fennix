#ifndef __FENNIX_KERNEL_FILESYSTEM_INITRD_H__
#define __FENNIX_KERNEL_FILESYSTEM_INITRD_H__

#include <types.h>

#include <filesystem.hpp>

namespace FileSystem
{
    class Initrd
    {
    public:
        struct InitrdHeader
        {
            uint32_t nfiles;
        };

        struct InitrdFileHeader
        {
            uint8_t magic;
            char name[64];
            uint32_t offset;
            uint32_t length;
        };

        Initrd(uintptr_t Address);
        ~Initrd();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_INITRD_H__
