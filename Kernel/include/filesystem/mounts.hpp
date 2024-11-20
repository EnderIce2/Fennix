#ifndef __FENNIX_KERNEL_FILESYSTEM_DEV_H__
#define __FENNIX_KERNEL_FILESYSTEM_DEV_H__

#include <types.h>

#include <filesystem.hpp>

namespace FileSystem
{
    /* Manage /system/dev */
    class Device
    {
    public:
        FileSystemNode *AddFileSystem(FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
        Device();
        ~Device();
    };

    /* Manage /system/mnt */
    class Mount
    {
    public:
        FileSystemNode *MountFileSystem(FileSystemOperations *Operator, uint64_t Mode, const char *Name);
        void DetectAndMountFS(void *drive);
        Mount();
        ~Mount();
    };

    /* Manage /system/prc */
    class Process
    {
    public:
        Process();
        ~Process();
    };

    /* Manage /system/drv */
    class Driver
    {
    public:
        FileSystemNode *AddDriver(struct FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
        Driver();
        ~Driver();
    };

    /* Manage /system/net */
    class Network
    {
    public:
        FileSystemNode *AddNetworkCard(struct FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
        Network();
        ~Network();
    };

    /* Manage /system/dev/serialX */
    class Serial
    {
    public:
        Serial();
        ~Serial();
    };

    /* Manage /system/dev/random */
    class Random
    {
    public:
        Random();
        ~Random();
    };

    /* Manage /system/dev/null */
    class Null
    {
    public:
        Null();
        ~Null();
    };

    /* Manage /system/dev/zero */
    class Zero
    {
    public:
        Zero();
        ~Zero();
    };

    /* Manage /system/dev/fbX */
    class FB
    {
    public:
        void SetFrameBufferData(uint64_t Address, uint64_t Size, uint32_t Width, uint32_t Height, uint32_t PixelsPerScanLine);
        FB();
        ~FB();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_DEV_H__
