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
