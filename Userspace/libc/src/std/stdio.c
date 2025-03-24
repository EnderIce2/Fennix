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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <bits/libc.h>
#include <fcntl.h>
#include "../print/printf.h"

struct _IO_FILE *_i_open_files[256];

export FILE *stdin;
export FILE *stdout;
export FILE *stderr;

void __init_stdio(void)
{
	stdin = malloc(sizeof(FILE));
	stdin->fd = 0;
	stdin->buffer = malloc(4096);
	stdin->buffer_size = 4096;
	stdin->buffer_pos = 0;
	stdin->flags = _i_READ;
	stdin->error = 0;
	stdin->eof = 0;

	stdout = malloc(sizeof(FILE));
	stdout->fd = 1;
	stdout->buffer = malloc(4096);
	stdout->buffer_size = 4096;
	stdout->buffer_pos = 0;
	stdout->flags = _i_WRITE;
	stdout->error = 0;
	stdout->eof = 0;

	stderr = malloc(sizeof(FILE));
	stderr->fd = 2;
	stderr->buffer = malloc(4096);
	stderr->buffer_size = 4096;
	stderr->buffer_pos = 0;
	stderr->flags = _i_WRITE;
	stderr->error = 0;
	stderr->eof = 0;

	_i_open_files[0] = stdin;
	_i_open_files[1] = stdout;
	_i_open_files[2] = stderr;
}

export void clearerr(FILE *);
export char *ctermid(char *);
export int dprintf(int, const char *restrict, ...);

export int fclose(FILE *stream)
{
	if (!stream)
		return EOF;

	if (stream->buffer)
		free(stream->buffer);

	sysdep(Close)(stream->fd);
	_i_open_files[stream->fd] = NULL;
	free(stream);
	return 0;
}

export FILE *fdopen(int, const char *);

export int feof(FILE *stream)
{
	if (!stream)
		return 0;
	return stream->eof;
}

export int ferror(FILE *stream)
{
	if (!stream)
		return 0;
	return stream->error;
}

export int fflush(FILE *stream)
{
	if (!stream)
		return EOF;

	if (stream->flags & _i_WRITE)
	{
		if (stream->buffer_pos > 0)
		{
			ssize_t written = sysdep(Write)(stream->fd, stream->buffer, stream->buffer_pos);
			if (written < 0)
			{
				stream->error = 1;
				return EOF;
			}
			stream->buffer_pos = 0;
		}
	}
	else if (stream->flags & _i_READ)
	{
		stream->buffer_pos = 0;
		stream->buffer_size = 0;
	}
	return 0;
}

export int fgetc(FILE *stream)
{
	if (!stream || !(stream->flags & _i_READ))
		return EOF;

	if (stream->buffer_pos >= stream->buffer_size)
	{
		int res = sysdep(Read)(stream->fd, stream->buffer, 4096);
		if (res <= 0)
		{
			if (res == 0)
				stream->eof = 1;
			else
				stream->error = 1;
			return EOF;
		}
		stream->buffer_pos = 0;
		stream->buffer_size = res;
	}

	return (unsigned char)stream->buffer[stream->buffer_pos++];
}

export int fgetpos(FILE *restrict, fpos_t *restrict);

export char *fgets(char *restrict s, int n, FILE *restrict stream)
{
	if (!s || n <= 0 || !stream)
		return NULL;

	int i = 0;
	while (i < n - 1)
	{
		int c = fgetc(stream);
		if (c == EOF)
		{
			if (i == 0)
				return NULL;
			break;
		}
		s[i++] = (char)c;
		if (c == '\n')
			break;
	}
	s[i] = '\0';
	return s;
}

export int fileno(FILE *);
export void flockfile(FILE *);
export FILE *fmemopen(void *restrict, size_t, const char *restrict);

export FILE *fopen(const char *restrict pathname, const char *restrict mode)
{
	int flags = 0;
	mode_t perm = 0;

	if (strcmp(mode, "r") == 0)
		flags = O_RDONLY;
	else if (strcmp(mode, "r+") == 0)
		flags = O_RDWR;
	else if (strcmp(mode, "w") == 0)
	{
		flags = O_WRONLY | O_CREAT | O_TRUNC;
		perm = 0644;
	}
	else if (strcmp(mode, "w+") == 0)
	{
		flags = O_RDWR | O_CREAT | O_TRUNC;
		perm = 0644;
	}
	else if (strcmp(mode, "a") == 0)
	{
		flags = O_WRONLY | O_CREAT | O_APPEND;
		perm = 0644;
	}
	else if (strcmp(mode, "a+") == 0)
	{
		flags = O_RDWR | O_CREAT | O_APPEND;
		perm = 0644;
	}
	else
		return NULL;

	int fd = sysdep(Open)(pathname, flags, perm);
	if (fd < 0)
		return NULL;

	FILE *file = malloc(sizeof(FILE));
	if (!file)
		return NULL;

	file->fd = fd;
	file->buffer = malloc(4096);
	file->buffer_size = 4096;
	file->buffer_pos = -1;
	file->flags = flags;
	file->error = 0;
	file->eof = 0;

	_i_open_files[fd] = file;
	return file;
}

