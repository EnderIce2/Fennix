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

#include <stddef.h>
#include <stdarg.h>
#include <bits/libc.h>
#include <sys/mman.h>

#include "elf.h"
#include "misc.h"
#define NANOPRINTF_IMPLEMENTATION 1
#include "nanoprintf.h"

char *print_buffer;
size_t print_buffer_size;
size_t print_buffer_offset;
void flush_buffer()
{
	if (print_buffer_offset > 0)
	{
		sysdep(Write)(1, print_buffer, print_buffer_offset);
		print_buffer_offset = 0;
	}
}

void print_wrapper(int c, void *)
{
	if (print_buffer_offset >= print_buffer_size - 1)
		flush_buffer();
	print_buffer[print_buffer_offset++] = (char)c;
}

void __init_print_buffer()
{
	print_buffer = (char *)sysdep(MemoryMap)(0,
											 0x1000,
											 PROT_READ | PROT_WRITE,
											 MAP_PRIVATE | MAP_ANONYMOUS,
											 -1, 0);
	print_buffer_size = 0x1000;
	print_buffer_offset = 0;
}

void __fini_print_buffer()
{
	flush_buffer();
	if (print_buffer != NULL)
		sysdep(MemoryUnmap)(print_buffer, 0x1000);
	print_buffer = NULL;
}

int printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = npf_vpprintf(print_wrapper, NULL, format, args);
	va_end(args);
	return ret;
}

int puts(const char *s)
{
	int len = strlen(s);
	memcpy(print_buffer + print_buffer_offset, s, len);
	print_buffer_offset += len;
	print_buffer[print_buffer_offset++] = '\n';
	flush_buffer();
	return len + 1;
}
