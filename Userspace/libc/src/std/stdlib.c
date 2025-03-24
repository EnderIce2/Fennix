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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "../mem/liballoc_1_1.h"

#define MAX_ATEXIT_FUNCS 32
typedef void (*atexit_func_t)(void);
static atexit_func_t __atexit_funcs[MAX_ATEXIT_FUNCS];
static int __num_atexit_funcs = 0;

export long a64l(const char *);
export _Noreturn void abort(void)
{
	kill(getpid(), SIGABRT);
	__builtin_unreachable();
}

export int abs(int i)
{
	return (i < 0) ? -i : i;
}

export int atexit(void (*func)(void))
{
	if (__num_atexit_funcs >= MAX_ATEXIT_FUNCS)
		return -1;
	__atexit_funcs[__num_atexit_funcs++] = func;
	return 0;
}

export double atof(const char *);

export int atoi(const char *str)
{
	return (int)strtol(str, (char **)NULL, 10);
}

export long int atol(const char *);
export void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
export void *calloc(size_t, size_t);
export div_t div(int, int);
export double drand48(void);
export char *ecvt(double, int, int *, int *);
export double erand48(unsigned short int[3]);

export void exit(int status)
{
	for (int i = __num_atexit_funcs - 1; i >= 0; --i)
		__atexit_funcs[i]();
	_exit(status);
	__builtin_unreachable();
}

export char *fcvt(double, int, int *, int *);
export void free(void *ptr) { return PREFIX(free)(ptr); }
export char *gcvt(double, int, char *);

export char *getenv(const char *name)
{
	if (name == NULL)
		return NULL;

	size_t len = strlen(name);
	for (char **env = environ; *env != 0; ++env)
	{
		char *thisEnv = *env;
		char cmp = strncmp(thisEnv, name, len) == 0;
		if (cmp && thisEnv[len] == '=')
			return thisEnv + len + 1;
	}
	return NULL;
}

export int getsubopt(char **, char *const *, char **);
export int grantpt(int);
export char *initstate(unsigned int, char *, size_t);
export long int jrand48(unsigned short int[3]);
export char *l64a(long);
export long int labs(long int);
export void lcong48(unsigned short int[7]);
export ldiv_t ldiv(long int, long int);
export long int lrand48(void);
export void *malloc(size_t size) { return PREFIX(malloc)(size); }
export int mblen(const char *, size_t);
export size_t mbstowcs(wchar_t *, const char *, size_t);
export int mbtowc(wchar_t *, const char *, size_t);
export char *mktemp(char *);
export int mkstemp(char *);
export long int mrand48(void);
export long int nrand48(unsigned short int[3]);
export char *ptsname(int);
export int putenv(char *);

export void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *))
{
	if (nel < 2 || width == 0)
		return;

	char *pivot = (char *)base + (nel / 2) * width;
	char *left = (char *)base;
	char *right = (char *)base + (nel - 1) * width;

	while (left <= right)
	{
		while (compar(left, pivot) < 0)
			left += width;
		while (compar(right, pivot) > 0)
			right -= width;

		if (left <= right)
		{
			for (size_t i = 0; i < width; i++)
			{
				char tmp = left[i];
				left[i] = right[i];
				right[i] = tmp;
			}
			left += width;
			right -= width;
		}
	}

	size_t left_size = (right - (char *)base) / width + 1;
	size_t right_size = nel - left_size - 1;

	qsort(base, left_size, width, compar);
	qsort(left, right_size, width, compar);
}

export int rand(void);
export int rand_r(unsigned int *);
export long random(void);

export void *realloc(void *ptr, size_t size)
{
	return PREFIX(realloc)(ptr, size);
}

export void *reallocarray(void *ptr, size_t nelem, size_t elsize)
{
	if (nelem && elsize > __SIZE_MAX__ / nelem)
	{
		errno = ENOMEM;
		return NULL;
	}

	return PREFIX(realloc)(ptr, nelem * elsize);
}

export char *realpath(const char *, char *);
export unsigned short int seed48(unsigned short int[3]);
export void setkey(const char *);
export char *setstate(const char *);
export void srand(unsigned int);
export void srand48(long int);
export void srandom(unsigned);
export double strtod(const char *, char **);

export long strtol(const char *restrict nptr, char **restrict endptr, int base)
{
	const char *s = nptr;
	long acc = 0;
	int c;
	long cutoff;
	int neg = 0, any, cutlim;

	do
	{
		c = *s++;
	} while (isspace(c));

	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;

	if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	else if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = neg ? LONG_MIN : LONG_MAX;
	cutlim = cutoff % base;
	cutoff /= base;
	if (neg)
	{
		if (cutlim > 0)
		{
			cutlim -= base;
			cutoff += 1;
		}
		cutlim = -cutlim;
	}

	for (acc = 0, any = 0;; c = *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0)
	{
		acc = neg ? LONG_MIN : LONG_MAX;
		errno = ERANGE;
	}
	else if (neg)
		acc = -acc;

	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);

	return acc;
}

