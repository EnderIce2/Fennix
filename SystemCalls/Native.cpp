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

#include <syscalls.hpp>
#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>

#include <debug.h>

#include "../syscalls.h"
#include "../kernel.h"

#include "../../Userspace/libs/include/sysbase.h"
#include "../ipc.h"

using InterProcessCommunication::IPC;
using InterProcessCommunication::IPCID;
using Tasking::Token;
using Tasking::TTL;
using Tasking::TTL::Trusted;
using Tasking::TTL::TrustedByKernel;
using Tasking::TTL::UnknownTrustLevel;
using Tasking::TTL::Untrusted;

static inline bool CheckTrust(int TrustLevel)
{
    // SmartTimeoutLock(SyscallsLock, 10000); - This is already done in the caller
    Token token = TaskManager->GetCurrentThread()->Security.UniqueToken;
    if (TaskManager->GetSecurityManager()->IsTokenTrusted(token, TrustLevel))
        return true;

    warn("Thread %s(%lld) tried to access a system call \"%s\" with insufficient trust level",
         TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID,
         KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_extract_return_addr(__builtin_return_address(0))));
    debug("Token: token=%#lx, trust=%d", token, TaskManager->GetSecurityManager()->GetTokenTrustLevel(token));
    return false;
}

static int sys_exit(SyscallsFrame *Frame, int code)
{
    /* Allow everyone to exit */
    if (!CheckTrust(TrustedByKernel | Trusted | Untrusted | UnknownTrustLevel))
        return SYSCALL_ACCESS_DENIED;

    trace("Userspace thread %s(%lld) exited with code %#llx", TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID, code);
    TaskManager->GetCurrentThread()->ExitCode = code;
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
    UNUSED(Frame);
    return SYSCALL_OK;
}

static int sys_print(SyscallsFrame *Frame, char Char, int Index)
{
    /* Only trusted threads can write to the kernel console */
    if (!CheckTrust(TrustedByKernel | Trusted))
        return SYSCALL_ACCESS_DENIED;

    char ret = Display->Print(Char, Index, true);
    if (!Config.BootAnimation && Index == 0)
#ifdef DEBUG
        Display->SetBuffer(Index);
#else
        if (Char == '\n')
            Display->SetBuffer(Index);
#endif
    UNUSED(Frame);
    return ret;
}

static uintptr_t sys_request_pages(SyscallsFrame *Frame, size_t Count)
{
    /* Allow everyone to request pages */
    if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
        return SYSCALL_ACCESS_DENIED;
    UNUSED(Frame);
    return (uintptr_t)TaskManager->GetCurrentThread()->Memory->RequestPages(Count, true);
}

static int sys_free_pages(SyscallsFrame *Frame, uintptr_t Address, size_t Count)
{
    /* Allow everyone to free pages */
    if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
        return SYSCALL_ACCESS_DENIED;
    TaskManager->GetCurrentThread()->Memory->FreePages((void *)Address, Count);
    UNUSED(Frame);
    return SYSCALL_OK;
}

static int sys_detach_address(SyscallsFrame *Frame, uintptr_t Address)
{
    /* Only trusted threads can detach allocated addresses */
    if (!CheckTrust(TrustedByKernel | Trusted))
        return SYSCALL_ACCESS_DENIED;
    TaskManager->GetCurrentThread()->Memory->DetachAddress((void *)Address);
    UNUSED(Frame);
    return SYSCALL_OK;
}

