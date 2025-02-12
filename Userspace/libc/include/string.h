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

#ifndef _STRING_H
#define _STRING_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stddef.h>
#include <locale.h>

	void *memccpy(void *restrict, const void *restrict, int, size_t);
	void *memchr(const void *, int, size_t);
	int memcmp(const void *, const void *, size_t);
	void *memcpy(void *restrict s1, const void *restrict s2, size_t n);
	void *memmem(const void *, size_t, const void *, size_t);
	void *memmove(void *, const void *, size_t);
	void *memset(void *, int, size_t);
	char *stpcpy(char *restrict, const char *restrict);
	char *stpncpy(char *restrict, const char *restrict, size_t);
	char *strcat(char *restrict, const char *restrict);
	char *strchr(const char *s, int c);
	int strcmp(const char *s1, const char *s2);
	int strcoll(const char *, const char *);
	int strcoll_l(const char *, const char *, locale_t);
	char *strcpy(char *restrict, const char *restrict);
	size_t strcspn(const char *s1, const char *s2);
	char *strdup(const char *);
	char *strerror(int);
	char *strerror_l(int, locale_t);
	int strerror_r(int, char *, size_t);
	size_t strlcat(char *restrict, const char *restrict, size_t);
	size_t strlcpy(char *restrict, const char *restrict, size_t);
	size_t strlen(const char *s);
	char *strncat(char *restrict, const char *restrict, size_t);
	int strncmp(const char *s1, const char *s2, size_t n);
	char *strncpy(char *restrict s1, const char *restrict s2, size_t n);
	char *strndup(const char *, size_t);
	size_t strnlen(const char *, size_t);
	char *strpbrk(const char *s1, const char *s2);
	char *strrchr(const char *, int);
	char *strsignal(int signum);
	size_t strspn(const char *s1, const char *s2);
	char *strstr(const char *, const char *);
	char *strtok(char *restrict s, const char *restrict sep);
	char *strtok_r(char *restrict s, const char *restrict sep, char **restrict state);
	size_t strxfrm(char *restrict, const char *restrict, size_t);
	size_t strxfrm_l(char *restrict, const char *restrict, size_t, locale_t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STRING_H
