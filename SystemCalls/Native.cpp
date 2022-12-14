#include <syscalls.hpp>
#include <memory.hpp>

#include <debug.h>

#include "../syscalls.h"
#include "../kernel.h"

static uint64_t sys_exit(SyscallsFrame *Frame, uint64_t code)
{
    trace("Userspace thread %s(%lld) exited with code %#llx", TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID, code);
    TaskManager->GetCurrentThread()->ExitCode = code;
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
    return 0;
}

static int sys_print(SyscallsFrame *Frame, char Char, int Index)
{
    int ret = Display->Print(Char, Index, true);
#ifdef DEBUG
    Display->SetBuffer(Index);
#endif
    return ret;
}

static uint64_t sys_request_pages(SyscallsFrame *Frame, uint64_t Count)
{
    return (uint64_t)TaskManager->GetCurrentThread()->Memory->RequestPages(Count);
}

static uint64_t sys_free_pages(SyscallsFrame *Frame, uint64_t Address, uint64_t Count)
{
    TaskManager->GetCurrentThread()->Memory->FreePages((void *)Address, Count);
    return 0;
}

static uint64_t sys_kernelctl(SyscallsFrame *Frame, uint64_t Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4)
{
    fixme("KernelCTL: %lld", Command);
    return 0;
}

static void *NativeSyscallsTable[] = {
    [_Exit] = (void *)sys_exit,
    [_Print] = (void *)sys_print,

    [_RequestPages] = (void *)sys_request_pages,
    [_FreePages] = (void *)sys_free_pages,

    [_KernelCTL] = (void *)sys_kernelctl,
};

uint64_t HandleNativeSyscalls(SyscallsFrame *Frame)
{
#if defined(__amd64__)
    debug("rax: %#llx, rbx: %#llx, rcx: %#llx, rdx: %#llx, rsi: %#llx, rdi: %#llx, rbp: %#llx, r8: %#llx, r9: %#llx, r10: %#llx, r11: %#llx, r12: %#llx, r13: %#llx, r14: %#llx, r15: %#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx, Frame->rsi, Frame->rdi, Frame->rbp, Frame->r8, Frame->r9, Frame->r10, Frame->r11, Frame->r12, Frame->r13, Frame->r14, Frame->r15);
    if (Frame->rax > sizeof(NativeSyscallsTable))
    {
        fixme("Syscall %lld not implemented", Frame->rax);
        return -1;
    }

    uint64_t (*call)(uint64_t, ...) = reinterpret_cast<uint64_t (*)(uint64_t, ...)>(NativeSyscallsTable[Frame->rax]);
    if (!call)
    {
        error("Syscall %#llx failed.", Frame->rax);
        return -1;
    }
    debug("[%#lx]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )", Frame->rax, Frame->rdi, Frame->rsi, Frame->rdx, Frame->rcx, Frame->r8, Frame->r9);
    uint64_t ret = call((uint64_t)Frame, Frame->rdi, Frame->rsi, Frame->rdx, Frame->r10, Frame->r8, Frame->r9);
    Frame->rax = ret;
    return ret;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
}
