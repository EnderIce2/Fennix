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
export char *strchr(const char *, int);

export int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}

	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

export int strcoll(const char *, const char *);
export int strcoll_l(const char *, const char *, locale_t);
export char *strcpy(char *restrict, const char *restrict);
export size_t strcspn(const char *, const char *);
export char *strdup(const char *);
export char *strerror(int);
export char *strerror_l(int, locale_t);
export int strerror_r(int, char *, size_t);
export size_t strlcat(char *restrict, const char *restrict, size_t);
export size_t strlcpy(char *restrict, const char *restrict, size_t);
export size_t strlen(const char *);
export char *strncat(char *restrict, const char *restrict, size_t);
export int strncmp(const char *, const char *, size_t);
export char *strncpy(char *restrict, const char *restrict, size_t);
export char *strndup(const char *, size_t);
export size_t strnlen(const char *, size_t);
export char *strpbrk(const char *, const char *);
export char *strrchr(const char *, int);
export char *strsignal(int);
export size_t strspn(const char *, const char *);
export char *strstr(const char *, const char *);
export char *strtok(char *restrict, const char *restrict);
export char *strtok_r(char *restrict, const char *restrict, char **restrict);
export size_t strxfrm(char *restrict, const char *restrict, size_t);
export size_t strxfrm_l(char *restrict, const char *restrict, size_t, locale_t);
