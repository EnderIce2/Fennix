/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#include <bits/syscalls.h>
#include <bits/libc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <stddef.h>

#ifndef FENNIX_DYNAMIC_LOADER
export __attribute__((naked, used, no_stack_protector)) void *__tls_get_addr(void *__data)
{
#warning "__tls_get_addr not implemented"
#if defined(__amd64__) || defined(__i386__)
	__asm__("ud2");
#endif
}

int __init_pthread(void)
{
	__pthread *ptr = (__pthread *)syscall6(sys_mmap, 0,
											0x1000,
											PROT_READ | PROT_WRITE,
											MAP_ANONYMOUS | MAP_PRIVATE,
											-1, 0);
	syscall2(sys_prctl, 0x1002, ptr);
	ptr->Self = ptr;
	ptr->CurrentError = 0;
	return 0;
}
#endif
