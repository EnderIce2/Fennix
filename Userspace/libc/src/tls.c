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

#include <fennix/syscalls.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stddef.h>

#include <fennix/syscalls.h>

export __attribute__((naked, used, no_stack_protector)) void *__tls_get_addr(void *__data)
{
	__asm__("ud2");
}

int __init_pthread(void)
{
	__pthread *ptr = (__pthread *)call_mmap(0,
											0x1000,
											__SYS_PROT_READ | __SYS_PROT_WRITE,
											__SYS_MAP_ANONYMOUS | __SYS_MAP_PRIVATE,
											-1, 0);
	call_prctl(__SYS_SET_FS, ptr, 0, 0, 0);
	ptr->Self = ptr;
	ptr->CurrentError = 0;
	return 0;
}
