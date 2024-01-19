/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <types.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct mbstate_t
	{
		int count;
		unsigned int value;
	} mbstate_t;

#define NAN (__builtin_nanf(""))

	int isdigit(int c);
	int isspace(int c);
	int isempty(char *str);
	int isalpha(int c);
	int isupper(int c);
	unsigned int isdelim(char c, const char *delim);
	long abs(long i);
	void swap(char *x, char *y);
	char *reverse(char *Buffer, int i, int j);

	float sqrtf(float x);
	double clamp(double x, double low, double high);

	float lerp(float a, float b, float t);
	float smoothstep(float a, float b, float t);
	float cubicInterpolate(float a, float b, float t);

	void backspace(char s[]);
	void append(char s[], char n);

	int atoi(const char *String);
	double atof(const char *String);
	char *itoa(int Value, char *Buffer, int Base);
	char *ltoa(long Value, char *Buffer, int Base);
	char *ultoa(unsigned long Value, char *Buffer, int Base);
	unsigned long int strtoul(const char *str, char **endptr, int base);

	void *memcpy_unsafe(void *dest, const void *src, size_t n);
	void *memset_unsafe(void *dest, int c, size_t n);
	void *memmove_unsafe(void *dest, const void *src, size_t n);
	int memcmp(const void *vl, const void *vr, size_t n);

	void *memcpy_sse(void *dest, const void *src, size_t n);
	void *memcpy_sse2(void *dest, const void *src, size_t n);
	void *memcpy_sse3(void *dest, const void *src, size_t n);
	void *memcpy_ssse3(void *dest, const void *src, size_t n);
	void *memcpy_sse4_1(void *dest, const void *src, size_t n);
	void *memcpy_sse4_2(void *dest, const void *src, size_t n);

	void *memset_sse(void *dest, int c, size_t n);
	void *memset_sse2(void *dest, int c, size_t n);
	void *memset_sse3(void *dest, int c, size_t n);
	void *memset_ssse3(void *dest, int c, size_t n);
	void *memset_sse4_1(void *dest, int c, size_t n);
	void *memset_sse4_2(void *dest, int c, size_t n);

	void *memmove_sse(void *dest, const void *src, size_t n);
	void *memmove_sse2(void *dest, const void *src, size_t n);
	void *memmove_sse3(void *dest, const void *src, size_t n);
	void *memmove_ssse3(void *dest, const void *src, size_t n);
	void *memmove_sse4_1(void *dest, const void *src, size_t n);
	void *memmove_sse4_2(void *dest, const void *src, size_t n);

	long unsigned __strlen(const char s[]);

	long unsigned strlen_sse(const char s[]);
	long unsigned strlen_sse2(const char s[]);
	long unsigned strlen_sse3(const char s[]);
	long unsigned strlen_ssse3(const char s[]);
	long unsigned strlen_sse4_1(const char s[]);
	long unsigned strlen_sse4_2(const char s[]);

	long unsigned strlen(const char s[]);
	int strncmp(const char *s1, const char *s2, unsigned long n);
	char *strcat_unsafe(char *destination, const char *source);
	char *strcpy_unsafe(char *destination, const char *source);
	char *strncpy(char *destination, const char *source, unsigned long num);
	int strcmp(const char *l, const char *r);
	char *strstr(const char *haystack, const char *needle);
	char *strdup(const char *String);
	char *strchr(const char *String, int Char);
	char *strrchr(const char *String, int Char);
	int strncasecmp(const char *string1, const char *string2, size_t count);
	int strcasecmp(const char *s1, const char *s2);
	char *strtok(char *src, const char *delim);
	long int strtol(const char *str, char **endptr, int base);
	size_t wcslen(const wchar_t *s);
	size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps);
	int log2(unsigned int n);

	void *__memcpy_chk(void *dest, const void *src, size_t len, size_t slen);
	void *__memset_chk(void *dest, int val, size_t len, size_t slen);
	void *__memmove_chk(void *dest, const void *src, size_t len, size_t slen);
	char *__strcat_chk(char *dest, const char *src, size_t slen);
	char *__strcpy_chk(char *dest, const char *src, size_t slen);

#ifdef __cplusplus
}
#endif

#undef memcpy
#define memcpy(dest, src, n) \
	__memcpy_chk(dest, src, n, __builtin_object_size(dest, 0))

#undef memset
#define memset(dest, c, n) \
	__memset_chk(dest, c, n, __builtin_object_size(dest, 0))

#undef memmove
#define memmove(dest, src, n) \
	__memmove_chk(dest, src, n, __builtin_object_size(dest, 0))

#undef strcat
#define strcat(dest, src) \
	__strcat_chk(dest, src, __builtin_object_size(dest, 0))

#undef strcpy
#define strcpy(dest, src) \
	__strcpy_chk(dest, src, __builtin_object_size(dest, 0))
