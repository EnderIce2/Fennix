#include <ipc.hpp>

#include <task.hpp>

#include "../kernel.h"
#include "../ipc.h"

namespace InterProcessCommunication
{
    IPCHandle *IPC::Create(IPCType Type, char UniqueToken[16])
    {
        SmartLock(this->IPCLock);
        IPCHandle *Hnd = (IPCHandle *)mem->RequestPages(TO_PAGES(sizeof(IPCHandle)));

        Hnd->ID = NextID++;
        Hnd->Node = vfs->Create(UniqueToken, VirtualFileSystem::NodeFlags::FILE, IPCNode);
        Hnd->Buffer = nullptr;
        Hnd->Length = 0;
        Hnd->Listening = false;

        Handles.push_back(Hnd);
        debug("Created IPC with ID %d", Hnd->ID);
        return Hnd;
    }

    IPCErrorCode IPC::Destroy(IPCID ID)
    {
        SmartLock(this->IPCLock);
        for (size_t i = 0; i < Handles.size(); i++)
        {
            if (Handles[i]->ID == ID)
            {
                vfs->Delete(Handles[i]->Node);
                mem->FreePages(Handles[i], TO_PAGES(sizeof(IPCHandle)));
                Handles.remove(i);
                debug("Destroyed IPC with ID %d", ID);
                return IPCSuccess;
            }
        }
        debug("Failed to destroy IPC with ID %d", ID);
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Allocate(IPCID ID, long Size)
    {
        SmartLock(this->IPCLock);
        if (Size < 0)
            return IPCError;

        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                if (Hnd->Buffer != nullptr || Hnd->Length != 0)
                    return IPCAlreadyAllocated;

                Hnd->Buffer = (uint8_t *)mem->RequestPages(TO_PAGES(Size));
                Hnd->Length = Size;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Deallocate(IPCID ID)
    {
        SmartLock(this->IPCLock);
        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                if (Hnd->Buffer == nullptr || Hnd->Length == 0)
                    return IPCNotAllocated;

                mem->FreePages(Hnd->Buffer, TO_PAGES(Hnd->Length));
                Hnd->Buffer = nullptr;
                Hnd->Length = 0;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Read(IPCID ID, void *Buffer, long Size)
    {
        SmartLock(this->IPCLock);
        if (Size < 0)
            return IPCError;

        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                if (Hnd->Listening)
                {
                    debug("IPC %d is listening", ID);
                    return IPCNotListening;
                }
                if (Hnd->Length < Size)
                {
                    debug("IPC %d is too small", ID);
                    return IPCError;
                }
                debug("IPC %d reading %d bytes", ID, Size);
                memcpy(Buffer, Hnd->Buffer, Size);
                debug("IPC read %d bytes", Size);
                return IPCSuccess;
            }
        }
        debug("IPC %d not found", ID);
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Write(IPCID ID, void *Buffer, long Size)
    {
        SmartLock(this->IPCLock);
        if (Size < 0)
        {
            debug("IPC %d is too small", ID);
            return IPCError;
        }

        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                if (!Hnd->Listening)
                {
                    debug("IPC %d is NOT listening", ID);
                    return IPCNotListening;
                }
                if (Hnd->Length < Size)
                {
                    debug("IPC %d is too small", ID);
                    return IPCError;
                }
                debug("IPC %d writing %d bytes", ID, Size);
                memcpy(Hnd->Buffer, Buffer, Size);
                Hnd->Listening = false;
                debug("IPC %d wrote %d bytes and now is %s", ID, Size, Hnd->Listening ? "listening" : "ready");
                return IPCSuccess;
            }
        }
        debug("IPC %d not found", ID);
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Listen(IPCID ID, bool Listen)
    {
        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                Hnd->Listening = Listen;
                debug("IPC %d is now set to %s", ID, Listen ? "listening" : "ready");
                return IPCSuccess;
            }
        }
        debug("IPC %d not found", ID);
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Wait(IPCID ID)
    {
        foreach (auto Hnd in Handles)
        {
            if (Hnd->ID == ID)
            {
                if (!CPU::Interrupts())
                    warn("Interrupts are disabled. This may cause a kernel hang.");
                debug("Waiting for IPC %d (now %s)", ID, Hnd->Listening ? "listening" : "ready");
                while (Hnd->Listening)
                    CPU::Halt();
                debug("IPC %d is ready", ID);
                return IPCSuccess;
            }
        }
        debug("IPC %d not found", ID);
        return IPCIDNotFound;
    }

    IPCHandle *IPC::SearchByToken(char UniqueToken[16])
    {
        foreach (auto Hnd in Handles)
        {
            if (strcmp(Hnd->Node->Name, UniqueToken) == 0)
            {
                debug("Found IPC with token %s", UniqueToken);
                return Hnd;
            }
        }
        debug("Failed to find IPC with token %s", UniqueToken);
        return nullptr;
    }

    int IPC::HandleSyscall(long Command, long Type, int ID, int Flags, void *Buffer, size_t Size)
    {
        switch (Command)
        {
        case IPC_CREATE:
        {
            char UniqueToken[16];
            if (Buffer != nullptr)
                strcpy(UniqueToken, (char *)Buffer);
            else
                snprintf(UniqueToken, 16, "IPC_%d", ID);
            IPCHandle *Hnd = this->Create((IPCType)Type, UniqueToken);
            this->Allocate(Hnd->ID, Size ? Size : PAGE_SIZE);
            return Hnd->ID;
        }
        case IPC_READ:
            return this->Read(ID, Buffer, Size);
        case IPC_WRITE:
            return TaskManager->GetProcessByID(Flags)->IPC->Write(ID, Buffer, Size);
        case IPC_DELETE:
        {
            this->Deallocate(ID);
            return this->Destroy(ID);
        }
        case IPC_WAIT:
            return this->Wait(ID);
        case IPC_LISTEN:
            return this->Listen(ID, Flags);
        default:
            return IPCInvalidCommand;
        }
        return IPCError;
    }

    IPC::IPC(void *Process)
    {
        this->Process = Process;
        mem = new Memory::MemMgr(nullptr, ((Tasking::PCB *)Process)->memDirectory);
        IPCNode = vfs->Create("ipc", VirtualFileSystem::NodeFlags::DIRECTORY, ((Tasking::PCB *)this->Process)->ProcessDirectory);
    }

    IPC::~IPC()
    {
        delete mem;
        vfs->Delete(IPCNode, true);
    }
}
