#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>

// TODO: Replace mem* with assembly code

/* Some of the functions are from musl library */
/* https://www.musl-libc.org/ */
/*
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void *memcpy_unsafe(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;

#ifdef __GNUC__

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LS >>
#define RS <<
#else
#define LS <<
#define RS >>
#endif

    typedef uint32_t __attribute__((__may_alias__)) u32;
    uint32_t w, x;

    for (; (uintptr_t)s % 4 && n; n--)
        *d++ = *s++;

    if ((uintptr_t)d % 4 == 0)
    {
        for (; n >= 16; s += 16, d += 16, n -= 16)
        {
            *(u32 *)(d + 0) = *(u32 *)(s + 0);
            *(u32 *)(d + 4) = *(u32 *)(s + 4);
            *(u32 *)(d + 8) = *(u32 *)(s + 8);
            *(u32 *)(d + 12) = *(u32 *)(s + 12);
        }
        if (n & 8)
        {
            *(u32 *)(d + 0) = *(u32 *)(s + 0);
            *(u32 *)(d + 4) = *(u32 *)(s + 4);
            d += 8;
            s += 8;
        }
        if (n & 4)
        {
            *(u32 *)(d + 0) = *(u32 *)(s + 0);
            d += 4;
            s += 4;
        }
        if (n & 2)
        {
            *d++ = *s++;
            *d++ = *s++;
        }
        if (n & 1)
        {
            *d = *s;
        }
        return dest;
    }

    if (n >= 32)
        switch ((uintptr_t)d % 4)
        {
        case 1:
            w = *(u32 *)s;
            *d++ = *s++;
            *d++ = *s++;
            *d++ = *s++;
            n -= 3;
            for (; n >= 17; s += 16, d += 16, n -= 16)
            {
                x = *(u32 *)(s + 1);
                *(u32 *)(d + 0) = (w LS 24) | (x RS 8);
                w = *(u32 *)(s + 5);
                *(u32 *)(d + 4) = (x LS 24) | (w RS 8);
                x = *(u32 *)(s + 9);
                *(u32 *)(d + 8) = (w LS 24) | (x RS 8);
                w = *(u32 *)(s + 13);
                *(u32 *)(d + 12) = (x LS 24) | (w RS 8);
            }
            break;
        case 2:
            w = *(u32 *)s;
            *d++ = *s++;
            *d++ = *s++;
            n -= 2;
            for (; n >= 18; s += 16, d += 16, n -= 16)
            {
                x = *(u32 *)(s + 2);
                *(u32 *)(d + 0) = (w LS 16) | (x RS 16);
                w = *(u32 *)(s + 6);
                *(u32 *)(d + 4) = (x LS 16) | (w RS 16);
                x = *(u32 *)(s + 10);
                *(u32 *)(d + 8) = (w LS 16) | (x RS 16);
                w = *(u32 *)(s + 14);
                *(u32 *)(d + 12) = (x LS 16) | (w RS 16);
            }
            break;
        case 3:
            w = *(u32 *)s;
            *d++ = *s++;
            n -= 1;
            for (; n >= 19; s += 16, d += 16, n -= 16)
            {
                x = *(u32 *)(s + 3);
                *(u32 *)(d + 0) = (w LS 8) | (x RS 24);
                w = *(u32 *)(s + 7);
                *(u32 *)(d + 4) = (x LS 8) | (w RS 24);
                x = *(u32 *)(s + 11);
                *(u32 *)(d + 8) = (w LS 8) | (x RS 24);
                w = *(u32 *)(s + 15);
                *(u32 *)(d + 12) = (x LS 8) | (w RS 24);
            }
            break;
        }
    if (n & 16)
    {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }
    if (n & 8)
    {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }
    if (n & 4)
    {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }
    if (n & 2)
    {
        *d++ = *s++;
        *d++ = *s++;
    }
    if (n & 1)
    {
        *d = *s;
    }
    return dest;
#endif

    for (; n; n--)
        *d++ = *s++;
    return dest;
}

void *memset_unsafe(void *dest, int c, size_t n)
{
    unsigned char *s = dest;
    size_t k;

    if (!n)
        return dest;
    s[0] = c;
    s[n - 1] = c;
    if (n <= 2)
        return dest;
    s[1] = c;
    s[2] = c;
    s[n - 2] = c;
    s[n - 3] = c;
    if (n <= 6)
        return dest;
    s[3] = c;
    s[n - 4] = c;
    if (n <= 8)
        return dest;

    k = -(uintptr_t)s & 3;
    s += k;
    n -= k;
    n &= -4;

#ifdef __GNUC__
    typedef uint32_t __attribute__((__may_alias__)) u32;
    typedef uint64_t __attribute__((__may_alias__)) u64;

    u32 c32 = ((u32)-1) / 255 * (unsigned char)c;
    *(u32 *)(s + 0) = c32;
    *(u32 *)(s + n - 4) = c32;
    if (n <= 8)
        return dest;
    *(u32 *)(s + 4) = c32;
    *(u32 *)(s + 8) = c32;
    *(u32 *)(s + n - 12) = c32;
    *(u32 *)(s + n - 8) = c32;
    if (n <= 24)
        return dest;
    *(u32 *)(s + 12) = c32;
    *(u32 *)(s + 16) = c32;
    *(u32 *)(s + 20) = c32;
    *(u32 *)(s + 24) = c32;
    *(u32 *)(s + n - 28) = c32;
    *(u32 *)(s + n - 24) = c32;
    *(u32 *)(s + n - 20) = c32;
    *(u32 *)(s + n - 16) = c32;

    k = 24 + ((uintptr_t)s & 4);
    s += k;
    n -= k;

    u64 c64 = c32 | ((u64)c32 << 32);
    for (; n >= 32; n -= 32, s += 32)
    {
        *(u64 *)(s + 0) = c64;
        *(u64 *)(s + 8) = c64;
        *(u64 *)(s + 16) = c64;
        *(u64 *)(s + 24) = c64;
    }
#else
    for (; n; n--, s++)
        *s = c;
#endif

    return dest;
}

void *memmove_unsafe(void *dest, const void *src, size_t n)
{
#ifdef __GNUC__
    typedef __attribute__((__may_alias__)) size_t WT;
#define WS (sizeof(WT))
#endif

    char *d = dest;
    const char *s = src;

    if (d == s)
        return d;
    if ((uintptr_t)s - (uintptr_t)d - n <= -2 * n)
        return memcpy(d, s, n);

    if (d < s)
    {
#ifdef __GNUC__
        if ((uintptr_t)s % WS == (uintptr_t)d % WS)
        {
            while ((uintptr_t)d % WS)
            {
                if (!n--)
                    return dest;
                *d++ = *s++;
            }
            for (; n >= WS; n -= WS, d += WS, s += WS)
                *(WT *)d = *(WT *)s;
        }
#endif
        for (; n; n--)
            *d++ = *s++;
    }
    else
    {
#ifdef __GNUC__
        if ((uintptr_t)s % WS == (uintptr_t)d % WS)
        {
            while ((uintptr_t)(d + n) % WS)
            {
                if (!n--)
                    return dest;
                d[n] = s[n];
            }
            while (n >= WS)
                n -= WS, *(WT *)(d + n) = *(WT *)(s + n);
        }
#endif
        while (n)
            n--, d[n] = s[n];
    }

    return dest;
}

int memcmp(const void *vl, const void *vr, size_t n)
{
    const unsigned char *l = vl, *r = vr;
    for (; n && *l == *r; n--, l++, r++)
        ;
    return n ? *l - *r : 0;
}

void backspace(char s[])
{
    int len = strlen(s);
    s[len - 1] = '\0';
}

void append(char s[], char n)
{
    int len = strlen(s);
    s[len] = n;
    s[len + 1] = '\0';
}

int strncmp(const char *s1, const char *s2, size_t n)
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

long unsigned strlen(const char s[])
{
    long unsigned i = 0;
    if (s)
        while (s[i] != '\0')
            ++i;
    return i;
}

char *strcat_unsafe(char *destination, const char *source)
{
    if ((destination == NULL) || (source == NULL))
        return NULL;
    char *start = destination;
    while (*start != '\0')
    {
        start++;
    }
    while (*source != '\0')
    {
        *start++ = *source++;
    }
    *start = '\0';
    return destination;
}

char *strcpy_unsafe(char *destination, const char *source)
{
    if (destination == NULL)
        return NULL;
    char *ptr = destination;
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
    return ptr;
}

char *strncpy(char *destination, const char *source, unsigned long num)
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

int strcmp(const char *l, const char *r)
{
    for (; *l == *r && *l; l++, r++)
        ;
    return *(unsigned char *)l - *(unsigned char *)r;
}

char *strstr(const char *haystack, const char *needle)
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

char *strchr(const char *String, int Char)
{
    while (*String != (char)Char)
    {
        if (!*String++)
            return 0;
    }
    return (char *)String;
}

char *strdup(const char *String)
{
    char *OutBuffer = kmalloc(strlen((char *)String) + 1);
    strncpy(OutBuffer, String, strlen(String) + 1);
    return OutBuffer;
}

int isalpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

long int strtol(const char *str, char **endptr, int base)
{
    const char *s;
    long acc, cutoff;
    int c;
    int neg, any, cutlim;

    s = str;
    do
    {
        c = *s++;
    } while (isspace(c));
    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else
    {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = neg ? LONG_MIN : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0)
    {
        acc = neg ? LONG_MIN : LONG_MAX;
    }
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : str);
    return (acc);
}

unsigned long int strtoul(const char *str, char **endptr, int base)
{
    const char *s;
    unsigned long acc, cutoff;
    int c;
    int neg, any, cutlim;

    s = str;
    do
    {
        c = *s++;
    } while (isspace(c));
    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else
    {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = neg ? LONG_MIN : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0)
    {
        acc = neg ? LONG_MIN : LONG_MAX;
    }
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : str);
    return (acc);
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

int isempty(char *str)
{
    if (strlen(str) == 0)
        return 1;
    while (*str != '\0')
    {
        if (!isspace(*str))
            return 0;
        str++;
    }
    return 1;
}

unsigned int isdelim(char c, char *delim)
{
    while (*delim != '\0')
    {
        if (c == *delim)
            return 1;
        delim++;
    }
    return 0;
}

int abs(int i) { return i < 0 ? -i : i; }

void swap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

char *reverse(char *Buffer, int i, int j)
{
    while (i < j)
        swap(&Buffer[i++], &Buffer[j--]);
    return Buffer;
}

char *strtok(char *src, const char *delim)
{
    static char *src1;
    if (!src)
        src = src1;

    if (!src)
        return NULL;

    while (1)
    {
        if (isdelim(*src, (char *)delim))
        {
            src++;
            continue;
        }
        if (*src == '\0')
            return NULL;

        break;
    }
    char *ret = src;
    while (1)
    {
        if (*src == '\0')
        {
            src1 = src;
            return ret;
        }
        if (isdelim(*src, (char *)delim))
        {
            *src = '\0';
            src1 = src + 1;
            return ret;
        }
        src++;
    }
    return NULL;
}

int atoi(const char *String)
{
    uint64_t Length = strlen((char *)String);
    uint64_t OutBuffer = 0;
    uint64_t Power = 1;
    for (uint64_t i = Length; i > 0; --i)
    {
        OutBuffer += (String[i - 1] - 48) * Power;
        Power *= 10;
    }
    return OutBuffer;
}

double atof(const char *String)
{
    // Originally from https://github.com/GaloisInc/minlibc/blob/master/atof.c
    /*
    Copyright (c) 2014 Galois Inc.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

      * Neither the name of Galois, Inc. nor the names of its contributors
        may be used to endorse or promote products derived from this
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    */
    double a = 0.0;
    int e = 0;
    int c;
    while ((c = *String++) != '\0' && isdigit(c))
    {
        a = a * 10.0 + (c - '0');
    }
    if (c == '.')
    {
        while ((c = *String++) != '\0' && isdigit(c))
        {
            a = a * 10.0 + (c - '0');
            e = e - 1;
        }
    }
    if (c == 'e' || c == 'E')
    {
        int sign = 1;
        int i = 0;
        c = *String++;
        if (c == '+')
            c = *String++;
        else if (c == '-')
        {
            c = *String++;
            sign = -1;
        }
        while (isdigit(c))
        {
            i = i * 10 + (c - '0');
            c = *String++;
        }
        e += i * sign;
    }
    while (e > 0)
    {
        a *= 10.0;
        e--;
    }
    while (e < 0)
    {
        a *= 0.1;
        e++;
    }
    return a;
}

