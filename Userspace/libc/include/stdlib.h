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

#ifndef _STDLIB_H
#define _STDLIB_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <sys/wait.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define NULL ((void *)0)
#define RAND_MAX 32767

#define MB_CUR_MAX 4

	typedef struct
	{
		int quot;
		int rem;
	} div_t;
	typedef struct
	{
		long quot;
		long rem;
	} ldiv_t;

	typedef __SIZE_TYPE__ size_t;
	// typedef __WCHAR_TYPE__ wchar_t;

	long a64l(const char *);
	void abort(void);
	int abs(int);
	int atexit(void (*func)(void));
	double atof(const char *);
	int atoi(const char *);
	long int atol(const char *);
	void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
	void *calloc(size_t, size_t);
	div_t div(int, int);
	double drand48(void);
	char *ecvt(double, int, int *, int *);
	double erand48(unsigned short int[3]);
	void exit(int status);
	char *fcvt(double, int, int *, int *);
	void free(void *ptr);
	char *gcvt(double, int, char *);
	char *getenv(const char *);
	int getsubopt(char **, char *const *, char **);
	int grantpt(int);
	char *initstate(unsigned int, char *, size_t);
	long int jrand48(unsigned short int[3]);
	char *l64a(long);
	long int labs(long int);
	void lcong48(unsigned short int[7]);
	ldiv_t ldiv(long int, long int);
	long int lrand48(void);
	void *malloc(size_t size);
	int mblen(const char *, size_t);
	size_t mbstowcs(wchar_t *, const char *, size_t);
	int mbtowc(wchar_t *, const char *, size_t);
	char *mktemp(char *);
	int mkstemp(char *);
	long int mrand48(void);
	long int nrand48(unsigned short int[3]);
	char *ptsname(int);
	int putenv(char *);
	void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
	int rand(void);
	int rand_r(unsigned int *);
	long random(void);
	void *realloc(void *, size_t);
	char *realpath(const char *, char *);
	unsigned short int seed48(unsigned short int[3]);
	void setkey(const char *);
	char *setstate(const char *);
	void srand(unsigned int);
	void srand48(long int);
	void srandom(unsigned);
	double strtod(const char *, char **);
	long int strtol(const char *, char **, int);
	unsigned long int strtoul(const char *, char **, int);
	int system(const char *);
	int ttyslot(void);
	int unlockpt(int);
	void *valloc(size_t);
	size_t wcstombs(char *, const wchar_t *, size_t);
	int wctomb(char *, wchar_t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STDLIB_H
