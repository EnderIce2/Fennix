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

#ifndef __FENNIX_KERNEL_IPC_H__
#define __FENNIX_KERNEL_IPC_H__

#include <types.h>
#include <filesystem.hpp>
#include <memory.hpp>
#include <lock.hpp>
#include <vector>
#include <atomic>

namespace InterProcessCommunication
{
    typedef int IPCID;

    enum IPCType
    {
        IPCNone,
        IPCMessagePassing,
        IPCPort,
        IPCSharedMemory,
        IPCPipe,
        IPCSocket
    };

    enum IPCErrorCode
    {
        IPCError = -1,
        IPCSuccess,
        IPCNotListening,
        IPCTimeout,
        IPCInvalidCommand,
        IPCAlreadyAllocated,
        IPCNotAllocated,
        IPCIDInUse,
        IPCIDNotRegistered,
        IPCIDNotFound
    };

    struct IPCHandle
    {
        IPCID ID;
        VirtualFileSystem::Node *Node;
        void *Buffer;
        long Length;
        std::atomic_bool Listening;
    };

    class IPC
    {
    private:
        NewLock(IPCLock);
        IPCID NextID = 0;
        std::vector<IPCHandle *> Handles;
        Memory::MemMgr *mem;
        VirtualFileSystem::Node *IPCNode;
        void *Process;

    public:
        std::vector<IPCHandle *> GetHandles() { return Handles; }
        void Fork(IPC *Parent);
        IPCHandle *Create(IPCType Type, char UniqueToken[16]);
        IPCErrorCode Destroy(IPCID ID);
        IPCErrorCode Allocate(IPCID ID, long Size);
        IPCErrorCode Deallocate(IPCID ID);
        IPCErrorCode Read(IPCID ID, void *Buffer, long Size);
        IPCErrorCode Write(IPCID ID, void *Buffer, long Size);
        IPCErrorCode Listen(IPCID ID, bool Listen);
        IPCErrorCode Wait(IPCID ID);
        IPCHandle *SearchByToken(char UniqueToken[16]);
        int HandleSyscall(long Command, long Type, int ID, int Flags, void *Buffer, size_t Size);

        IPC(void *Process);
        ~IPC();
    };
}

#endif // !__FENNIX_KERNEL_IPC_H__
