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

	void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n);
	void *memchr(const void *s, int c, size_t n);
	int memcmp(const void *s1, const void *s2, size_t n);
	void *memcpy(void *restrict s1, const void *restrict s2, size_t n);
	void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
	void *memmove(void *s1, const void *s2, size_t n);
	void *memset(void *s, int c, size_t n);
	char *stpcpy(char *restrict s1, const char *restrict s2);
	char *stpncpy(char *restrict s1, const char *restrict s2, size_t n);
	char *strcat(char *restrict s1, const char *restrict s2);
	char *strchr(const char *s, int c);
	int strcmp(const char *s1, const char *s2);
	int strcoll(const char *s1, const char *s2);
	int strcoll_l(const char *s1, const char *s2, locale_t locale);
	char *strcpy(char *restrict s1, const char *restrict s2);
	size_t strcspn(const char *s1, const char *s2);
	char *strdup(const char *s);
	char *strerror(int errnum);
	char *strerror_l(int errnum, locale_t locale);
	int strerror_r(int errnum, char *strerrbuf, size_t buflen);
	size_t strlcat(char *restrict dst, const char *restrict src, size_t dstsize);
	size_t strlcpy(char *restrict dst, const char *restrict src, size_t dstsize);
	size_t strlen(const char *s);
	char *strncat(char *restrict s1, const char *restrict s2, size_t n);
	int strncmp(const char *s1, const char *s2, size_t n);
	char *strncpy(char *restrict s1, const char *restrict s2, size_t n);
	char *strndup(const char *s, size_t size);
	size_t strnlen(const char *s, size_t maxlen);
	char *strpbrk(const char *s1, const char *s2);
	char *strrchr(const char *s, int c);
	char *strsignal(int signum);
	size_t strspn(const char *s1, const char *s2);
	char *strstr(const char *s1, const char *s2);
	char *strtok(char *restrict s, const char *restrict sep);
	char *strtok_r(char *restrict s, const char *restrict sep, char **restrict state);
	size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n);
	size_t strxfrm_l(char *restrict s1, const char *restrict s2, size_t n, locale_t locale);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STRING_H
