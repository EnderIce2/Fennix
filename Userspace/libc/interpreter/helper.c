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

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;
	while (n--)
		*p++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;
	while (n--)
		*d++ = *s++;
	return dest;
}

size_t strlen(const char *s)
{
	const char *p = s;
	while (*p)
		p++;
	return p - s;
}

char *strcpy(char *dest, const char *src)
{
	char *d = dest;
	while ((*d++ = *src++))
		;
	return dest;
}

int strcmp(const char *l, const char *r)
{
	while (*l && *l == *r)
	{
		l++;
		r++;
	}
	return *l - *r;
}

char *strcat(char *dest, const char *src)
{
	char *d = dest;
	while (*d)
		d++;
	while ((*d++ = *src++))
		;
	return dest;
}

unsigned long elf_hash(const unsigned char *name)
{
	unsigned long hash = 0, high;
	while (*name)
	{
		hash = (hash << 4) + *name++;
		if ((high = hash & 0xF0000000))
			hash ^= high >> 24;
		hash &= ~high;
	}
	return hash;
}

uint32_t gnu_hash(const char *name)
{
	uint32_t hash = 5381;
	for (; *name; name++)
	{
		hash = (hash << 5) + hash + (unsigned char)(*name); // hash * 33 + c
	}
	return hash;
}

Elf64_Sym *find_symbol(const char *name, uint32_t *hash_table, Elf64_Sym *symtab, const char *strtab)
{
	/* Symbol Hash Table
	|-------------------|
	|      nbucket      |
	|-------------------|
	|      nchain       |
	|-------------------|
	|     bucket[0]     |
	|      . . .        |
	|bucket[nbucket - 1]|
	|-------------------|
	|     chain[0]      |
	|      . . .        |
	| chain[nchain - 1] |
	|-------------------|
	*/
	unsigned long h = elf_hash(name);		  // or gnu_hash(name)
	unsigned long bucket = h % hash_table[0]; // hash_table[0] = nbucket

	for (unsigned long i = hash_table[2 + bucket];
		 i != STN_UNDEF;
		 i = hash_table[2 + hash_table[0] + i])
	{
		if (!strcmp(&strtab[symtab[i].st_name], name))
			return &symtab[i];
	}
	return NULL;
}
