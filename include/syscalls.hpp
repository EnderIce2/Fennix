#ifndef __FENNIX_KERNEL_SYSCALLS_H__
#define __FENNIX_KERNEL_SYSCALLS_H__

#include <types.h>

typedef struct SyscallsFrame
{
#if defined(__amd64__)
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t InterruptNumber, ErrorCode, rip, cs, rflags, rsp, ss;
#elif defined(__i386__)
    uint32_t ebp, edi, esi, edx, ecx, ebx, eax;
    uint32_t InterruptNumber, ErrorCode, eip, cs, eflags, esp, ss;
#elif defined(__aarch64__)
#endif
} SyscallsFrame;

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame);
uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame);

/**
 * @brief Initialize syscalls for the current CPU. (Function is available on x32, x64 & aarch64)
 */
void InitializeSystemCalls();

#endif // !__FENNIX_KERNEL_SYSCALLS_H__
