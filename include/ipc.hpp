#ifndef __FENNIX_KERNEL_IPC_H__
#define __FENNIX_KERNEL_IPC_H__

#include <types.h>
#include <filesystem.hpp>
#include <vector.hpp>
#include <memory.hpp>
#include <atomic.hpp>
#include <lock.hpp>

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
        Atomic<bool> Listening;
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
