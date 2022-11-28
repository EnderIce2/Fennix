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
    int abs(int i);
    void swap(char *x, char *y);
    char *reverse(char *Buffer, int i, int j);

    void backspace(char s[]);
    void append(char s[], char n);

    int atoi(const char *String);
    double atof(const char *String);
    char *itoa(int Value, char *Buffer, int Base);
    char *ltoa(long Value, char *Buffer, int Base);
    char *ultoa(unsigned long Value, char *Buffer, int Base);

    void *memcpy_unsafe(void *dest, const void *src, size_t n);
    void *memset_unsafe(void *dest, int c, size_t n);
    void *memmove_unsafe(void *dest, const void *src, size_t n);
    int memcmp(const void *vl, const void *vr, size_t n);

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
    int strncasecmp(const char *lhs, const char *rhs, long unsigned int Count);
    int strcasecmp(const char *lhs, const char *rhs);
    char *strtok(char *src, const char *delim);

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
