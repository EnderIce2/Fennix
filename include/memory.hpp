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

#ifndef __FENNIX_KERNEL_INTERNAL_MEMORY_H__
#define __FENNIX_KERNEL_INTERNAL_MEMORY_H__

#ifdef __cplusplus
#include <filesystem.hpp>
#include <boot/binfo.h>
#include <bitmap.hpp>
#include <lock.hpp>
#include <cstddef>
#include <atomic>
#include <new>
#endif // __cplusplus
#include <types.h>

#ifdef __cplusplus

extern uintptr_t _bootstrap_start, _bootstrap_end;
extern uintptr_t _kernel_start, _kernel_end;
extern uintptr_t _kernel_text_start, _kernel_text_end;
extern uintptr_t _kernel_data_start, _kernel_data_end;
extern uintptr_t _kernel_rodata_start, _kernel_rodata_end;
extern uintptr_t _kernel_bss_start, _kernel_bss_end;

namespace Memory
{
	enum MemoryAllocatorType
	{
		None,

		/** Allocate memory by pages. */
		Pages,

		XallocV1,
		XallocV2,
		liballoc11,

		/**
		 * @warning Not working as expected.
		 *
		 * FIXME: This allocator is not working as expected.
		 */
		rpmalloc_,
	};
}

#include <memory/physical.hpp>
#include <memory/virtual.hpp>
#include <memory/swap_pt.hpp>
#include <memory/table.hpp>
#include <memory/macro.hpp>
#include <memory/stack.hpp>
#include <memory/vma.hpp>
#include <memory/brk.hpp>

void InitializeMemoryManagement();
void CreatePageTable(Memory::PageTable *pt);

extern Memory::Physical KernelAllocator;
extern Memory::PageTable *KernelPageTable;

#endif // __cplusplus

#ifndef __FENNIX_KERNEL_STDLIB_H__

EXTERNC void *malloc(size_t Size);
EXTERNC void *calloc(size_t n, size_t Size);
EXTERNC void *realloc(void *Address, size_t Size);
EXTERNC void free(void *Address);

#endif // !__FENNIX_KERNEL_STDLIB_H__

#define kmalloc(Size) malloc(Size)
#define kcalloc(n, Size) calloc(n, Size)
#define krealloc(Address, Size) realloc(Address, Size)
#define kfree(Address) free(Address)

#endif // !__FENNIX_KERNEL_INTERNAL_MEMORY_H__
