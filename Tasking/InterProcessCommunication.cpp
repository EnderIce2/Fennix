#include <ipc.hpp>

#include <lock.hpp>
#include <task.hpp>

#include "../kernel.h"

NewLock(IPCLock);

InterProcessCommunication::IPC *ipc = nullptr;

namespace InterProcessCommunication
{
    IPCHandle *IPC::RegisterHandle(IPCPort Port)
    {
        SmartLock(IPCLock);
        if (Port == 0)
            return nullptr;

        Tasking::PCB *pcb = TaskManager->GetCurrentProcess();

        if (pcb->IPCHandles->Get((int)Port) != 0)
            return nullptr;

        IPCHandle *handle = new IPCHandle;
        handle->ID = -1;
        handle->Buffer = nullptr;
        handle->Length = 0;
        handle->Operation = IPCOperationNone;
        handle->Listening = 0;
        handle->Error = IPCUnknown;
        pcb->IPCHandles->AddNode(Port, (uint64_t)handle);
        return handle;
    }

    IPCError IPC::Listen(IPCPort Port)
    {
        SmartLock(IPCLock);
        if (Port == 0)
            return IPCError{IPCInvalidPort};

        Tasking::PCB *pcb = TaskManager->GetCurrentProcess();

        if (pcb->IPCHandles->Get((int)Port) == 0)
            return IPCError{IPCPortNotRegistered};

        IPCHandle *handle = (IPCHandle *)pcb->IPCHandles->Get((int)Port);
        handle->Listening = 1;
        return IPCError{IPCSuccess};
    }

    IPCHandle *IPC::Wait(IPCPort Port)
    {
        SmartLock(IPCLock);
        if (Port == 0)
            return nullptr;

        Tasking::PCB *pcb = TaskManager->GetCurrentProcess();

        if (pcb->IPCHandles->Get((int)Port) == 0)
            return nullptr;

        IPCHandle *handle = (IPCHandle *)pcb->IPCHandles->Get((int)Port);

        while (handle->Listening == 1)
            CPU::Pause();

        return handle;
    }

    IPCError IPC::Read(Tasking::UPID ID, IPCPort Port, uint8_t *&Buffer, long &Size)
    {
        SmartLock(IPCLock);
        if (Port == 0)
            return IPCError{IPCInvalidPort};

        Tasking::PCB *pcb = TaskManager->GetCurrentProcess();

        if (pcb->IPCHandles->Get((int)Port) == 0)
            return IPCError{IPCInvalidPort};

        IPCHandle *handle = (IPCHandle *)pcb->IPCHandles->Get((int)Port);

        if (handle->Listening == 0)
            return IPCError{IPCPortInUse};

        Buffer = handle->Buffer;
        Size = handle->Length;
        handle->Operation = IPCOperationRead;
        handle->Listening = 1;
        handle->Error = IPCSuccess;

        return IPCError{IPCSuccess};
    }

    IPCError IPC::Write(Tasking::UPID ID, IPCPort Port, uint8_t *Buffer, long Size)
    {
        SmartLock(IPCLock);
        if (Port == 0)
            return IPCError{IPCInvalidPort};

        Vector<Tasking::PCB *> Processes = TaskManager->GetProcessList();

        for (uint64_t i = 0; i < Processes.size(); i++)
        {
            Tasking::PCB *pcb = Processes[i];

            if (pcb->ID == ID)
            {
                if (pcb->IPCHandles->Get((int)Port) == 0)
                    return IPCError{IPCInvalidPort};

                IPCHandle *handle = (IPCHandle *)pcb->IPCHandles->Get((int)Port);

                if (handle->Listening == 0)
                    return IPCError{IPCNotListening};

                handle->Buffer = Buffer;
                handle->Length = Size;
                handle->Operation = IPCOperationWrite;
                handle->Listening = 0;
                handle->Error = IPCSuccess;
            }
        }

        return IPCError{IPCIDNotFound};
    }

    IPC::IPC()
    {
        SmartLock(IPCLock);
        trace("Starting IPC Service...");
    }

    IPC::~IPC()
    {
    }
}
