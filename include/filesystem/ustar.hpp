#ifndef __FENNIX_KERNEL_FILESYSTEM_USTAR_H__
#define __FENNIX_KERNEL_FILESYSTEM_USTAR_H__

#include <types.h>

#include <filesystem.hpp>

namespace VirtualFileSystem
{
    class USTAR
    {
        enum FileType
        {
            REGULAR_FILE = '0',
            HARDLINK = '1',
            SYMLINK = '2',
            CHARDEV = '3',
            BLOCKDEV = '4',
            DIRECTORY = '5',
            FIFO = '6'
        };

        struct FileHeader
        {
            char name[100];
            char mode[8];
            char uid[8];
            char gid[8];
            char size[12];
            char mtime[12];
            char chksum[8];
            char typeflag[1];
            char link[100];
            char signature[6];
            char version[2];
            char owner[32];
            char group[32];
            char dev_maj[8];
            char dev_min[8];
            char prefix[155];
            char pad[12];
        };

    private:
        uint32_t getsize(const char *s)
        {
            uint32_t ret = 0;
            while (*s)
            {
                ret *= 8;
                ret += *s - '0';
                s++;
            }
            return ret;
        }

        int string2int(const char *str)
        {
            int res = 0;
            for (int i = 0; str[i] != '\0'; ++i)
                res = res * 10 + str[i] - '0';
            return res;
        }

    public:
        USTAR(uintptr_t Address, Virtual *vfs_ctx);
        ~USTAR();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_USTAR_H__
