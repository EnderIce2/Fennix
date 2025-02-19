/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>

void __conv_glibc(char **ptr)
{
	__UINTPTR_TYPE__ tmp = (__UINTPTR_TYPE__)*ptr;
#ifdef __linux__
	tmp &= 0x7fffffffffff;
#endif
	*ptr = (char *)tmp;
}

int test_memccpy(void)
{
	char src[20] = "test\0string";
	char dest[20];

	memset(dest, 0x55, sizeof(dest));
	void *result = memccpy(dest, src, '\0', sizeof(src));

	if (memcmp(dest, "test\0", 5) != 0)
		return 1;

	if (memccpy(dest, src, 'x', 10) != NULL)
		return 2;

	if (memccpy(dest, src, 't', 0) != NULL)
		return 3;
	return 0;
}

int test_memchr(void)
{
	const char *str = "abcdef\0ghijkl";

	if (memchr(str, 'd', 10) != str + 3)
		return 1;

	if (memchr(str, '\0', 10) != str + 6)
		return 2;

	if (memchr(str, 'x', 10) != NULL)
		return 3;

	if (memchr(str, 'g', 7) != NULL)
		return 4;
	return 0;
}

int test_memcmp(void)
{
	char a[] = {1, 2, 3, 4};
	char b[] = {1, 2, 4, 4};

	if (memcmp(a, a, 4) != 0)
		return 1;

	if (memcmp(a, b, 4) >= 0)
		return 2;

	if (memcmp(a, b, 0) != 0)
		return 3;
	return 0;
}

int test_memcpy(void)
{
	char src[20] = "test string";
	char dest[20];

	if (memcpy(dest, src, strlen(src) + 1) != dest)
		return 1;
	if (strcmp(dest, src) != 0)
		return 2;

	if (memcpy(dest, src, 0) != dest)
		return 3;
	return 0;
}

int test_memmem(void)
{
	const char *haystack = "abc123def123ghi";
	size_t haylen = strlen(haystack);
	const char *needle = "123";
	size_t ndlen = strlen(needle);

	if (memmem(haystack, haylen, needle, ndlen) != haystack + 3)
		return 1;

	const void *second = memmem(haystack + 4, haylen - 4, needle, ndlen);
	if (second != haystack + 9)
		return 2;

	if (memmem(haystack, haylen, "xyz", 3) != NULL)
		return 3;

	if (memmem(haystack, haylen, "", 0) != haystack)
		return 4;
	return 0;
}

int test_memmove(void)
{
	char buf[20] = "abcdefghij";

	if (memmove(buf + 2, buf, 5) != buf + 2)
		return 1;
	if (memcmp(buf, "ababcdehij", 10) != 0)
		return 2;

	memcpy(buf, "abcdefghij", 10);
	if (memmove(buf, buf + 3, 4) != buf)
		return 3;
	if (memcmp(buf, "defgefghij", 10) != 0)
		return 4;
	return 0;
}

int test_memset(void)
{
	char buf[10];
	const char pattern = 0xAA;

	if (memset(buf, pattern, sizeof(buf)) != buf)
		return 1;
	for (size_t i = 0; i < sizeof(buf); i++)
		if (buf[i] != pattern)
			return 2;

	if (memset(buf, 0, 5) != buf)
		return 3;
	for (int i = 0; i < 5; i++)
		if (buf[i] != 0)
			return 4;
	return 0;
}

int test_stpcpy(void)
{
	char dest[20];
	const char *src = "test string";

	char *end = stpcpy(dest, src);
	__conv_glibc(&end);
	char *end2 = dest + strlen(src);
	if (end != end2)
		return 1;
	if (strcmp(dest, src) != 0)
		return 2;

	*dest = 0x55;
	end = stpcpy(dest, "");
	__conv_glibc(&end);
	if (end != dest)
		return 3;
	if (*dest != '\0')
		return 4;
	return 0;
}

int test_stpncpy(void)
{
	char dest[20];
	const char *src = "test";

	memset(dest, 0x55, sizeof(dest));
	char *end = stpncpy(dest, src, 4);
	__conv_glibc(&end);
	if (end != dest + 4)
		return 1;
	if (memcmp(dest, "test\x55", 5) != 0)
		return 2;

	end = stpncpy(dest, src, 6);
	__conv_glibc(&end);
	if (end != dest + 4)
		return 3;
	if (memcmp(dest, "test\0\0", 6) != 0)
		return 4;
	return 0;
}

