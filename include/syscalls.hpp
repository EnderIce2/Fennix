#ifndef __FENNIX_KERNEL_SYSCALLS_H__
#define __FENNIX_KERNEL_SYSCALLS_H__

#include <types.h>

typedef struct SyscallsFrame
{
#if defined(a64)
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t InterruptNumber, ErrorCode, rip, cs, rflags, rsp, ss;
#elif defined(a32)
    uint32_t ebp, edi, esi, edx, ecx, ebx, eax;
    uint32_t InterruptNumber, ErrorCode, eip, cs, eflags, esp, ss;
#elif defined(aa64)
#endif
} SyscallsFrame;

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame);
uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame);

/**
 * @brief Initialize syscalls for the current CPU. (Function is available on x32, x64 & aarch64)
 */
void InitializeSystemCalls();

#endif // !__FENNIX_KERNEL_SYSCALLS_H__
