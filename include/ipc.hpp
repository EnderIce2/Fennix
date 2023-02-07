#ifndef __FENNIX_KERNEL_IPC_H__
#define __FENNIX_KERNEL_IPC_H__

#include <types.h>
#include <filesystem.hpp>
#include <vector.hpp>
#include <memory.hpp>
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
        IPCInvalidPort,
        IPCAlreadyAllocated,
        IPCNotAllocated,
        IPCIDInUse,
        IPCIDNotRegistered,
        IPCIDNotFound
    };

    struct IPCHandle
    {
        IPCID ID;
        long Length;
        uint8_t *Buffer;
        bool Listening;
        VirtualFileSystem::Node *Node;
        IPCErrorCode Error;
    };

    class IPC
    {
    private:
        NewLock(IPCLock);
        IPCID NextID = 0;
        Vector<IPCHandle *> Handles;
        Memory::MemMgr *mem;
        VirtualFileSystem::Node *IPCNode;
        void *Process;

    public:
        IPC(void *Process);
        ~IPC();

        int HandleSyscall(long Command, long Type, int ID, int Flags, void *Buffer, size_t Size);
        IPCHandle *Create(IPCType Type, char UniqueToken[16]);
        IPCErrorCode Destroy(IPCID ID);
        IPCErrorCode Read(IPCID ID, uint8_t *Buffer, long Size);
        IPCErrorCode Write(IPCID ID, uint8_t *Buffer, long Size);
        IPCErrorCode Listen(IPCID ID);
        IPCHandle *Wait(IPCID ID);
        IPCErrorCode Allocate(IPCID ID, long Size);
        IPCErrorCode Deallocate(IPCID ID);
    };
}

#endif // !__FENNIX_KERNEL_IPC_H__
