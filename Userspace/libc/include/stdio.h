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

#ifndef _STDIO_H
#define _STDIO_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>

#define BUFSIZ 512
#define FILENAME_MAX 255
#define FOPEN_MAX 8
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#define L_ctermid 20
#define L_cuserid 20
#define L_tmpnam 255
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0
#define TMP_MAX 10000

#define EOF (-1)

#ifndef NULL
#define NULL ((void *)0)
#endif

#define P_tmpdir "/tmp/"

	typedef long fpos_t;
	typedef __SIZE_TYPE__ size_t;

	struct _IO_FILE
	{
		int fd;
		char *buffer;
		size_t buffer_size;
		size_t buffer_pos;
		int flags;
		int error;
		int eof;
	};

#define _i_READ 1
#define _i_WRITE 2
	extern struct _IO_FILE *_i_open_files[256];

	typedef struct _IO_FILE FILE;

	extern char *optarg;
	extern int opterr;
	extern int optind;
	extern int optopt;

	extern FILE *stdin;
	extern FILE *stdout;
	extern FILE *stderr;

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define stdin stdin
#define stdout stdout
#define stderr stderr

	void clearerr(FILE *);
	char *ctermid(char *);
	int dprintf(int, const char *restrict, ...);
	int fclose(FILE *stream);
	FILE *fdopen(int, const char *);
	int feof(FILE *stream);
	int ferror(FILE *stream);
	int fflush(FILE *stream);
	int fgetc(FILE *stream);
	int fgetpos(FILE *restrict, fpos_t *restrict);
	char *fgets(char *restrict s, int n, FILE *restrict stream);
	int fileno(FILE *);
	void flockfile(FILE *);
	FILE *fmemopen(void *restrict, size_t, const char *restrict);
	FILE *fopen(const char *restrict pathname, const char *restrict mode);
	int fprintf(FILE *restrict stream, const char *restrict format, ...);
	int fputc(int c, FILE *stream);
	int fputs(const char *restrict s, FILE *restrict stream);
	size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
	FILE *freopen(const char *restrict, const char *restrict, FILE *restrict);
	int fscanf(FILE *restrict, const char *restrict, ...);
	int fseek(FILE *stream, long offset, int whence);
	int fseeko(FILE *, off_t, int);
	int fsetpos(FILE *, const fpos_t *);
	long ftell(FILE *);
	off_t ftello(FILE *);
	int ftrylockfile(FILE *);
	void funlockfile(FILE *);
	size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
	int getc(FILE *stream);
	int getchar(void);
	int getc_unlocked(FILE *);
	int getchar_unlocked(void);
	ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delimiter, FILE *restrict stream);
	ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream);
	FILE *open_memstream(char **, size_t *);
	int pclose(FILE *);
	void perror(const char *s);
	FILE *popen(const char *, const char *);
	int printf(const char *restrict format, ...);
	int putc(int c, FILE *stream);
	int putchar(int c);
	int putc_unlocked(int c, FILE *stream);
	int putchar_unlocked(int c);
	int puts(const char *s);
	int remove(const char *);
	int rename(const char *, const char *);
	int renameat(int, const char *, int, const char *);
	void rewind(FILE *);
	int scanf(const char *restrict, ...);
	void setbuf(FILE *restrict, char *restrict);
	int setvbuf(FILE *restrict, char *restrict, int, size_t);
	int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
	int sprintf(char *restrict s, const char *restrict format, ...);
	int sscanf(const char *restrict, const char *restrict, ...);
	FILE *tmpfile(void);
	char *tmpnam(char *);
	int ungetc(int, FILE *);
	int vdprintf(int, const char *restrict, va_list);
	int vfprintf(FILE *restrict stream, const char *restrict format, va_list ap);
	int vfscanf(FILE *restrict, const char *restrict, va_list);
	int vprintf(const char *restrict, va_list);
	int vscanf(const char *restrict, va_list);
	int vsnprintf(char *restrict, size_t, const char *restrict, va_list);
	int vsprintf(char *restrict, const char *restrict, va_list);
	int vsscanf(const char *restrict, const char *restrict, va_list);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STDIO_H