char *itoa(int Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    int n = abs(Value);
    int i = 0;

    while (n)
    {
        int r = n % Base;
        if (r >= 10)
            Buffer[i++] = 65 + (r - 10);
        else
            Buffer[i++] = 48 + r;
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    if (Value < 0 && Base == 10)
        Buffer[i++] = '-';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

char *ltoa(long Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    long n = abs(Value);
    int i = 0;

    while (n)
    {
        int r = n % Base;
        if (r >= 10)
            Buffer[i++] = 65 + (r - 10);
        else
            Buffer[i++] = 48 + r;
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    if (Value < 0 && Base == 10)
        Buffer[i++] = '-';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

char *ultoa(unsigned long Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    unsigned long n = Value;
    int i = 0;

    while (n)
    {
        int r = n % Base;
        if (r >= 10)
            Buffer[i++] = 65 + (r - 10);
        else
            Buffer[i++] = 48 + r;
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

extern void __chk_fail(void) __attribute__((__noreturn__));

// #define DBG_CHK 1

__no_stack_protector void *__memcpy_chk(void *dest, const void *src, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx len:%llu slen:%llu )", dest, src, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL");
        while (1)
            ;
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL");
        while (1)
            ;
    }

    if (unlikely(len == 0))
    {
        error("len is 0");
        while (1)
            ;
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0");
        while (1)
            ;
    }

    if (unlikely(len > slen))
        __chk_fail();
    return memcpy_unsafe(dest, src, len);
}

__no_stack_protector void *__memset_chk(void *dest, int val, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx val:%#x len:%llu slen:%llu )", dest, val, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL");
        while (1)
            ;
    }

    if (unlikely(len == 0))
    {
        error("len is 0");
        while (1)
            ;
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0");
        while (1)
            ;
    }

    if (unlikely(len > slen))
        __chk_fail();
    return memset_unsafe(dest, val, len);
}

__no_stack_protector void *__memmove_chk(void *dest, const void *src, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx len:%llu slen:%llu )", dest, src, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL");
        while (1)
            ;
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL");
        while (1)
            ;
    }

    if (unlikely(len == 0))
    {
        error("len is 0");
        while (1)
            ;
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0");
        while (1)
            ;
    }

    if (unlikely(len > slen))
        __chk_fail();
    return memmove_unsafe(dest, src, len);
}

__no_stack_protector char *__strcat_chk(char *dest, const char *src, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx slen:%llu )", dest, src, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL");
        while (1)
            ;
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL");
        while (1)
            ;
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0");
        while (1)
            ;
    }

    size_t dest_len = strlen(dest);
    if (unlikely(dest_len + strlen(src) + 1 > slen))
        __chk_fail();
    return strcat_unsafe(dest, src);
}

__no_stack_protector char *__strcpy_chk(char *dest, const char *src, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx slen:%llu )", dest, src, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL");
        while (1)
            ;
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL");
        while (1)
            ;
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0");
        while (1)
            ;
    }

    size_t len = strlen(src);

    if (unlikely(len >= slen))
        __chk_fail();
    return strcpy_unsafe(dest, src);
}
