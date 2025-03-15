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

#include <sys/types.h>
#include <string.h>
#include <bits/libc.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

export void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n)
{
	unsigned char *dest = (unsigned char *)s1;
	const unsigned char *src = (const unsigned char *)s2;
	unsigned char uc = (unsigned char)c;

	while (n--)
	{
		*dest = *src;
		if (*src == uc)
			return (void *)(dest + 1);
		dest++;
		src++;
	}
	return NULL;
}

export void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *src = (const unsigned char *)s;
	unsigned char uc = (unsigned char)c;

	while (n--)
	{
		if (*src == uc)
			return (void *)src;
		src++;
	}
	return NULL;
}

export int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (n--)
	{
		if (*p1 != *p2)
			return *p1 - *p2;
		p1++;
		p2++;
	}
	return 0;
}

export void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
	unsigned char *dest = (unsigned char *)s1;
	const unsigned char *src = (const unsigned char *)s2;

	while (n >= sizeof(unsigned long))
	{
		*(unsigned long *)dest = *(const unsigned long *)src;
		dest += sizeof(unsigned long);
		src += sizeof(unsigned long);
		n -= sizeof(unsigned long);
	}

	while (n--)
		*dest++ = *src++;
	return s1;
}

export void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen)
{
	const unsigned char *h = (const unsigned char *)haystack;
	const unsigned char *n = (const unsigned char *)needle;

	if (needlelen == 0)
		return (void *)haystack;

	if (haystacklen < needlelen)
		return NULL;

	for (size_t i = 0; i <= haystacklen - needlelen; i++)
	{
		if (memcmp(h + i, n, needlelen) == 0)
			return (void *)(h + i);
	}

	return NULL;
}

export void *memmove(void *s1, const void *s2, size_t n)
{
	unsigned char *dest = (unsigned char *)s1;
	const unsigned char *src = (const unsigned char *)s2;

	if (dest < src)
	{
		while (n--)
			*dest++ = *src++;
	}
	else
	{
		dest += n;
		src += n;
		while (n--)
			*--dest = *--src;
	}

	return s1;
}

export void *memset(void *s, int c, size_t n)
{
	unsigned char *p = (unsigned char *)s;
	while (n--)
		*p++ = (unsigned char)c;
	return s;
}

export char *stpcpy(char *restrict s1, const char *restrict s2)
{
	while ((*s1 = *s2) != '\0')
	{
		s1++;
		s2++;
	}
	return s1;
}

export char *stpncpy(char *restrict s1, const char *restrict s2, size_t n)
{
	for (int i = 0; i < n; ++i)
	{
		char buf = s2[i];
		s1[i] = buf;
		if (buf != '\0')
			continue;
		for (int j = i + 1; j < n; ++j)
			s1[j] = '\0';
		return s1 + i;
	}
	return s1 + n;
}

export char *strcat(char *restrict s1, const char *restrict s2)
{
	char *dest = s1;
	while (*dest)
		dest++;
	while ((*dest++ = *s2++))
		;
	return s1;
}

export char *strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return (char *)s;
		s++;
	}
	if (c == '\0')
		return (char *)s;
	return NULL;
}

export int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}

	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

export int strcoll(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}

	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

export int strcoll_l(const char *s1, const char *s2, locale_t locale);

export char *strcpy(char *restrict s1, const char *restrict s2)
{
	char *dest = s1;
	while (*s2)
		*dest++ = *s2++;
	*dest = '\0';
	return s1;
}

export size_t strcspn(const char *s1, const char *s2)
{
	const char *p = s1;
	const char *spanp;
	char c, sc;

	while ((c = *p++) != 0)
	{
		for (spanp = s2; (sc = *spanp++) != 0;)
		{
			if (sc == c)
				return p - 1 - s1;
		}
	}
	return p - 1 - s1;
}

export char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *new_str = (char *)malloc(len);
	if (!new_str)
		return NULL;
	memcpy(new_str, s, len);
	return new_str;
}

