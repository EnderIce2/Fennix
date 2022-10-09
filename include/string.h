#pragma once
#include <types.h>

#ifdef __cplusplus
extern "C"
{
#endif
    int isdigit(int c);
    int isspace(int c);
    int isempty(char *str);
    unsigned int isdelim(char c, char *delim);

    void *memcpy(void *dest, const void *src, size_t n);
    void *memset(void *dest, int c, size_t n);
    void *memmove(void *dest, const void *src, size_t n);
    int memcmp(const void *vl, const void *vr, size_t n);
    long unsigned strlen(const char s[]);
    int strncmp(const char *s1, const char *s2, unsigned long n);
    char *strcat(char *destination, const char *source);
    char *strcpy(char *destination, const char *source);
    char *strncpy(char *destination, const char *source, unsigned long num);
    int strcmp(const char *l, const char *r);
    char *strstr(const char *haystack, const char *needle);
    char *strdup(const char *String);
    char *strchr(const char *String, int Char);
    char *strrchr(const char *String, int Char);
    int strncasecmp(const char *lhs, const char *rhs, long unsigned int Count);
    int strcasecmp(const char *lhs, const char *rhs);
    char *strtok(char *src, const char *delim);

#ifdef __cplusplus
}
#endif
