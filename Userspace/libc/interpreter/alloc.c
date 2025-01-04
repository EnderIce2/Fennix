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
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "elf.h"
#include "misc.h"

typedef struct MemoryBlock
{
	struct MemoryBlock *next;
	void *slots;
	size_t slot_size;
	size_t slots_per_block;
	uint8_t *bitmap;
} MemoryBlock;

MemoryBlock *memory_pool = NULL;
#define PAGE_SIZE 0x1000

void *request_page(size_t size)
{
	size_t aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	void *addr = (void *)call_mmap(NULL, aligned_size, __SYS_PROT_READ | __SYS_PROT_WRITE, __SYS_MAP_ANONYMOUS | __SYS_MAP_PRIVATE, -1, 0);
	if ((intptr_t)addr < 0)
		return NULL;
	return addr;
}

void free_page(void *addr, size_t size)
{
	size_t aligned_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	call_munmap(addr, aligned_size);
}

MemoryBlock *allocate_block(size_t slot_size)
{
	size_t block_size = PAGE_SIZE;
	size_t slots_per_block = block_size / slot_size;
	size_t bitmap_size = (slots_per_block + 7) / 8;

	MemoryBlock *block = request_page(block_size);
	if (!block)
		return NULL;

	block->slots = (void *)((uintptr_t)block + sizeof(MemoryBlock) + bitmap_size);
	block->slot_size = slot_size;
	block->slots_per_block = slots_per_block;
	block->bitmap = (uint8_t *)((uintptr_t)block + sizeof(MemoryBlock));
	memset(block->bitmap, 0, bitmap_size);
	block->next = NULL;

	return block;
}

void *mini_malloc(size_t size)
{
	MemoryBlock *block = memory_pool;
	while (block)
	{
		for (size_t i = 0; i < block->slots_per_block; i++)
		{
			size_t byte = i / 8, bit = i % 8;
			if (!(block->bitmap[byte] & (1 << bit)))
			{
				block->bitmap[byte] |= (1 << bit);
				return (void *)((uintptr_t)block->slots + i * size);
			}
		}
		block = block->next;
	}

	block = allocate_block(size);
	if (!block)
		return NULL;

	block->next = memory_pool;
	memory_pool = block;

	block->bitmap[0] |= 1;
	return block->slots;
}

void mini_free(void *ptr)
{
	MemoryBlock *block = memory_pool;
	while (block)
	{
		if ((uintptr_t)ptr >= (uintptr_t)block->slots &&
			(uintptr_t)ptr < (uintptr_t)block->slots + block->slots_per_block * block->slot_size)
		{
			size_t index = ((uintptr_t)ptr - (uintptr_t)block->slots) / block->slot_size;
			size_t byte = index / 8, bit = index % 8;
			block->bitmap[byte] &= ~(1 << bit);
			return;
		}
		block = block->next;
	}
}