/* already defined in src/std/errno.c */
// export char *strerror(int errnum)
// {
// 	static char unknown_error[32];
// 	switch (errnum)
// 	{
// 	case 0:
// 		return "No error";
// 	case E2BIG:
// 		return "Argument list too long";
// 	case EACCES:
// 		return "Permission denied";
// 	case EADDRINUSE:
// 		return "Address in use";
// 	case EADDRNOTAVAIL:
// 		return "Address not available";
// 	case EAFNOSUPPORT:
// 		return "Address family not supported";
// 	case EAGAIN:
// 		return "Resource temporarily unavailable";
// 	case EALREADY:
// 		return "Connection already in progress";
// 	case EBADF:
// 		return "Bad file descriptor";
// 	case EBADMSG:
// 		return "Bad message";
// 	case EBUSY:
// 		return "Resource busy";
// 	case ECANCELED:
// 		return "Operation canceled";
// 	case ECHILD:
// 		return "No child process";
// 	case ECONNABORTED:
// 		return "Connection aborted";
// 	case ECONNREFUSED:
// 		return "Connection refused";
// 	case ECONNRESET:
// 		return "Connection reset";
// 	case EDEADLK:
// 		return "Resource deadlock would occur";
// 	case EDESTADDRREQ:
// 		return "Destination address required";
// 	case EDOM:
// 		return "Domain error";
// 	case EEXIST:
// 		return "File exists";
// 	case EFAULT:
// 		return "Bad address";
// 	case EFBIG:
// 		return "File too large";
// 	case EHOSTUNREACH:
// 		return "Host is unreachable";
// 	case EIDRM:
// 		return "Identifier removed";
// 	case EILSEQ:
// 		return "Illegal byte sequence";
// 	case EINPROGRESS:
// 		return "Operation in progress";
// 	case EINTR:
// 		return "Interrupted function call";
// 	case EINVAL:
// 		return "Invalid argument";
// 	case EIO:
// 		return "Input/output error";
// 	case EISCONN:
// 		return "Socket is connected";
// 	case EISDIR:
// 		return "Is a directory";
// 	case ELOOP:
// 		return "Symbolic link loop";
// 	case EMFILE:
// 		return "File descriptor value too large or too many open streams";
// 	case EMLINK:
// 		return "Too many links";
// 	case EMSGSIZE:
// 		return "Message too large";
// 	case ENAMETOOLONG:
// 		return "Filename too long";
// 	case ENETDOWN:
// 		return "Network is down";
// 	case ENETRESET:
// 		return "The connection was aborted by the network";
// 	case ENETUNREACH:
// 		return "Network unreachable";
// 	case ENFILE:
// 		return "Too many files open in system";
// 	case ENOBUFS:
// 		return "No buffer space available";
// 	case ENODATA:
// 		return "No message available";
// 	case ENODEV:
// 		return "No such device";
// 	case ENOENT:
// 		return "No such file or directory";
// 	case ENOEXEC:
// 		return "Executable file format error";
// 	case ENOLCK:
// 		return "No locks available";
// 	case ENOMEM:
// 		return "Not enough space";
// 	case ENOMSG:
// 		return "No message of the desired type";
// 	case ENOPROTOOPT:
// 		return "Protocol not available";
// 	case ENOSPC:
// 		return "No space left on device";
// 	case ENOSR:
// 		return "No STREAM resources";
// 	case ENOSTR:
// 		return "Not a STREAM";
// 	case ENOSYS:
// 		return "Functionality not supported";
// 	case ENOTCONN:
// 		return "Socket not connected";
// 	case ENOTDIR:
// 		return "Not a directory";
// 	case ENOTEMPTY:
// 		return "Directory not empty";
// 	case ENOTRECOVERABLE:
// 		return "State not recoverable";
// 	case ENOTSOCK:
// 		return "Not a socket";
// 	case ENOTSUP:
// 		return "Not supported";
// 	case ENOTTY:
// 		return "Inappropriate I/O control operation";
// 	case ENXIO:
// 		return "No such device or address";
// 	case EOPNOTSUPP:
// 		return "Operation not supported on socket";
// 	case EOVERFLOW:
// 		return "Value too large to be stored in data type";
// 	case EOWNERDEAD:
// 		return "Previous owner died";
// 	case EPERM:
// 		return "Operation not permitted";
// 	case EPIPE:
// 		return "Broken pipe";
// 	case EPROTO:
// 		return "Protocol error";
// 	case EPROTONOSUPPORT:
// 		return "Protocol not supported";
// 	case EPROTOTYPE:
// 		return "Protocol wrong type for socket";
// 	case ERANGE:
// 		return "Result too large";
// 	case EROFS:
// 		return "Read-only file system";
// 	case ESPIPE:
// 		return "Invalid seek";
// 	case ESRCH:
// 		return "No such process";
// 	case ETIME:
// 		return "STREAM ioctl() timeout";
// 	case ETIMEDOUT:
// 		return "Connection timed out";
// 	case ETXTBSY:
// 		return "Text file busy";
// 	case EWOULDBLOCK:
// 		return "Operation would block";
// 	case EXDEV:
// 		return "Improper link";
// 	default:
// 		snprintf(unknown_error, sizeof(unknown_error), "Unknown error %d", errnum);
// 		return unknown_error;
// 	}
// }

