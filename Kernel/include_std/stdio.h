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

#ifndef __FENNIX_KERNEL_STDIO_H__
#define __FENNIX_KERNEL_STDIO_H__

#include <types.h>

START_EXTERNC

#define FILENAME_MAX 4096

typedef struct
{
	int st;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

#include <printf.h>
// int printf(const char *format, ...) __attribute__((format(__printf__, (1), (2))));
// int vprintf(const char *format, va_list arg) __attribute__((format(__printf__, ((1)), (0))));
// int sprintf(char *s, const char *format, ...) __attribute__((format(__printf__, (2), (3))));
// int vsprintf(char *s, const char *format, va_list arg) __attribute__((format(__printf__, ((2)), (0))));
// int snprintf(char *s, size_t count, const char *format, ...) __attribute__((format(__printf__, (3), (4))));
// int vsnprintf(char *s, size_t count, const char *format, va_list arg) __attribute__((format(__printf__, ((3)), (0))));

int asprintf(char **strp, const char *fmt, ...) __attribute__((format(__printf__, (2), (3))));
int vasprintf(char **strp, const char *fmt, va_list ap) __attribute__((format(__printf__, ((2)), (0))));

int fprintf(FILE *stream, const char *format, ...) __attribute__((format(__printf__, (2), (3))));

int fputs(const char *s, FILE *stream);

END_EXTERNC

#endif // !__FENNIX_KERNEL_STDIO_H__
