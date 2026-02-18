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

#define KERNEL_STACK_SIZE (128 * 1024 * 1024) /* 128 MiB */
#define USER_STACK_SIZE (8 * 1024 * 1024)	  /* 8 MiB */

/* To pages */
#define TO_PAGES(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
/* From pages */
#define FROM_PAGES(d) ((d) * PAGE_SIZE)

#if defined(__amd64__) || defined(__aarch64__)
#define USER_ALLOC_BASE 0x0000000000100000
#define USER_ALLOC_END 0x00007FFFFFFFFFFF

#define USER_STACK_TOP 0x00007FFFFFFF1000
#define USER_STACK_BASE (USER_STACK_TOP - USER_STACK_SIZE + 1)

#define KERNEL_STACK_TOP 0xFFFFCFFFFFFF1000
#define KERNEL_STACK_BASE (KERNEL_STACK_TOP - KERNEL_STACK_SIZE + 1)

#define KERNEL_HHDM_OFFSET 0xFFFF800000000000
#define KERNEL_VMA_OFFSET 0xFFFFFFFF80000000
#elif defined(__i386__) || defined(__arm__)
#define KERNEL_VMA_OFFSET 0xC0000000
#define KERNEL_HHDM_OFFSET 0xD0000000

#define USER_ALLOC_BASE 0x80000000
#define USER_ALLOC_END 0xA0000000

#define KERNEL_STACK_BASE 0xA0000000
#define KERNEL_STACK_TOP 0xB0000000

#define USER_STACK_BASE 0xEFFFFFFF
#define USER_STACK_TOP 0xE0000000
#endif