export long long strtoll(const char *restrict nptr, char **restrict endptr, int base)
{
	const char *s = nptr;
	long long acc = 0;
	int c;
	long long cutoff;
	int neg = 0, any, cutlim;

	do
	{
		c = *s++;
	} while (isspace(c));

	if (c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else if (c == '+')
		c = *s++;

	if ((base == 0 || base == 16) &&
		c == '0' && (*s == 'x' || *s == 'X'))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	else if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = neg ? LLONG_MIN : LLONG_MAX;
	cutlim = cutoff % base;
	cutoff /= base;
	if (neg)
	{
		if (cutlim > 0)
		{
			cutlim -= base;
			cutoff += 1;
		}
		cutlim = -cutlim;
	}

	for (acc = 0, any = 0;; c = *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0)
	{
		acc = neg ? LLONG_MIN : LLONG_MAX;
		errno = ERANGE;
	}
	else if (neg)
		acc = -acc;

	if (endptr != 0)
		*endptr = (char *)(any ? s - 1 : nptr);

	return acc;
}

export unsigned long int strtoul(const char *restrict str, char **restrict endptr, int base)
{
	const char *s = str;
	unsigned long acc = 0;
	int c;
	unsigned long cutoff;
	int any, cutlim;

	if (base < 0 || base == 1 || base > 36)
	{
		errno = EINVAL;
		if (endptr)
			*endptr = (char *)str;
		return 0;
	}

	while (isspace((unsigned char)*s))
		s++;

	if (*s == '+')
		s++;
	else if (*s == '-')
	{
		errno = EINVAL;
		if (endptr)
			*endptr = (char *)str;
		return 0;
	}

	if ((base == 0 || base == 16) && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
	{
		s += 2;
		base = 16;
	}
	else if (base == 0)
		base = *s == '0' ? 8 : 10;

	cutoff = ULONG_MAX / base;
	cutlim = ULONG_MAX % base;

	for (acc = 0, any = 0;; c = *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0)
	{
		acc = ULONG_MAX;
		errno = ERANGE;
	}
	else if (any == 0)
	{
		errno = EINVAL;
	}

	if (endptr)
		*endptr = (char *)(any ? s - 1 : str);

	return acc;
}

export unsigned long long strtoull(const char *restrict str, char **restrict endptr, int base)
{
	const char *s = str;
	unsigned long long acc = 0;
	int c;
	unsigned long long cutoff;
	int any, cutlim;

	if (base < 0 || base == 1 || base > 36)
	{
		errno = EINVAL;
		if (endptr)
			*endptr = (char *)str;
		return 0;
	}

	while (isspace((unsigned char)*s))
		s++;

	if (*s == '+')
		s++;
	else if (*s == '-')
	{
		errno = EINVAL;
		if (endptr)
			*endptr = (char *)str;
		return 0;
	}

	if ((base == 0 || base == 16) && *s == '0' && (s[1] == 'x' || s[1] == 'X'))
	{
		s += 2;
		base = 16;
	}
	else if (base == 0)
		base = *s == '0' ? 8 : 10;

	cutoff = ULLONG_MAX / base;
	cutlim = ULLONG_MAX % base;

	for (acc = 0, any = 0;; c = *s++)
	{
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;

		if (c >= base)
			break;

		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else
		{
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if (any < 0)
	{
		acc = ULLONG_MAX;
		errno = ERANGE;
	}
	else if (any == 0)
	{
		errno = EINVAL;
	}

	if (endptr)
		*endptr = (char *)(any ? s - 1 : str);

	return acc;
}

export int system(const char *command)
{
	if (command == NULL)
		return 1;

	pid_t pid;
	int status;
	struct sigaction sa, old_int, old_quit;
	sigset_t new_mask, old_mask;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, &old_int);
	sigaction(SIGQUIT, &sa, &old_quit);

	sigemptyset(&new_mask);
	sigaddset(&new_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &new_mask, &old_mask);

	if ((pid = fork()) == 0)
	{
		sigaction(SIGINT, &old_int, NULL);
		sigaction(SIGQUIT, &old_quit, NULL);
		sigprocmask(SIG_SETMASK, &old_mask, NULL);
		execl("/bin/sh", "sh", "-c", command, (char *)0);
		_exit(127);
	}

	if (pid == -1)
		status = -1;
	else
	{
		while (waitpid(pid, &status, 0) == -1)
		{
			if (errno != EINTR)
			{
				status = -1;
				break;
			}
		}
	}

	sigaction(SIGINT, &old_int, NULL);
	sigaction(SIGQUIT, &old_quit, NULL);
	sigprocmask(SIG_SETMASK, &old_mask, NULL);
	return status;
}

export int ttyslot(void);
export int unlockpt(int);
export void *valloc(size_t);
export size_t wcstombs(char *, const wchar_t *, size_t);
export int wctomb(char *, wchar_t);
