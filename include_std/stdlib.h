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

#ifndef __FENNIX_KERNEL_STDLIB_H__
#define __FENNIX_KERNEL_STDLIB_H__

#include <types.h>

START_EXTERNC

#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__

void *malloc(size_t Size);
void *calloc(size_t n, size_t Size);
void *realloc(void *Address, size_t Size);
void free(void *Address);

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__

void abort();

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

END_EXTERNC

#endif // !__FENNIX_KERNEL_STDLIB_H__
