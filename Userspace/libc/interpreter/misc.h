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

#ifndef __FENNIX_DL_HELPER_H__
#define __FENNIX_DL_HELPER_H__

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>

#define ALIGN_UP(x, align) ((__typeof__(x))(((uintptr_t)(x) + ((align) - 1)) & (~((align) - 1))))
#define ALIGN_DOWN(x, align) ((__typeof__(x))((x) & (~((align) - 1))))

#ifndef __FENNIX_DL_ELF_H__
#error "Please include elf.h before misc.h"
#endif

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *l, const char *r);
char *strcat(char *dest, const char *src);

unsigned long elf_hash(const unsigned char *name);
uint32_t gnu_hash(const char *name);
Elf64_Sym *find_symbol(const char *name, uint32_t *hash_table, Elf64_Sym *symtab, const char *strtab);

void __init_print_buffer();
void __fini_print_buffer();
int printf(const char *format, ...);

void *mini_malloc(size_t size);
void mini_free(void *ptr);

#endif // !__FENNIX_DL_HELPER_H__
