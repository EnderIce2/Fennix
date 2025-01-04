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

export int abs(int);

export int atexit(void (*func)(void))
{
	if (__num_atexit_funcs >= MAX_ATEXIT_FUNCS)
		return -1;
	__atexit_funcs[__num_atexit_funcs++] = func;
	return 0;
}

export double atof(const char *);
export int atoi(const char *);
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
export char *getenv(const char *);
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
export void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
export int rand(void);
export int rand_r(unsigned int *);
export long random(void);
export void *realloc(void *, size_t);
export char *realpath(const char *, char *);
export unsigned short int seed48(unsigned short int[3]);
export void setkey(const char *);
export char *setstate(const char *);
export void srand(unsigned int);
export void srand48(long int);
export void srandom(unsigned);
export double strtod(const char *, char **);
export long int strtol(const char *, char **, int);
export unsigned long int strtoul(const char *, char **, int);
export int system(const char *);
export int ttyslot(void);
export int unlockpt(int);
export void *valloc(size_t);
export size_t wcstombs(char *, const wchar_t *, size_t);
export int wctomb(char *, wchar_t);
