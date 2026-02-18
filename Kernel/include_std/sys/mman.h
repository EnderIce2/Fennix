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

#pragma once
#include <types.h>

/* stubs for rpmalloc.c */

#ifndef MAP_PRIVATE
#define MAP_PRIVATE 0x0
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x0
#endif

#ifndef PROT_READ
#define PROT_READ 0x0
#endif

#ifndef PROT_WRITE
#define PROT_WRITE 0x0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED 0x0
#endif

#ifndef POSIX_MADV_DONTNEED
#define POSIX_MADV_DONTNEED 0x0
#endif

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int posix_madvise(void *addr, size_t size, int advice);
int munmap(void *addr, size_t length);