export char *strerror_l(int errnum, locale_t locale);

export int strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
	const char *errmsg = strerror(errnum);
	if (strlen(errmsg) >= buflen)
	{
		if (buflen > 0)
		{
			strncpy(strerrbuf, errmsg, buflen - 1);
			strerrbuf[buflen - 1] = '\0';
		}
		return ERANGE;
	}
	else
	{
		strcpy(strerrbuf, errmsg);
		return 0;
	}
}

size_t strlcat(char *restrict dst, const char *restrict src, size_t dstsize)
{
	size_t dst_len = strnlen(dst, dstsize);
	size_t src_len = strlen(src);

	if (dst_len == dstsize)
		return dstsize + src_len;

	if (src_len < dstsize - dst_len)
		memcpy(dst + dst_len, src, src_len + 1);
	else
	{
		memcpy(dst + dst_len, src, dstsize - dst_len - 1);
		dst[dstsize - 1] = '\0';
	}

	return dst_len + src_len;
}

size_t strlcpy(char *restrict dst, const char *restrict src, size_t dstsize)
{
	size_t src_len = strlen(src);

	if (dstsize != 0)
	{
		size_t copy_len = (src_len >= dstsize) ? dstsize - 1 : src_len;
		memcpy(dst, src, copy_len);
		dst[copy_len] = '\0';
	}

	return src_len;
}

export size_t strlen(const char *s)
{
	const char *start = s;
	while (*s)
		s++;
	return s - start;
}

export char *strncat(char *restrict s1, const char *restrict s2, size_t n)
{
	char *dest = s1;
	while (*dest)
		dest++;

	while (n-- && *s2)
		*dest++ = *s2++;

	*dest = '\0';
	return s1;
}

export int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n--)
	{
		if (*s1 != *s2)
			return *(unsigned char *)s1 - *(unsigned char *)s2;
		if (*s1 == '\0')
			return 0;
		s1++;
		s2++;
	}
	return 0;
}

export char *strncpy(char *restrict s1, const char *restrict s2, size_t n)
{
	char *dest = s1;
	while (n && (*dest++ = *s2++))
		n--;

	if (n)
		while (--n)
			*dest++ = '\0';

	return s1;
}

export char *strndup(const char *s, size_t size)
{
	size_t len = strnlen(s, size);
	char *new_str = (char *)malloc(len + 1);
	if (!new_str)
		return NULL;
	memcpy(new_str, s, len);
	new_str[len] = '\0';
	return new_str;
}

