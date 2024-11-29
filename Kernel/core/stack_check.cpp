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

#include <types.h>
#include <debug.h>
#include <rand.hpp>

#include "../kernel.h"

extern __noreturn void HandleStackSmashing();
extern __noreturn void HandleBufferOverflow();

/* EXTERNC */ uintptr_t __stack_chk_guard = 0;

EXTERNC __no_stack_protector uintptr_t __stack_chk_guard_init(void)
{
	int MaxRetries = 0;
#if UINTPTR_MAX == UINT32_MAX
	uint32_t num;
Retry:
	num = Random::rand32();
	if (num < 0x1000 && MaxRetries++ < 10)
		goto Retry;
	return num;

#else
	uint64_t num;
Retry:
	num = Random::rand64();
	if (num < 0x100000 && MaxRetries++ < 10)
		goto Retry;
	return num;
#endif
}

EXTERNC __constructor __no_stack_protector void __guard_setup(void)
{
	debug("__guard_setup");
	if (__stack_chk_guard == 0)
		__stack_chk_guard = __stack_chk_guard_init();
	debug("Stack guard value: %ld", __stack_chk_guard);
}

EXTERNC __noreturn __no_stack_protector void __stack_chk_fail(void)
{
	CPU::PageTable(KernelPageTable);

	void *Stack = nullptr;
#if defined(__amd64__)
	asmv("movq %%rsp, %0"
		 : "=r"(Stack));
#elif defined(__i386__)
	asmv("movl %%esp, %0"
		 : "=r"(Stack));
#elif defined(__aarch64__)
	asmv("mov %%sp, %0"
		 : "=r"(Stack));
#endif
	error("Stack address: %#lx", Stack);

	HandleStackSmashing();
}

// https://github.com/gcc-mirror/gcc/blob/master/libssp/ssp.c
EXTERNC __noreturn nsa void __chk_fail(void)
{
	CPU::PageTable(KernelPageTable);

	HandleBufferOverflow();
}
