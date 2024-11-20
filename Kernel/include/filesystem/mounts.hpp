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

#ifndef __FENNIX_KERNEL_FILESYSTEM_DEV_H__
#define __FENNIX_KERNEL_FILESYSTEM_DEV_H__

#include <types.h>

#include <filesystem.hpp>

namespace VirtualFileSystem
{
    /* Manage /system/dev */
    class Device
    {
    public:
        Node *AddFileSystem(FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
        Device();
        ~Device();
    };

    /* Manage /system/mnt */
    class Mount
    {
    public:
        Node *MountFileSystem(FileSystemOperations *Operator, uint64_t Mode, const char *Name);
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
        Node *AddDriver(struct FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
        Driver();
        ~Driver();
    };

    /* Manage /system/net */
    class Network
    {
    public:
        Node *AddNetworkCard(struct FileSystemOperations *Operator, uint64_t Mode, const char *Name, int Flags);
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
        void SetFrameBufferData(uintptr_t Address, size_t Size, uint32_t Width, uint32_t Height, uint32_t PixelsPerScanLine);
        FB();
        ~FB();
    };
}

#endif // !__FENNIX_KERNEL_FILESYSTEM_DEV_H__
