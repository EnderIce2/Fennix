#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscalls.h>
#include <errno.h>

#include <sys/types.h> // For PUBLIC

PUBLIC FILE *stdin = NULL;
PUBLIC FILE *stdout = NULL;
PUBLIC FILE *stderr = NULL;

PUBLIC FILE *fopen(const char *filename, const char *mode)
{
	void *KPrivate = (void *)syscall2(_FileOpen, (uint64_t)filename, (uint64_t)mode);
	if (IsSyscallError(KPrivate))
		return NULL;

	FILE *FilePtr = malloc(sizeof(FILE));
	FilePtr->KernelPrivate = KPrivate;
	return FilePtr;
}

PUBLIC size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (ptr == NULL || stream == NULL || size == 0 || nmemb == 0)
	{
		errno = EINVAL;
		return 0;
	}

	syscall3(_FileSeek, stream->KernelPrivate, stream->_offset, SEEK_SET);
	return syscall3(_FileRead, (uint64_t)stream->KernelPrivate, (uint64_t)ptr, size * nmemb);
}

PUBLIC size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (ptr == NULL || stream == NULL || size == 0 || nmemb == 0)
	{
		errno = EINVAL;
		return 0;
	}

	syscall3(_FileSeek, stream->KernelPrivate, stream->_offset, SEEK_SET);
	return syscall3(_FileWrite, (uint64_t)stream->KernelPrivate, (uint64_t)ptr, size * nmemb);
}

PUBLIC int fclose(FILE *fp)
{
	if (fp == NULL)
	{
		errno = EINVAL;
		return EOF;
	}

	void *KP = fp->KernelPrivate;
	free(fp);
	return syscall1(_FileClose, (uint64_t)KP);
}

PUBLIC off_t fseek(FILE *stream, off_t offset, int whence)
{
	if (stream == NULL || whence < 0 || whence > 2)
	{
		errno = EINVAL;
		return -1;
	}

	off_t new_offset = syscall3(_FileSeek, stream->KernelPrivate, offset, whence);
	if (IsSyscallError(new_offset))
		return -1;
	stream->_offset = new_offset;
	return new_offset;
}

PUBLIC long ftell(FILE *stream)
{
	return stream->_offset;
}

PUBLIC int fflush(FILE *stream)
{
	if (stream == NULL)
	{
		errno = EINVAL;
		return EOF;
	}

	errno = ENOSYS;
	return EOF;
}

PUBLIC int fprintf(FILE *stream, const char *format, ...)
{
	if (stream == NULL || format == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	va_list args;
	va_start(args, format);
	const int ret = vfprintf(stream, format, args);
	va_end(args);
	return ret;
}

PUBLIC void setbuf(FILE *stream, char *buf)
{
}

PUBLIC int vfprintf(FILE *stream, const char *format, va_list arg)
{
	return 0;
}

PUBLIC int vsscanf(const char *s, const char *format, va_list arg)
{
}

PUBLIC int sscanf(const char *s, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	const int ret = vsscanf(s, format, args);
	va_end(args);
	return ret;
}
