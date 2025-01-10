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
#if defined(__amd64__)
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;

	uint64_t bp;
	uint64_t di;
	uint64_t si;
	uint64_t dx;
	uint64_t cx;
	uint64_t bx;
	uint64_t ax;

	uint64_t ReturnAddress;
	uint64_t CodeSegment;
	uint64_t Flags;
	uint64_t StackPointer;
	uint64_t StackSegment;
#elif defined(__i386__)
	uint32_t bp;
	uint32_t di;
	uint32_t si;
	uint32_t dx;
	uint32_t cx;
	uint32_t bx;
	uint32_t ax;

	uint32_t ReturnAddress;
	uint32_t CodeSegment;
	uint32_t Flags;
	uint32_t StackPointer;
	uint32_t StackSegment;
#elif defined(__aarch64__)
	uint64_t ReturnAddress; /* x0 */
	uint64_t x[30];
	uint64_t StackPointer;
	uint64_t ExceptionLinkRegister;
	uint64_t ExceptionSyndromeRegister;
	uint64_t FaultAddressRegister;
	uint64_t SavedProgramStatusRegister;
#endif

	uintptr_t ReturnValue() const
	{
#if defined(__amd64__)
		return ax;
#elif defined(__i386__)
		return ax;
#elif defined(__aarch64__)
		return x[0];
#endif
	}

	uintptr_t Arg0() const
	{
#if defined(__amd64__)
		return di;
#elif defined(__i386__)
		return di;
#elif defined(__aarch64__)
		return x[0];
#endif
	}

	uintptr_t Arg1() const
	{
#if defined(__amd64__)
		return si;
#elif defined(__i386__)
		return cx;
#elif defined(__aarch64__)
		return x[1];
#endif
	}

	uintptr_t Arg2() const
	{
#if defined(__amd64__)
		return dx;
#elif defined(__i386__)
		return dx;
#elif defined(__aarch64__)
		return x[2];
#endif
	}

	uintptr_t Arg3() const
	{
#if defined(__amd64__)
		return r10;
#elif defined(__i386__)
		return si;
#elif defined(__aarch64__)
		return x[3];
#endif
	}

	uintptr_t Arg4() const
	{
#if defined(__amd64__)
		return r8;
#elif defined(__i386__)
		return di;
#elif defined(__aarch64__)
		return x[4];
#endif
	}

	uintptr_t Arg5() const
	{
#if defined(__amd64__)
		return r9;
#elif defined(__i386__)
		return bp;
#elif defined(__aarch64__)
		return x[5];
#endif
	}
} SyscallsFrame;
#define SysFrm SyscallsFrame

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame);
uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame);

/**
 * @brief Initialize syscalls for the current CPU. (Function is available on x32, x64 & aarch64)
 */
void InitializeSystemCalls();

#endif // !__FENNIX_KERNEL_SYSCALLS_H__