static uintptr_t sys_kernelctl(SyscallsFrame *Frame, enum KCtl Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4)
{
    /* Only trusted threads can use kernelctl */
    if (!CheckTrust(TrustedByKernel | Trusted))
        return SYSCALL_ACCESS_DENIED;

    switch (Command)
    {
    case KCTL_GET_PID:
        return TaskManager->GetCurrentThread()->Parent->ID;
    case KCTL_GET_TID:
        return TaskManager->GetCurrentThread()->ID;
    case KCTL_GET_PAGE_SIZE:
        return PAGE_SIZE;
    case KCTL_IS_CRITICAL:
        return TaskManager->GetCurrentThread()->Security.IsCritical;
    case KCTL_REGISTER_ELF_LIB:
    {
        char *Identifier = (char *)Arg1;
        const char *Path = (const char *)Arg2;

        if (!Identifier || !Path)
            return SYSCALL_INVALID_ARGUMENT;

        std::string FullPath = Path;
        int retries = 0;
    RetryReadPath:
        debug("KCTL_REGISTER_ELF_LIB: Trying to open %s", FullPath.c_str());
        std::shared_ptr<VirtualFileSystem::File> f = vfs->Open(FullPath.c_str());

        if (f->Status != VirtualFileSystem::FileStatus::OK)
        {
            FullPath.clear();
            switch (retries)
            {
            case 0:
                FullPath = "/system/lib/";
                break;
            case 1:
                FullPath = "/system/lib64/";
                break;
            case 2:
                FullPath = "/system/";
                break;
            case 3:
            {
                // TODO: Check process binary path
                break;
            }
            default:
            {
                vfs->Close(f);
                return SYSCALL_INVALID_ARGUMENT;
            }
            }
            FullPath += Path;
            vfs->Close(f);
            retries++;
            goto RetryReadPath;
        }

        vfs->Close(f);
        if (Execute::AddLibrary(Identifier, (void *)f->node->Address, f->node->Length))
            return SYSCALL_OK;
        else
            return SYSCALL_INTERNAL_ERROR;
    }
    case KCTL_GET_ELF_LIB_FILE:
    {
        char *Identifier = (char *)Arg1;
        if (!Identifier)
            return 0;

        Execute::SharedLibraries lib = Execute::GetLibrary(Identifier);
        if (!lib.Address)
            debug("Failed to get library address %#lx", (uintptr_t)lib.Address);

        debug("Returning library address %#lx", (uintptr_t)lib.Address);
        return (uintptr_t)lib.Address;
    }
    case KCTL_GET_ELF_LIB_MEMORY_IMAGE:
    {
        char *Identifier = (char *)Arg1;
        if (!Identifier)
            return 0;

        Execute::SharedLibraries lib = Execute::GetLibrary(Identifier);

        if (!lib.MemoryImage)
            debug("Failed to get library memory image %#lx", (uintptr_t)lib.MemoryImage);

        debug("Returning memory image %#lx", (uintptr_t)lib.MemoryImage);
        return (uintptr_t)lib.MemoryImage;
    }
    default:
    {
        warn("KernelCTL: Unknown command: %lld", Command);
        return SYSCALL_INVALID_ARGUMENT;
    }
    }

    UNUSED(Arg1);
    UNUSED(Arg2);
    UNUSED(Arg3);
    UNUSED(Arg4);
    UNUSED(Frame);
}

static int sys_ipc(SyscallsFrame *Frame, enum IPCCommand Command, enum IPCType Type, int ID, int Flags, void *Buffer, size_t Size)
{
    /* Allow everyone to use IPC */
    if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
        return SYSCALL_ACCESS_DENIED;
    UNUSED(Frame);
    return TaskManager->GetCurrentProcess()->IPC->HandleSyscall(Command, Type, ID, Flags, Buffer, Size);
}

