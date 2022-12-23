#include <syscalls.hpp>
#include <memory.hpp>

#include <debug.h>

#include "../syscalls.h"
#include "../kernel.h"

#include "../../Userspace/libs/include/sysbase.h"

static int sys_exit(SyscallsFrame *Frame, int code)
{
    trace("Userspace thread %s(%lld) exited with code %#llx", TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID, code);
    TaskManager->GetCurrentThread()->ExitCode = code;
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
    UNUSED(Frame);
    return 0;
}

static int sys_print(SyscallsFrame *Frame, char Char, int Index)
{
    int ret = Display->Print(Char, Index, true);
#ifdef DEBUG
    Display->SetBuffer(Index);
#endif
    UNUSED(Frame);
    return ret;
}

static uintptr_t sys_request_pages(SyscallsFrame *Frame, size_t Count)
{
    UNUSED(Frame);
    return (uintptr_t)TaskManager->GetCurrentThread()->Memory->RequestPages(Count);
}

static int sys_free_pages(SyscallsFrame *Frame, uintptr_t Address, size_t Count)
{
    TaskManager->GetCurrentThread()->Memory->FreePages((void *)Address, Count);
    UNUSED(Frame);
    return 0;
}

static int sys_kernelctl(SyscallsFrame *Frame, int Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4)
{
    switch (Command)
    {
    case KCTL_GET_PID:
        return TaskManager->GetCurrentThread()->Parent->ID;
    case KCTL_GET_TID:
        return TaskManager->GetCurrentThread()->ID;
    case KCTL_GET_PAGE_SIZE:
        return PAGE_SIZE;
    default:
    {
        warn("KernelCTL: Unknown command: %lld", Command);
        return -1;
    }
    }

    UNUSED(Arg1);
    UNUSED(Arg2);
    UNUSED(Arg3);
    UNUSED(Arg4);
    UNUSED(Frame);
    return -1;
}

static int sys_file_open(SyscallsFrame *Frame)
{
    fixme("sys_file_open: %#lx", Frame);
    return -1;
}

static int sys_file_close(SyscallsFrame *Frame)
{
    fixme("sys_file_close: %#lx", Frame);
    return -1;
}

static int sys_file_read(SyscallsFrame *Frame)
{
    fixme("sys_file_read: %#lx", Frame);
    return -1;
}

static int sys_file_write(SyscallsFrame *Frame)
{
    fixme("sys_file_write: %#lx", Frame);
    return -1;
}

static int sys_file_seek(SyscallsFrame *Frame)
{
    fixme("sys_file_seek: %#lx", Frame);
    return -1;
}

static int sys_file_status(SyscallsFrame *Frame)
{
    fixme("sys_file_status: %#lx", Frame);
    return -1;
}

static int sys_wait(SyscallsFrame *Frame)
{
    fixme("sys_wait: %#lx", Frame);
    return -1;
}

static int sys_kill(SyscallsFrame *Frame)
{
    fixme("sys_kill: %#lx", Frame);
    return -1;
}

static int sys_spawn(SyscallsFrame *Frame)
{
    fixme("sys_spawn: %#lx", Frame);
    return -1;
}

static int sys_spawn_thread(SyscallsFrame *Frame)
{
    fixme("sys_spawn_thread: %#lx", Frame);
    return -1;
}

static int sys_get_thread_list_of_process(SyscallsFrame *Frame)
{
    fixme("sys_get_thread_list_of_process: %#lx", Frame);
    return -1;
}

static int sys_get_current_process(SyscallsFrame *Frame)
{
    fixme("sys_get_current_process: %#lx", Frame);
    return -1;
}

static int sys_get_current_thread(SyscallsFrame *Frame)
{
    fixme("sys_get_current_thread: %#lx", Frame);
    return -1;
}

static int sys_get_process_by_pid(SyscallsFrame *Frame)
{
    fixme("sys_get_process_by_pid: %#lx, %#lx, %d", Frame, Buffer, Count);
    return -1;
}

static int sys_get_thread_by_tid(SyscallsFrame *Frame)
{
    fixme("sys_get_thread_by_tid: %#lx, %#lx, %d", Frame, Buffer, Count);
    return -1;
}

static int sys_kill_process(SyscallsFrame *Frame)
{
    fixme("sys_kill_process: %#lx", Frame);
    return -1;
}

static int sys_kill_thread(SyscallsFrame *Frame)
{
    fixme("sys_kill_thread: %#lx", Frame);
    return -1;
}

static int sys_sys_reserved_create_process(SyscallsFrame *Frame)
{
    fixme("sys_sys_reserved_create_process: %#lx", Frame);
    return -1;
}

static int sys_sys_reserved_create_thread(SyscallsFrame *Frame)
{
    fixme("sys_sys_reserved_create_thread: %#lx", Frame);
    return -1;
}

static void *NativeSyscallsTable[] = {
    [_Exit] = (void *)sys_exit,
    [_Print] = (void *)sys_print,

    [_RequestPages] = (void *)sys_request_pages,
    [_FreePages] = (void *)sys_free_pages,

    [_KernelCTL] = (void *)sys_kernelctl,

    [_FileOpen] = (void *)sys_file_open,
    [_FileClose] = (void *)sys_file_close,
    [_FileRead] = (void *)sys_file_read,
    [_FileWrite] = (void *)sys_file_write,
    [_FileSeek] = (void *)sys_file_seek,
    [_FileStatus] = (void *)sys_file_status,

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
#if defined(__amd64__)
    // debug("rax: %#llx, rbx: %#llx, rcx: %#llx, rdx: %#llx, rsi: %#llx, rdi: %#llx, rbp: %#llx, r8: %#llx, r9: %#llx, r10: %#llx, r11: %#llx, r12: %#llx, r13: %#llx, r14: %#llx, r15: %#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx, Frame->rsi, Frame->rdi, Frame->rbp, Frame->r8, Frame->r9, Frame->r10, Frame->r11, Frame->r12, Frame->r13, Frame->r14, Frame->r15);
    if (Frame->rax > sizeof(NativeSyscallsTable))
    {
        fixme("Syscall %lld not implemented", Frame->rax);
        return -1;
    }

    uintptr_t (*call)(uintptr_t, ...) = reinterpret_cast<uintptr_t (*)(uintptr_t, ...)>(NativeSyscallsTable[Frame->rax]);
    if (!call)
    {
        error("Syscall %#llx failed.", Frame->rax);
        return -1;
    }
    debug("[%#lx]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )", Frame->rax, Frame->rdi, Frame->rsi, Frame->rdx, Frame->rcx, Frame->r8, Frame->r9);
    uintptr_t ret = call((uintptr_t)Frame, Frame->rdi, Frame->rsi, Frame->rdx, Frame->r10, Frame->r8, Frame->r9);
    Frame->rax = ret;
    return ret;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
}
