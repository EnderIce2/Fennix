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
#include <fennix/syscalls.h>

export void *memccpy(void *restrict, const void *restrict, int, size_t);
export void *memchr(const void *, int, size_t);
export int memcmp(const void *, const void *, size_t);

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

export void *memmem(const void *, size_t, const void *, size_t);
export void *memmove(void *, const void *, size_t);
export void *memset(void *, int, size_t);
export char *stpcpy(char *restrict, const char *restrict);
export char *stpncpy(char *restrict, const char *restrict, size_t);
export char *strcat(char *restrict, const char *restrict);

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

export int strcoll_l(const char *, const char *, locale_t);

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

export char *strdup(const char *);
export char *strerror(int);
export char *strerror_l(int, locale_t);
export int strerror_r(int, char *, size_t);
export size_t strlcat(char *restrict, const char *restrict, size_t);
export size_t strlcpy(char *restrict, const char *restrict, size_t);

export size_t strlen(const char *s)
{
	const char *start = s;
	while (*s)
		s++;
	return s - start;
}

export char *strncat(char *restrict, const char *restrict, size_t);

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

export char *strndup(const char *, size_t);
export size_t strnlen(const char *, size_t);

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

export char *strrchr(const char *, int);

export char *strsignal(int signum)
{
	switch (signum)
	{
	case __SYS_SIGNULL:
		return "NULL signal";
	case __SYS_SIGABRT:
		return "Aborted";
	case __SYS_SIGALRM:
		return "Alarm clock";
	case __SYS_SIGBUS:
		return "Bus error";
	case __SYS_SIGCHLD:
		return "Child status changed";
	case __SYS_SIGCONT:
		return "Continued";
	case __SYS_SIGFPE:
		return "Arithmetic exception";
	case __SYS_SIGHUP:
		return "Hangup";
	case __SYS_SIGILL:
		return "Illegal instruction";
	case __SYS_SIGINT:
		return "Interrupt";
	case __SYS_SIGKILL:
		return "Killed";
	case __SYS_SIGPIPE:
		return "Broken pipe";
	case __SYS_SIGQUIT:
		return "Quit";
	case __SYS_SIGSEGV:
		return "Segmentation fault";
	case __SYS_SIGSTOP:
		return "Stopped (signal)";
	case __SYS_SIGTERM:
		return "Terminated";
	case __SYS_SIGTSTP:
		return "Stopped (user)";
	case __SYS_SIGTTIN:
		return "Stopped (tty input)";
	case __SYS_SIGTTOU:
		return "Stopped (tty output)";
	case __SYS_SIGUSR1:
		return "User defined signal 1";
	case __SYS_SIGUSR2:
		return "User defined signal 2";
	case __SYS_SIGPOLL:
		return "Pollable event occurred";
	case __SYS_SIGPROF:
		return "Profiling timer expired";
	case __SYS_SIGSYS:
		return "Bad system call";
	case __SYS_SIGTRAP:
		return "Trace/breakpoint trap";
	case __SYS_SIGURG:
		return "Urgent I/O condition";
	case __SYS_SIGVTALRM:
		return "Virtual timer expired";
	case __SYS_SIGXCPU:
		return "CPU time limit exceeded";
	case __SYS_SIGXFSZ:
		return "File size limit exceeded";
	default:
		return NULL;
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

export char *strstr(const char *, const char *);

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

export size_t strxfrm(char *restrict, const char *restrict, size_t);
export size_t strxfrm_l(char *restrict, const char *restrict, size_t, locale_t);
