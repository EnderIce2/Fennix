#ifndef __FENNIX_KERNEL_IPC_H__
#define __FENNIX_KERNEL_IPC_H__

#include <types.h>

#include <lock.hpp>

namespace InterProcessCommunication
{
    typedef int IPCPort;

    enum IPCOperationType
    {
        IPCOperationNone,
        IPCOperationWrite,
        IPCOperationRead
    };

    enum IPCErrorCode
    {
        IPCUnknown,
        IPCSuccess,
        IPCNotListening,
        IPCTimeout,
        IPCInvalidPort,
        IPCPortInUse,
        IPCPortNotRegistered,
        IPCIDNotFound
    };

    typedef struct
    {
        int ID;
        long Length;
        uint8_t *Buffer;
        bool Listening;
        IPCOperationType Operation;
        IPCErrorCode Error;
        LockClass Lock;
    } IPCHandle;

    typedef struct
    {
        int ID;
        long Length;
        IPCOperationType Operation;
        IPCErrorCode Error;
        uint8_t *Buffer;

        // Reserved
        IPCHandle *HandleBuffer;
    } __attribute__((packed)) IPCSyscallHandle;

    struct IPCError
    {
        uint64_t ErrorCode;
    };

    class IPC
    {
    private:
    public:
        IPC();
        ~IPC();

        IPCHandle *RegisterHandle(IPCPort Port);
        IPCError Listen(IPCPort Port);
        IPCHandle *Wait(IPCPort Port);
        IPCError Read(unsigned long /* Tasking::UPID */ ID, IPCPort Port, uint8_t *&Buffer, long &Size);
        IPCError Write(unsigned long /* Tasking::UPID */ ID, IPCPort Port, uint8_t *Buffer, long Size);
    };
}

extern InterProcessCommunication::IPC *ipc;

#endif // !__FENNIX_KERNEL_IPC_H__
