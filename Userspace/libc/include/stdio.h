#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/stat.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/**
 * @brief EOF - End of file
 *
 * The value returned by several functions to indicate the end of file.
 */
#define EOF (-1)

struct _IO_marker
{
	struct _IO_marker *_next;
	struct _IO_FILE *_sbuf;
	int _pos;
};

struct _IO_FILE
{
	int _flags;

	char *_IO_read_ptr;
	char *_IO_read_end;
	char *_IO_read_base;
	char *_IO_write_base;
	char *_IO_write_ptr;
	char *_IO_write_end;
	char *_IO_buf_base;
	char *_IO_buf_end;

	__off_t _offset;

	struct _IO_marker *_markers;
	struct _IO_FILE *_chain;
	int _fileno;
};

typedef struct _IO_FILE FILE;

#ifdef __cplusplus
extern "C"
{
#endif

	extern FILE *stdin;
	extern FILE *stdout;
	extern FILE *stderr;
#define stdin stdin
#define stdout stdout
#define stderr stderr

	FILE *freopen(const char *filename, const char *mode, FILE *stream);
	FILE *fopen(const char *filename, const char *mode);
	size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
	off_t fseek(FILE *stream, long offset, int whence);
	long ftell(FILE *stream);
	int fclose(FILE *fp);
	int fflush(FILE *stream);
	int fprintf(FILE *stream, const char *format, ...);
	int printf(const char *format, ...);
	void setbuf(FILE *stream, char *buf);
	int vfprintf(FILE *stream, const char *format, va_list arg);
	int vsscanf(const char *s, const char *format, va_list arg);
	int sscanf(const char *s, const char *format, ...);

	int fputc(int c, FILE *stream);
	int putc(int c, FILE *stream);
	int fputs(const char *s, FILE *stream);
	int puts(const char *s);
	int putchar(int c);

	int fgetc(FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
