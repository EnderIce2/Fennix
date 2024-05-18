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

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>

FILE __local_stdin = {.st = 0};
FILE __local_stdout = {.st = 1};
FILE __local_stderr = {.st = 2};

FILE *stdin = &__local_stdin;
FILE *stdout = &__local_stdout;
FILE *stderr = &__local_stderr;

EXTERNC int asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vasprintf(strp, fmt, ap);
	va_end(ap);
	return ret;
}

#define va_copy(dest, src) __builtin_va_copy(dest, src)

EXTERNC int vasprintf(char **strp, const char *fmt, va_list ap)
{
	va_list ap2;
	va_copy(ap2, ap);
	int len = vsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);
	if (len < 0)
	{
		return -1;
	}
	*strp = (char *)malloc(len + 1);
	if (!*strp)
	{
		return -1;
	}
	int ret = vsnprintf(*strp, len + 1, fmt, ap);
	if (ret < 0)
	{
		free(*strp);
		*strp = NULL;
	}
	return ret;
}

int fprintf(FILE *stream, const char *format, ...)
{
	switch (stream->st)
	{
	case 0:
	{
		error("fprintf() called with stdin");
		return -1;
	}
	case 1:
	{
		va_list ap;
		va_start(ap, format);
		int ret = vprintf(format, ap);
		va_end(ap);
		return ret;
	}
	case 2:
	{
		va_list ap;
		va_start(ap, format);
		int ret = vprintf(format, ap);
		va_end(ap);
		return ret;
	}
	default:
		return -1;
	}
}

int fputs(const char *s, FILE *stream)
{
	for (const char *c = s; *c; c++)
		uart_wrapper(*c, NULL);
	return 0;
}
