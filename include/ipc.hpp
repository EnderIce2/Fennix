#ifndef __FENNIX_KERNEL_IPC_H__
#define __FENNIX_KERNEL_IPC_H__

#include <types.h>

#include <lock.hpp>

namespace InterProcessCommunication
{
    typedef unsigned int IPCPort;

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
        IPCPortNotRegistered
    };

    typedef struct
    {
        int ID;
        int Length;
        void *Buffer;
        bool Listening;
        IPCOperationType Type;
        IPCErrorCode Error;
        LockClass Lock;
    } IPCHandle;

    typedef struct
    {
        int ID;
        int Length;
        IPCOperationType Type;
        IPCErrorCode Error;
        void *Buffer;

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
        IPCHandle *Wait(IPCPort port);
        IPCError Read(int pid, IPCPort port, void *buf, int size);
        IPCError Write(int pid, IPCPort port, void *buf, int size);
    };
}

extern InterProcessCommunication::IPC *ipc;

#endif // !__FENNIX_KERNEL_IPC_H__