int test_strcat(void)
{
	char dest[20] = "Hello";
	const char *src = " World";

	if (strcat(dest, src) != dest)
		return 1;
	if (strcmp(dest, "Hello World") != 0)
		return 2;

	dest[0] = '\0';
	if (strcat(dest, "test") != dest)
		return 3;
	if (strcmp(dest, "test") != 0)
		return 4;
	return 0;
}

int test_strchr(void)
{
	const char *str = "abcdef\0ghijkl";

	if (strchr(str, 'c') != str + 2)
		return 1;

	if (strchr(str, '\0') != str + 6)
		return 2;

	if (strchr(str, 'x') != NULL)
		return 3;
	return 0;
}

int test_strcmp(void)
{
	if (strcmp("test", "test") != 0)
		return 1;

	if (strcmp("apple", "banana") >= 0)
		return 2;
	if (strcmp("banana", "apple") <= 0)
		return 3;

	if (strcmp("test", "testing") >= 0)
		return 4;
	return 0;
}

int test_strcoll(void)
{
	if (strcoll("a", "b") != strcmp("a", "b"))
		return 1;
	if (strcoll("a", "a") != 0)
		return 2;
	return 0;
}

int test_strcoll_l(void)
{
	// locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
	// if (loc == NULL)
	// 	return 1;

	// int result = strcoll_l("a", "b", loc);
	// freelocale(loc);
	// return (result == strcmp("a", "b")) ? 0 : 2;
	return 0;
}

int test_strcpy(void)
{
	char dest[20];
	const char *src = "test string";

	if (strcpy(dest, src) != dest)
		return 1;
	if (strcmp(dest, src) != 0)
		return 2;

	// Empty string
	if (strcpy(dest, "") != dest)
		return 3;
	if (dest[0] != '\0')
		return 4;
	return 0;
}

int test_strcspn(void)
{
	// No matches
	if (strcspn("abcdef", "xyz") != 6)
		return 1;

	// First character match
	if (strcspn("abcdef", "a") != 0)
		return 2;

	// Middle match
	if (strcspn("abcdef", "d") != 3)
		return 3;
	return 0;
}

int test_strdup(void)
{
	const char *src = "test string";
	char *copy = strdup(src);

	if (copy == NULL)
		return 1;
	if (strcmp(copy, src) != 0)
		return 2;
	if (copy == src)
		return 3;

	free(copy);
	return 0;
}

// int test_strerror(void)
// {
// 	const char *e1 = strerror(EINVAL);
// 	if (e1 == NULL || e1[0] == '\0')
// 		return 1;

// 	const char *e2 = strerror(9999);
// 	if (e2 == NULL || e2[0] == '\0')
// 		return 2;
// return 0;
// }

int test_strerror_l(void)
{
	// locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
	// if (loc == NULL)
	// 	return 1;

	// const char *e1 = strerror_l(EINVAL, loc);
	// freelocale(loc);
	// if (e1 == NULL || e1[0] == '\0')
	// 	return 2;
	return 0;
}

int test_strerror_r(void)
{
	// char buf[256];

	// if (strerror_r(EINVAL, buf, sizeof(buf)) != 0)
	// 	return 1;
	// if (strlen(buf) == 0)
	// 	return 2;

	// if (strerror_r(EINVAL, buf, 5) != ERANGE)
	// 	return 3;
	return 0;
}

int test_strlcat(void)
{
	// char dest[10] = "Hello";

	// if (strlcat(dest, "World", sizeof(dest)) != 10)
	// 	return 1;
	// if (strcmp(dest, "HelloWor") != 0)
	// 	return 2;

	// dest[0] = '\0';
	// if (strlcat(dest, "test", sizeof(dest)) != 4)
	// 	return 3;
	return 0;
}

int test_strlcpy(void)
{
	// char dest[5];

	// if (strlcpy(dest, "Hello World", sizeof(dest)) != 11)
	// 	return 1;
	// if (strcmp(dest, "Hell") != 0)
	// 	return 2;

	// if (strlcpy(dest, "test", sizeof(dest)) != 4)
	// 	return 3;
	// if (strcmp(dest, "test") != 0)
	// 	return 4;
	return 0;
}

int test_strlen(void)
{
	if (strlen("") != 0)
		return 1;
	if (strlen("test") != 4)
		return 2;
	if (strlen("test\0string") != 4)
		return 3;
	return 0;
}