export size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;
	while (len < maxlen && s[len] != '\0')
		len++;
	return len;
}

export char *strpbrk(const char *s1, const char *s2)
{
	while (*s1)
	{
		const char *s = s2;
		while (*s)
		{
			if (*s1 == *s)
				return (char *)s1;
			s++;
		}
		s1++;
	}
	return NULL;
}

export char *strrchr(const char *s, int c)
{
	const char *last = NULL;
	do
	{
		if (*s == (char)c)
			last = s;
	} while (*s++);
	return (char *)last;
}

export char *strsignal(int signum)
{
	switch (signum)
	{
	case SIGNULL:
		return "NULL signal";
	case SIGABRT:
		return "Aborted";
	case SIGALRM:
		return "Alarm clock";
	case SIGBUS:
		return "Bus error";
	case SIGCHLD:
		return "Child status changed";
	case SIGCONT:
		return "Continued";
	case SIGFPE:
		return "Arithmetic exception";
	case SIGHUP:
		return "Hangup";
	case SIGILL:
		return "Illegal instruction";
	case SIGINT:
		return "Interrupt";
	case SIGKILL:
		return "Killed";
	case SIGPIPE:
		return "Broken pipe";
	case SIGQUIT:
		return "Quit";
	case SIGSEGV:
		return "Segmentation fault";
	case SIGSTOP:
		return "Stopped (signal)";
	case SIGTERM:
		return "Terminated";
	case SIGTSTP:
		return "Stopped (user)";
	case SIGTTIN:
		return "Stopped (tty input)";
	case SIGTTOU:
		return "Stopped (tty output)";
	case SIGUSR1:
		return "User defined signal 1";
	case SIGUSR2:
		return "User defined signal 2";
	case SIGPOLL:
		return "Pollable event occurred";
	case SIGPROF:
		return "Profiling timer expired";
	case SIGSYS:
		return "Bad system call";
	case SIGTRAP:
		return "Trace/breakpoint trap";
	case SIGURG:
		return "Urgent I/O condition";
	case SIGVTALRM:
		return "Virtual timer expired";
	case SIGXCPU:
		return "CPU time limit exceeded";
	case SIGXFSZ:
		return "File size limit exceeded";
	default:
		return "Unknown signal";
	}
}

export size_t strspn(const char *s1, const char *s2)
{
	const char *p = s1;
	const char *spanp;
	char c, sc;

cont:
	c = *p++;
	for (spanp = s2; (sc = *spanp++) != 0;)
	{
		if (sc == c)
			goto cont;
	}
	return p - 1 - s1;
}

export char *strstr(const char *s1, const char *s2)
{
	if (!*s2)
		return (char *)s1;

	for (; *s1; s1++)
	{
		const char *h = s1;
		const char *n = s2;

		while (*h && *n && *h == *n)
		{
			h++;
			n++;
		}

		if (!*n)
			return (char *)s1;
	}

	return NULL;
}

export char *strtok(char *restrict s, const char *restrict sep)
{
	static char *last;
	return strtok_r(s, sep, &last);
}

export char *strtok_r(char *restrict s, const char *restrict sep, char **restrict state)
{
	char *start;
	char *end;

	if (s == NULL)
		s = *state;

	s += strspn(s, sep);
	if (*s == '\0')
	{
		*state = s;
		return NULL;
	}

	start = s;
	end = strpbrk(start, sep);
	if (end)
	{
		*end = '\0';
		*state = end + 1;
	}
	else
		*state = start + strlen(start);

	return start;
}

export size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n)
{
	size_t len = 0;
	while (s2[len] != '\0' && len < n - 1)
	{
		s1[len] = s2[len];
		len++;
	}
	if (n > 0)
		s1[len] = '\0';
	while (s2[len] != '\0')
		len++;
	return len;
}

export size_t strxfrm_l(char *restrict s1, const char *restrict s2, size_t n, locale_t locale);
