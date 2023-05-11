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
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t ReturnAddress;
    uint64_t CodeSegment;
    uint64_t Flags;
    uint64_t StackPointer;
    uint64_t StackSegment;
#elif defined(a32)
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;

    uint32_t ReturnAddress;
    uint32_t CodeSegment;
    uint32_t Flags;
    uint32_t StackPointer;
    uint32_t StackSegment;
#elif defined(aa64)
    uint32_t ReturnAddress;
    uint32_t StackPointer;
#endif
} SyscallsFrame;

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame);
uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame);

/**
 * @brief Initialize syscalls for the current CPU. (Function is available on x32, x64 & aarch64)
 */
void InitializeSystemCalls();

#endif // !__FENNIX_KERNEL_SYSCALLS_H__