export int fprintf(FILE *restrict stream, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vfprintf(stream, format, args);
	va_end(args);
	return ret;
}

export int fputc(int c, FILE *stream)
{
	if (!stream || !(stream->flags & _i_WRITE))
		return EOF;

	stream->buffer[stream->buffer_pos++] = (char)c;

	if (stream->buffer_pos >= stream->buffer_size)
	{
		if (fflush(stream) == EOF)
			return EOF;
	}

	return (unsigned char)c;
}

export int fputs(const char *restrict s, FILE *restrict stream)
{
	if (!stream || !(stream->flags & _i_WRITE))
		return EOF;

	while (*s)
	{
		if (fputc(*s++, stream) == EOF)
			return EOF;
	}

	return 0;
}

export size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
{
	size_t total_bytes = size * nitems;
	size_t bytes_read = 0;

	while (bytes_read < total_bytes)
	{
		if (stream->buffer_pos >= stream->buffer_size)
		{
			int res = sysdep(Read)(stream->fd, stream->buffer, stream->buffer_size);
			if (res <= 0)
			{
				if (res == 0)
					stream->eof = 1;
				else
					stream->error = 1;
				break;
			}
			stream->buffer_pos = 0;
			stream->buffer_size = res;
		}

		size_t bytes_to_copy = total_bytes - bytes_read;
		size_t available = stream->buffer_size - stream->buffer_pos;
		if (bytes_to_copy > available)
			bytes_to_copy = available;

		memcpy((char *)ptr + bytes_read, stream->buffer + stream->buffer_pos, bytes_to_copy);
		stream->buffer_pos += bytes_to_copy;
		bytes_read += bytes_to_copy;
	}

	return bytes_read / size;
}

export FILE *freopen(const char *restrict, const char *restrict, FILE *restrict);
export int fscanf(FILE *restrict, const char *restrict, ...);

export int fseek(FILE *stream, long offset, int whence)
{
	int res = sysdep(Seek)(stream->fd, offset, whence);
	if (res < 0)
	{
		stream->error = 1;
		return EOF;
	}
	stream->buffer_pos = 0;
	stream->buffer_size = 0;
	return 0;
}

export int fseeko(FILE *, off_t, int);
export int fsetpos(FILE *, const fpos_t *);

export long ftell(FILE *stream)
{
	return sysdep(Tell)(stream->fd);
}

export off_t ftello(FILE *);
export int ftrylockfile(FILE *);
export void funlockfile(FILE *);

export size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
{
	size_t total_bytes = size * nitems;
	size_t bytes_written = 0;

	while (bytes_written < total_bytes)
	{
		size_t bytes_to_copy = total_bytes - bytes_written;
		size_t space_available = stream->buffer_size - stream->buffer_pos;

		if (bytes_to_copy > space_available)
			bytes_to_copy = space_available;

		memcpy(stream->buffer + stream->buffer_pos, (const char *)ptr + bytes_written, bytes_to_copy);
		stream->buffer_pos += bytes_to_copy;
		bytes_written += bytes_to_copy;

		if (stream->buffer_pos == stream->buffer_size)
		{
			if (sysdep(Write)(stream->fd, stream->buffer, stream->buffer_size) != stream->buffer_size)
			{
				stream->error = 1;
				break;
			}
			stream->buffer_pos = 0;
		}
	}

	return bytes_written / size;
}

export int getc(FILE *stream)
{
	return fgetc(stream);
}

export int getchar(void)
{
	return getc(stdin);
}

export int getc_unlocked(FILE *);
export int getchar_unlocked(void);

export ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delimiter, FILE *restrict stream)
{
	if (!lineptr || !n || !stream)
	{
		errno = EINVAL;
		return -1;
	}

	if (!*lineptr)
	{
		*n = 128;
		*lineptr = malloc(*n);
		if (!*lineptr)
		{
			errno = ENOMEM;
			return -1;
		}
	}

	size_t pos = 0;
	int c;

	while ((c = fgetc(stream)) != EOF)
	{
		if (pos + 1 >= *n)
		{
			*n *= 2;
			char *new_lineptr = realloc(*lineptr, *n);
			if (!new_lineptr)
			{
				errno = ENOMEM;
				return -1;
			}
			*lineptr = new_lineptr;
		}

		(*lineptr)[pos++] = (char)c;

		if (c == delimiter)
			break;
	}

	if (ferror(stream))
		return -1;

	if (pos == 0 && feof(stream))
		return -1;

	(*lineptr)[pos] = '\0';
	return pos;
}

export ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
	return getdelim(lineptr, n, '\n', stream);
}

export FILE *open_memstream(char **, size_t *);
export int pclose(FILE *);

export void perror(const char *s)
{
	fputs(s, stderr);
	fputs(": ", stderr);
	fputs(strerror(errno), stderr);
	fputc('\n', stderr);
}

export FILE *popen(const char *, const char *);

export int printf(const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vprintf_(format, args);
	va_end(args);
	return ret;
}

export int putc(int c, FILE *stream) { return fputc(c, stream); }
export int putchar(int c) { return putc(c, stdout); }
export int putc_unlocked(int c, FILE *stream) { return fputc(c, stream); }
export int putchar_unlocked(int c) { return putc_unlocked(c, stdout); }

export int puts(const char *s)
{
	if (fputs(s, stdout) == EOF)
		return EOF;
	if (fputc('\n', stdout) == EOF)
		return EOF;
	return 0;
}

export int remove(const char *path)
{
	if (!path)
	{
		errno = EINVAL;
		return -1;
	}

	struct stat path_stat;
	if (sysdep(Stat)(path, &path_stat) < 0)
		return -1;

	if (S_ISDIR(path_stat.st_mode))
		return sysdep(RemoveDirectory)(path);
	else
		return sysdep(Unlink)(path);
}

export int rename(const char *, const char *);
export int renameat(int, const char *, int, const char *);
export void rewind(FILE *);
export int scanf(const char *restrict, ...);
export void setbuf(FILE *restrict, char *restrict);
export int setvbuf(FILE *restrict, char *restrict, int, size_t);

export int snprintf(char *restrict s, size_t n, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vsnprintf_(s, n, format, args);
	va_end(args);
	return ret;
}

export int sprintf(char *restrict s, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vsprintf_(s, format, args);
	va_end(args);
	return ret;
}

export int sscanf(const char *restrict s, const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);

	const char *p = format;
	int matchedItems = 0;

	while (*p)
	{
		if (isspace(*p))
		{
			while (isspace(*p))
				p++;
			while (isspace(*s))
				s++;
		}
		else if (*p == '%')
		{
			p++;
			if (*p == '\0')
				break;

			int suppress_assignment = 0;
			if (*p == '*')
			{
				suppress_assignment = 1;
				p++;
			}

			int width = 0;
			while (isdigit(*p))
			{
				width = width * 10 + (*p - '0');
				p++;
			}

			char specifier = *p++;
			if (specifier == '\0')
				break;

			if (specifier == 'd' || specifier == 'i')
			{
				int value = 0;
				while (isdigit(*s) || (*s == '-' && value == 0))
				{
					if (*s == '-')
					{
						s++;
						continue;
					}
					value = value * 10 + (*s - '0');
					s++;
				}
				if (!suppress_assignment)
				{
					int *arg = va_arg(args, int *);
					*arg = value;
					matchedItems++;
				}
			}
			else if (specifier == 's')
			{
				char *str = va_arg(args, char *);
				while (*s && !isspace(*s) && (width == 0 || width-- > 0))
				{
					if (!suppress_assignment)
						*str++ = *s;
					s++;
				}
				if (!suppress_assignment)
					*str = '\0';
				matchedItems++;
			}
			else if (specifier == 'c')
			{
				char *ch = va_arg(args, char *);
				if (*s && (width == 0 || width-- > 0))
				{
					if (!suppress_assignment)
						*ch = *s;
					s++;
					matchedItems++;
				}
			}
			else if (specifier == '%')
			{
				if (*s == '%')
					s++;
				else
					break;
			}
			else
				break;
		}
		else
		{
			if (*s == *p)
			{
				s++;
				p++;
			}
			else
				break;
		}
	}

	va_end(args);
	return matchedItems;
}

export FILE *tmpfile(void);
export char *tmpnam(char *);
export int ungetc(int, FILE *);
export int vdprintf(int, const char *restrict, va_list);

export int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap)
{
	if (!stream || !(stream->flags & _i_WRITE))
		return EOF;

	int ret = vprintf_(format, ap);
	if (ret < 0)
	{
		stream->error = 1;
		return EOF;
	}

	const char *p = format;
	while (*p)
	{
		if (fputc(*p++, stream) == EOF)
		{
			stream->error = 1;
			return EOF;
		}
	}

	return ret;
}

export int vfscanf(FILE *restrict, const char *restrict, va_list);
export int vprintf(const char *restrict, va_list);
export int vscanf(const char *restrict, va_list);

export int vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list ap)
{
	int ret = vsnprintf_(s, n, format, ap);
	if (ret < 0)
		return ret;
	return ret;
}

export int vsprintf(char *restrict, const char *restrict, va_list);
export int vsscanf(const char *restrict, const char *restrict, va_list);
