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

#include <bits/libc.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "elf.h"
#include "misc.h"

#undef PAGE_SIZE
#define PAGE_SIZE 0x1000
#undef ALIGN_UP
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct MemoryBlock
{
	struct MemoryBlock *next;
	void *slots;
	size_t slot_size;
	size_t slots_per_block;
	uint8_t *bitmap;
	size_t bitmap_size;
	size_t total_size;
} MemoryBlock;

static MemoryBlock *memory_pool = NULL;

void *request_page(size_t size)
{
	size_t aligned_size = ALIGN_UP(size, PAGE_SIZE);
	void *addr = (void *)sysdep(MemoryMap)(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if ((intptr_t)addr < 0)
		return NULL;
	return addr;
}

void free_page(void *addr, size_t size)
{
	size_t aligned_size = ALIGN_UP(size, PAGE_SIZE);
	sysdep(MemoryUnmap)(addr, aligned_size);
}

MemoryBlock *allocate_block(size_t slot_size)
{
	if (slot_size == 0 || slot_size > PAGE_SIZE / 2)
		return NULL;

	size_t estimated_slots = (PAGE_SIZE - sizeof(MemoryBlock)) / (slot_size + 1.0 / 8.0);
	if (estimated_slots == 0)
		estimated_slots = 1;

	size_t bitmap_size = ALIGN_UP((estimated_slots + 7) / 8, 8);
	size_t total_slots_size = estimated_slots * slot_size;
	size_t total_size = sizeof(MemoryBlock) + bitmap_size + total_slots_size;
	total_size = ALIGN_UP(total_size, PAGE_SIZE);

	MemoryBlock *block = (MemoryBlock *)request_page(total_size);
	if (!block)
		return NULL;

	uintptr_t base = (uintptr_t)block + sizeof(MemoryBlock);
	block->bitmap = (uint8_t *)base;
	block->bitmap_size = bitmap_size;

	block->slots = (void *)ALIGN_UP(base + bitmap_size, 16);
	block->slot_size = slot_size;
	block->slots_per_block = (total_size - ((uintptr_t)block->slots - (uintptr_t)block)) / slot_size;
	block->total_size = total_size;
	block->next = NULL;

	memset(block->bitmap, 0, bitmap_size);
	return block;
}

void *mini_malloc(size_t size)
{
	if (size == 0)
		return NULL;

	MemoryBlock *block = memory_pool;
	while (block)
	{
		if (block->slot_size == size)
		{
			for (size_t i = 0; i < block->slots_per_block; i++)
			{
				size_t byte = i / 8, bit = i % 8;
				if (!(block->bitmap[byte] & (1 << bit)))
				{
					block->bitmap[byte] |= (1 << bit);
					return (void *)((uintptr_t)block->slots + i * block->slot_size);
				}
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
		uintptr_t start = (uintptr_t)block->slots;
		uintptr_t end = start + block->slots_per_block * block->slot_size;

		if ((uintptr_t)ptr >= start && (uintptr_t)ptr < end)
		{
			size_t index = ((uintptr_t)ptr - start) / block->slot_size;
			size_t byte = index / 8, bit = index % 8;
			block->bitmap[byte] &= ~(1 << bit);
			return;
		}
		block = block->next;
	}
}