static int sys_file_open(SyscallsFrame *Frame)
{
    fixme("sys_file_open: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_file_close(SyscallsFrame *Frame)
{
    fixme("sys_file_close: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_file_read(SyscallsFrame *Frame)
{
    fixme("sys_file_read: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_file_write(SyscallsFrame *Frame)
{
    fixme("sys_file_write: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_file_seek(SyscallsFrame *Frame)
{
    fixme("sys_file_seek: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_file_status(SyscallsFrame *Frame)
{
    fixme("sys_file_status: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sleep(SyscallsFrame *Frame, uint64_t Milliseconds)
{
    if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
        return SYSCALL_ACCESS_DENIED;
    TaskManager->Sleep(Milliseconds);
    return 0;
}

static int sys_wait(SyscallsFrame *Frame)
{
    fixme("sys_wait: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill(SyscallsFrame *Frame)
{
    fixme("sys_kill: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn(SyscallsFrame *Frame)
{
    fixme("sys_spawn: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn_thread(SyscallsFrame *Frame)
{
    fixme("sys_spawn_thread: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_thread_list_of_process(SyscallsFrame *Frame)
{
    fixme("sys_get_thread_list_of_process: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_process(SyscallsFrame *Frame)
{
    fixme("sys_get_current_process: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_thread(SyscallsFrame *Frame)
{
    fixme("sys_get_current_thread: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_process_by_pid(SyscallsFrame *Frame)
{
    fixme("sys_get_process_by_pid: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_thread_by_tid(SyscallsFrame *Frame)
{
    fixme("sys_get_thread_by_tid: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_process(SyscallsFrame *Frame)
{
    fixme("sys_kill_process: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_thread(SyscallsFrame *Frame)
{
    fixme("sys_kill_thread: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_process(SyscallsFrame *Frame)
{
    fixme("sys_sys_reserved_create_process: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_thread(SyscallsFrame *Frame)
{
    fixme("sys_sys_reserved_create_thread: %#lx", Frame);
    return SYSCALL_NOT_IMPLEMENTED;
}

static void *NativeSyscallsTable[] = {
    [_Exit] = (void *)sys_exit,
    [_Print] = (void *)sys_print,

    [_RequestPages] = (void *)sys_request_pages,
    [_FreePages] = (void *)sys_free_pages,
    [_DetachAddress] = (void *)sys_detach_address,

    [_KernelCTL] = (void *)sys_kernelctl,
    [_IPC] = (void *)sys_ipc,

    [_FileOpen] = (void *)sys_file_open,
    [_FileClose] = (void *)sys_file_close,
    [_FileRead] = (void *)sys_file_read,
    [_FileWrite] = (void *)sys_file_write,
    [_FileSeek] = (void *)sys_file_seek,
    [_FileStatus] = (void *)sys_file_status,

    [_Sleep] = (void *)sys_sleep,
    [_Wait] = (void *)sys_wait,
    [_Kill] = (void *)sys_kill,
    [_Spawn] = (void *)sys_spawn,
    [_SpawnThread] = (void *)sys_spawn_thread,
    [_GetThreadListOfProcess] = (void *)sys_get_thread_list_of_process,
    [_GetCurrentProcess] = (void *)sys_get_current_process,
    [_GetCurrentThread] = (void *)sys_get_current_thread,
    [_GetProcessByPID] = (void *)sys_get_process_by_pid,
    [_GetThreadByTID] = (void *)sys_get_thread_by_tid,
    [_KillProcess] = (void *)sys_kill_process,
    [_KillThread] = (void *)sys_kill_thread,
    [_SysReservedCreateProcess] = (void *)sys_sys_reserved_create_process,
    [_SysReservedCreateThread] = (void *)sys_sys_reserved_create_thread,
};

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame)
{
#if defined(a64)
    if (Frame->rax > sizeof(NativeSyscallsTable))
    {
        fixme("Syscall %ld not implemented", Frame->rax);
        return SYSCALL_NOT_IMPLEMENTED;
    }

    uintptr_t (*call)(uintptr_t, ...) = reinterpret_cast<uintptr_t (*)(uintptr_t, ...)>(NativeSyscallsTable[Frame->rax]);
    if (!call)
    {
        error("Syscall %#lx failed.", Frame->rax);
        return SYSCALL_INTERNAL_ERROR;
    }

    debug("[%#lx]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
          Frame->rax,
          Frame->rdi, Frame->rsi, Frame->rdx, Frame->rcx, Frame->r8, Frame->r9);

    uintptr_t ret = call((uintptr_t)Frame, Frame->rdi, Frame->rsi, Frame->rdx, Frame->r10, Frame->r8, Frame->r9);
    Frame->rax = ret;
    return ret;
#elif defined(a32)
    if (Frame->eax > sizeof(NativeSyscallsTable))
    {
        fixme("Syscall %ld not implemented", Frame->eax);
        return SYSCALL_NOT_IMPLEMENTED;
    }

    /* ... */

    return SYSCALL_INTERNAL_ERROR;
#elif defined(aa64)
    return SYSCALL_INTERNAL_ERROR;
#endif
}
