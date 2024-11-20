#include <string.h>

#include "../../mem/liballoc_1_1.h"

PUBLIC size_t strlen(const char *str)
{
	long unsigned i = 0;
	if (str)
		while (str[i] != '\0')
			++i;
	return i;
}

PUBLIC int strcmp(const char *l, const char *r)
{
	for (; *l == *r && *l; l++, r++)
		;
	return *(unsigned char *)l - *(unsigned char *)r;
}

PUBLIC int strncmp(const char *s1, const char *s2, size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		char c1 = s1[i], c2 = s2[i];
		if (c1 != c2)
			return c1 - c2;
		if (!c1)
			return 0;
	}
	return 0;
}

PUBLIC int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;
	int result;
	if (p1 == p2)
		return 0;
	while ((result = tolower(*p1) - tolower(*p2++)) == 0)
		if (*p1++ == '\0')
			break;
	return result;
}

PUBLIC int strncasecmp(const char *string1, const char *string2, size_t count)
{
	if (count)
	{
		const unsigned char *s1 = (const unsigned char *)string1;
		const unsigned char *s2 = (const unsigned char *)string2;
		int result;
		do
		{
			if ((result = tolower(*s1) - tolower(*s2++)) != 0 || !*s1++)
				break;
		} while (--count);
		return result;
	}
	return 0;
}

PUBLIC char *strstr(const char *haystack, const char *needle)
{
	const char *a = haystack, *b = needle;
	while (1)
	{
		if (!*b)
			return (char *)haystack;
		if (!*a)
			return NULL;
		if (*a++ != *b++)
		{
			a = ++haystack;
			b = needle;
		}
	}
}

PUBLIC char *strncpy(char *destination, const char *source, unsigned long num)
{
	if (destination == NULL)
		return NULL;
	char *ptr = destination;
	while (*source && num--)
	{
		*destination = *source;
		destination++;
		source++;
	}
	*destination = '\0';
	return ptr;
}

PUBLIC char *strdup(const char *s)
{
	char *buf = (char *)__malloc(strlen((char *)s) + 1);
	strncpy(buf, s, strlen(s) + 1);
	return buf;
}

PUBLIC char *strchr(char const *s, int c)
{
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++)
		if (s[i] == c)
			return (char *)s + i;

	return NULL;
}

PUBLIC char *strrchr(char const *s, int c)
{
	size_t len = strlen(s);
	size_t pos = len;

	while (s[pos] != c && pos-- != 0)
		;

	if (pos == len)
		return NULL;

	return (char *)s + pos;
}