int test_strncat(void)
{
	char dest[10] = "Hello";

	if (strncat(dest, " World", 3) != dest)
		return 1;
	if (strcmp(dest, "Hello Wo") != 0)
		return 2;

	if (strncat(dest, "test", 0) != dest)
		return 3;
	return 0;
}

int test_strncmp(void)
{
	if (strncmp("apple", "applesauce", 5) != 0)
		return 1;

	if (strncmp("apple", "apricot", 3) >= 0)
		return 2;

	if (strncmp("apple", "banana", 0) != 0)
		return 3;
	return 0;
}

int test_strncpy(void)
{
	char dest[10];

	if (strncpy(dest, "test", 5) != dest)
		return 1;
	if (memcmp(dest, "test\0", 5) != 0)
		return 2;

	memset(dest, 0x55, sizeof(dest));
	if (strncpy(dest, "hi", 2) != dest)
		return 3;
	if (memcmp(dest, "hi\x55\x55\x55\x55\x55\x55\x55\x55", 10) != 0)
		return 4;
	return 0;
}

int test_strndup(void)
{
	const char *src = "test string";
	char *copy = strndup(src, 4);

	if (copy == NULL)
		return 1;
	if (strcmp(copy, "test") != 0)
		return 2;

	free(copy);
	return 0;
}

int test_strnlen(void)
{
	if (strnlen("test", 5) != 4)
		return 1;
	if (strnlen("test", 2) != 2)
		return 2;
	if (strnlen("", 10) != 0)
		return 3;
	return 0;
}

int test_strpbrk(void)
{
	const char *str = "abcdefghi";

	if (strpbrk(str, "xyz") != NULL)
		return 1;
	if (strpbrk(str, "def") != str + 3)
		return 2;
	if (strpbrk(str, "fed") != str + 3)
		return 3;
	return 0;
}

int test_strrchr(void)
{
	const char *str = "abcdabcd";

	if (strrchr(str, 'b') != str + 5)
		return 1;
	if (strrchr(str, '\0') != str + strlen(str))
		return 2;
	if (strrchr(str, 'x') != NULL)
		return 3;
	return 0;
}

int test_strsignal(void)
{
	const char *sig = strsignal(SIGSEGV);
	if (sig == NULL || sig[0] == '\0')
		return 1;

	sig = strsignal(999);
	if (sig == NULL || sig[0] == '\0')
		return 2;
	return 0;
}

int test_strspn(void)
{
	int result = strspn("abcdef", "abc");
	if (result != 3)
		return 1;

	result = strspn("aabbcc", "ab");
	if (result != 4)
		return 2;

	result = strspn("xyz", "abc");
	if (result != 0)
		return 3;
	return 0;
}

int test_strstr(void)
{
	const char *haystack = "The quick brown fox jumps over the lazy dog";

	if (strstr(haystack, "brown") != haystack + 10)
		return 1;
	if (strstr(haystack, "BROWN") != NULL)
		return 2;
	if (strstr(haystack, "") != haystack)
		return 3;
	return 0;
}

int test_strtok(void)
{
	char str[] = "a,b,c,d";
	char *token;

	if (strtok(str, ",") == NULL)
		return 1;
	if (strcmp(str, "a") != 0)
		return 2;

	int count = 0;
	while ((token = strtok(NULL, ",")) != NULL)
		count++;
	if (count != 3)
		return 3;
	return 0;
}

int test_strtok_r(void)
{
	char str[] = "1;2;3";
	char *saveptr;
	char *token;

	token = strtok_r(str, ";", &saveptr);
	__conv_glibc(&token);
	if (strcmp(token, "1") != 0)
		return 1;

	int count = 0;
	while ((token = strtok_r(NULL, ";", &saveptr)) != NULL)
		count++;
	if (count != 2)
		return 2;
	return 0;
}

int test_strxfrm(void)
{
	char dest[20];
	size_t len = strxfrm(dest, "apple", sizeof(dest));

	if (len >= sizeof(dest))
		return 1;
	if (strcmp(dest, "apple") != 0)
		return 2;
	return 0;
}

int test_strxfrm_l(void)
{
	// locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
	// if (loc == NULL)
	// 	return 1;

	// char dest[20];
	// size_t len = strxfrm_l(dest, "apple", sizeof(dest), loc);
	// freelocale(loc);

	// if (len >= sizeof(dest))
	// 	return 2;
	// if (strcmp(dest, "apple") != 0)
	// 	return 3;
	return 0;
}
