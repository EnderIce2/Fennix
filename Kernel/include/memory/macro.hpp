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

#ifndef __FENNIX_KERNEL_MEMORY_MACROS_H__
#define __FENNIX_KERNEL_MEMORY_MACROS_H__

#include <types.h>

// kilobyte
#define TO_KiB(d) ((d) / 1024)
// megabyte
#define TO_MiB(d) ((d) / 1024 / 1024)
// gigabyte
#define TO_GiB(d) ((d) / 1024 / 1024 / 1024)
// terabyte
#define TO_TiB(d) ((d) / 1024 / 1024 / 1024 / 1024)
// petabyte
#define TO_PiB(d) ((d) / 1024 / 1024 / 1024 / 1024 / 1024)
// exobyte
#define TO_EiB(d) ((d) / 1024 / 1024 / 1024 / 1024 / 1024 / 1024)

#define PAGE_SIZE 0x1000		/* 4 KiB */
#define PAGE_SIZE_4K PAGE_SIZE	/* 4 KiB */
#define PAGE_SIZE_2M 0x200000	/* 2 MiB */
#define PAGE_SIZE_4M 0x400000	/* 4 MiB */
#define PAGE_SIZE_1G 0x40000000 /* 1 GiB */

#define STACK_SIZE 0x4000		 /* 16 KiB */
#define LARGE_STACK_SIZE 0x20000 /* 128 KiB */
#define USER_STACK_SIZE 0x2000	 /* 8 KiB */

/* To pages */
#define TO_PAGES(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
/* From pages */
#define FROM_PAGES(d) ((d) * PAGE_SIZE)

#if defined(__amd64__) || defined(__aarch64__)
#define KERNEL_VMA_OFFSET 0xFFFFFFFF80000000

#define USER_ALLOC_BASE 0xFFFFA00000000000 /* 256 GiB */
#define USER_ALLOC_END 0xFFFFB00000000000

#define KERNEL_STACK_BASE 0xFFFFB00000000000 /* 256 GiB */
#define KERNEL_STACK_END 0xFFFFC00000000000

#define USER_STACK_END 0xFFFFEFFF00000000 /* 256 MiB */
#define USER_STACK_BASE 0xFFFFEFFFFFFF0000
#elif defined(__i386__)
#define KERNEL_VMA_OFFSET 0xC0000000

#define USER_ALLOC_BASE 0x80000000
#define USER_ALLOC_END 0xA0000000

#define KERNEL_STACK_BASE 0xA0000000
#define KERNEL_STACK_END 0xB0000000

#define USER_STACK_BASE 0xEFFFFFFF
#define USER_STACK_END 0xE0000000
#endif

#endif // !__FENNIX_KERNEL_MEMORY_MACROS_H__
