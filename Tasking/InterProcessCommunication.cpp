#include <ipc.hpp>

#include <task.hpp>

#include "../kernel.h"

namespace InterProcessCommunication
{
    IPCHandle *IPC::Create(IPCType Type, char UniqueToken[16])
    {
        SmartLock(IPCLock);

        IPCHandle *Handle = (IPCHandle *)mem->RequestPages(TO_PAGES(sizeof(IPCHandle)));
        Handle->ID = NextID++;
        Handle->Node = vfs->Create(UniqueToken, VirtualFileSystem::NodeFlags::FILE, IPCNode);
        Handle->Node->Address = (uintptr_t)mem->RequestPages(TO_PAGES(sizeof(4096)));
        Handle->Node->Length = 4096;
        Handles.push_back(Handle);
        return Handle;
    }

    IPCErrorCode IPC::Destroy(IPCID ID)
    {
        SmartLock(IPCLock);
        for (size_t i = 0; i < Handles.size(); i++)
        {
            if (Handles[i]->ID == ID)
            {
                mem->FreePages(Handles[i], TO_PAGES(sizeof(IPCHandle)));
                Handles.remove(i);
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Read(IPCID ID, uint8_t *Buffer, long Size)
    {
        SmartLock(IPCLock);
        if (Size < 0)
            return IPCError;

        foreach (auto Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                if (Handle->Listening)
                    return IPCNotListening;
                if (Handle->Length < Size)
                    return IPCError;
                memcpy(Buffer, Handle->Buffer, Size);
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Write(IPCID ID, uint8_t *Buffer, long Size)
    {
        SmartLock(IPCLock);
        if (Size < 0)
            return IPCError;

        foreach (auto Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                if (!Handle->Listening)
                    return IPCNotListening;
                if (Handle->Length < Size)
                    return IPCError;
                memcpy(Handle->Buffer, Buffer, Size);
                Handle->Listening = false;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Listen(IPCID ID)
    {
        SmartLock(IPCLock);
        foreach (auto Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                Handle->Listening = true;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCHandle *IPC::Wait(IPCID ID)
    {
        SmartLock(IPCLock);
        foreach (auto &Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                while (Handle->Listening)
                    CPU::Pause();
                return Handle;
            }
        }
        return nullptr;
    }

    IPCErrorCode IPC::Allocate(IPCID ID, long Size)
    {
        SmartLock(IPCLock);
        if (Size < 0)
            return IPCError;

        foreach (auto Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                if (Handle->Buffer != nullptr || Handle->Length != 0)
                    return IPCAlreadyAllocated;

                Handle->Buffer = (uint8_t *)mem->RequestPages(TO_PAGES(Size));
                Handle->Length = Size;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    IPCErrorCode IPC::Deallocate(IPCID ID)
    {
        SmartLock(IPCLock);
        foreach (auto Handle in Handles)
        {
            if (Handle->ID == ID)
            {
                if (Handle->Buffer == nullptr || Handle->Length == 0)
                    return IPCNotAllocated;

                mem->FreePages(Handle->Buffer, TO_PAGES(Handle->Length));
                Handle->Buffer = nullptr;
                Handle->Length = 0;
                return IPCSuccess;
            }
        }
        return IPCIDNotFound;
    }

    int IPC::HandleSyscall(long Command, long Type, int ID, int Flags, void *Buffer, size_t Size)
    {
        return 0;
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
