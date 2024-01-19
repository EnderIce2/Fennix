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

#include <memory/macro.hpp>
#include <sys/mman.h>
#include <memory.hpp>
#include <assert.h>
#include <unistd.h>

// #include "rpmalloc.c"
#include "../../../../kernel.h"

struct heap_t
{
	char pad[56408];
};

static heap_t *__rpmalloc_tls_heap = nullptr;
EXTERNC heap_t **__memory_thread_heap(void)
{
	if (unlikely(!TaskManager || !thisThread))
	{
		if (unlikely(!__rpmalloc_tls_heap))
		{
			__rpmalloc_tls_heap = (heap_t *)KernelAllocator.RequestPages(TO_PAGES(sizeof(heap_t)));
			debug("rpmalloc TLS heap: %#lx", __rpmalloc_tls_heap);
			memset(__rpmalloc_tls_heap, 0, sizeof(heap_t));
			assert(__rpmalloc_tls_heap);
		}
		return &__rpmalloc_tls_heap;
	}
	return &__rpmalloc_tls_heap;
	heap_t *heap = (heap_t *)thisThread->TLS.pBase;
	return (heap_t **)heap;
}

EXTERNC uintptr_t __get_tid(void)
{
	if (unlikely(!TaskManager || !thisThread))
		return (uintptr_t)-1;
	return thisThread->ID;
}

EXTERNC long __rpmalloc_sysconf(int name)
{
	switch (name)
	{
	case _SC_PAGESIZE:
		return PAGE_SIZE;
	default:
		return -1;
	}
}

EXTERNC void *__rpmalloc_mmap(void *addr, size_t length, int, int, int fd, off_t offset)
{
	assert(addr == 0 && fd == -1 && offset == 0);

	void *ptr = KernelAllocator.RequestPages(TO_PAGES(length));
	debug("Requested %d pages, got %p", TO_PAGES(length), ptr);
	if (ptr == nullptr)
		return MAP_FAILED;
	return ptr;
}

EXTERNC int __rpmalloc_munmap(void *addr, size_t length)
{
	KernelAllocator.FreePages(addr, TO_PAGES(length));
	debug("Freed %d pages at %p", TO_PAGES(length), addr);
	return 0;
}

EXTERNC int __rpmalloc_posix_madvise(void *addr, size_t length, int advice)
{
	function("%#lx %d %d", addr, length, advice);
	return 0;
}
