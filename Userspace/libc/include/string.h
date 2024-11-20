#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

	size_t strlen(const char *str);
	int strcmp(const char *l, const char *r);
	int strncmp(const char *s1, const char *s2, size_t n);
	int strcasecmp(const char *s1, const char *s2);
	int strncasecmp(const char *string1, const char *string2, size_t count);
	char *strstr(const char *haystack, const char *needle);
	char *strncpy(char *destination, const char *source, unsigned long num);
	char *strdup(const char *s);
	char *strchr(char const *s, int c);
	char *strrchr(char const *s, int c);

	void *memcpy(void *dest, const void *src, size_t n);
	void *memset(void *dest, int c, size_t n);
	void *memmove(void *dest, const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
