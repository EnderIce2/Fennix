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

#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>
#include <cpu.hpp>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "../kernel.h"

EXTERNC int memcmp(const void *vl, const void *vr, size_t n)
{
    const unsigned char *l = (unsigned char *)vl, *r = (unsigned char *)vr;
    for (; n && *l == *r; n--, l++, r++)
        ;
    return n ? *l - *r : 0;
}

EXTERNC void backspace(char s[])
{
    int len = s_cst(int, strlen(s));
    s[len - 1] = '\0';
}

EXTERNC void append(char s[], char n)
{
    int len = s_cst(int, strlen(s));
    s[len] = n;
    s[len + 1] = '\0';
}

EXTERNC int strncmp(const char *s1, const char *s2, size_t n)
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

EXTERNC long unsigned strlen(const char s[])
{
    long unsigned i = 0;
    if (s)
        while (s[i] != '\0')
            ++i;
    return i;
}

EXTERNC char *strcat_unsafe(char *destination, const char *source)
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

EXTERNC char *strcpy_unsafe(char *destination, const char *source)
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

EXTERNC char *strncpy(char *destination, const char *source, unsigned long num)
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

EXTERNC int strcmp(const char *l, const char *r)
{
    for (; *l == *r && *l; l++, r++)
        ;
    return *(unsigned char *)l - *(unsigned char *)r;
}

EXTERNC char *strstr(const char *haystack, const char *needle)
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

EXTERNC char *strchr(const char *String, int Char)
{
    while (*String != (char)Char)
    {
        if (!*String++)
            return 0;
    }
    return (char *)String;
}

EXTERNC char *strdup(const char *String)
{
    char *OutBuffer = (char *)kmalloc(strlen((char *)String) + 1);
    strncpy(OutBuffer, String, strlen(String) + 1);
    return OutBuffer;
}

EXTERNC int isalpha(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

EXTERNC int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}

EXTERNC long int strtol(const char *str, char **endptr, int base)
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
    cutlim = s_cst(int, cutoff % base);
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

EXTERNC unsigned long int strtoul(const char *str, char **endptr, int base)
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
    cutlim = s_cst(int, cutoff % base);
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

EXTERNC int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

EXTERNC int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

EXTERNC int isempty(char *str)
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

EXTERNC unsigned int isdelim(char c, const char *delim)
{
    while (*delim != '\0')
    {
        if (c == *delim)
            return 1;
        delim++;
    }
    return 0;
}

EXTERNC long abs(long i) { return i < 0 ? -i : i; }

EXTERNC void swap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

EXTERNC char *reverse(char *Buffer, int i, int j)
{
    while (i < j)
        swap(&Buffer[i++], &Buffer[j--]);
    return Buffer;
}

EXTERNC float sqrtf(float x)
{
    if (x < 0.0f)
        return NAN;

    if (x < 1e-7f)
        return 0.0f;

    float guess = x / 2.0f;
    for (short i = 0; i < 10; i++)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        if (guess == 0.0f)
            return 0.0f;
#pragma GCC diagnostic pop

        guess = (guess + x / guess) / 2.0f;
    }
    return guess;
}

EXTERNC double clamp(double x, double low, double high)
{
    if (x < low)
        return low;
    else if (x > high)
        return high;
    else
        return x;
}

EXTERNC float lerp(float a, float b, float t)
{
    return (1 - t) * a + t * b;
}

EXTERNC float smoothstep(float a, float b, float t)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
    t = clamp(s_cst(double, t), 0.0, 1.0);
#pragma GCC diagnostic pop
    return lerp(a, b, t * t * (3 - 2 * t));
}

EXTERNC float cubicInterpolate(float a, float b, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return a + (-2 * t3 + 3 * t2) * b;
}

EXTERNC char *strtok(char *src, const char *delim)
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

int strcasecmp(const char *s1, const char *s2)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    int result;
    if (p1 == p2)
        return 0;
    while ((result = std::tolower(*p1) - std::tolower(*p2++)) == 0)
        if (*p1++ == '\0')
            break;
    return result;
}

size_t wcslen(const wchar_t *s)
{
    size_t len = 0;

    while (s[len] != L'\0')
    {
        if (s[++len] == L'\0')
            return len;
        if (s[++len] == L'\0')
            return len;
        if (s[++len] == L'\0')
            return len;
        ++len;
    }

    return len;
}

size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps)
{
    size_t count = 0;

    while (len > 0)
    {
        wchar_t wc = **src;
        if (wc == L'\0')
            break;

        if (wc < 0x80)
        {
            if (dst)
                *dst++ = (char)wc;
            count++;
            len--;
        }
        else if (wc < 0x800)
        {
            if (dst)
            {
                *dst++ = (char)(0xC0 | (wc >> 6));
                *dst++ = (char)(0x80 | (wc & 0x3F));
            }
            count += 2;
            len -= 2;
        }
        else if (wc < 0x10000)
        {
            if (dst)
            {
                *dst++ = (char)(0xE0 | (wc >> 12));
                *dst++ = (char)(0x80 | ((wc >> 6) & 0x3F));
                *dst++ = (char)(0x80 | (wc & 0x3F));
            }
            count += 3;
            len -= 3;
        }
        else
        {
            if (dst)
            {
                *dst++ = (char)(0xF0 | (wc >> 18));
                *dst++ = (char)(0x80 | ((wc >> 12) & 0x3F));
                *dst++ = (char)(0x80 | ((wc >> 6) & 0x3F));
                *dst++ = (char)(0x80 | (wc & 0x3F));
            }
            count += 4;
            len -= 4;
        }

        (*src)++;
    }

    if (dst)
        *dst = '\0';

    return count;
}

EXTERNC int atoi(const char *String)
{
    uint64_t Length = strlen((char *)String);
    int OutBuffer = 0;
    int Power = 1;
    for (uint64_t i = Length; i > 0; --i)
    {
        OutBuffer += (String[i - 1] - 48) * Power;
        Power *= 10;
    }
    return OutBuffer;
}

EXTERNC double atof(const char *String)
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

EXTERNC char *itoa(int Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    int n = s_cst(int, abs(Value));
    int i = 0;

    while (n)
    {
        int r = s_cst(int, n % Base);
        if (r >= 10)
            Buffer[i++] = s_cst(char, 65 + (r - 10));
        else
            Buffer[i++] = s_cst(char, 48 + r);
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    if (Value < 0 && Base == 10)
        Buffer[i++] = '-';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

EXTERNC char *ltoa(long Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    long n = abs(Value);
    int i = 0;

    while (n)
    {
        int r = s_cst(int, n % Base);
        if (r >= 10)
            Buffer[i++] = s_cst(char, 65 + (r - 10));
        else
            Buffer[i++] = s_cst(char, 48 + r);
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    if (Value < 0 && Base == 10)
        Buffer[i++] = '-';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

EXTERNC char *ultoa(unsigned long Value, char *Buffer, int Base)
{
    if (Base < 2 || Base > 32)
        return Buffer;

    unsigned long n = Value;
    int i = 0;

    while (n)
    {
        int r = s_cst(int, n % Base);
        if (r >= 10)
            Buffer[i++] = s_cst(char, 65 + (r - 10));
        else
            Buffer[i++] = s_cst(char, 48 + r);
        n = n / Base;
    }

    if (i == 0)
        Buffer[i++] = '0';

    Buffer[i] = '\0';
    return reverse(Buffer, 0, i - 1);
}

EXTERNC int log2(unsigned int n)
{
    if (n == 0)
    {
        error("log2(0) is undefined.");
        return -1;
    }

    int log = 0;

    if ((n & 0xFFFF0000) != 0)
    {
        n >>= 16;
        log = 16;
    }

    if (n >= 256)
    {
        n >>= 8;
        log += 8;
    }

    if (n >= 16)
    {
        n >>= 4;
        log += 4;
    }

    if (n >= 4)
    {
        n >>= 2;
        log += 2;
    }

    return log + (n >> 1);
}

EXTERNC void __chk_fail(void) __attribute__((__noreturn__));

__noreturn __always_inline static inline void __convert_chk_fail(void)
{
#if defined(a64) || defined(a32)
    asmv("int3");
#else
#warning "Not implemented!"
#endif
    __builtin_unreachable();
}

// #define DBG_CHK 1

EXTERNC __no_stack_protector void *__memcpy_chk(void *dest, const void *src, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx len:%llu slen:%llu )", dest, src, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len == 0))
    {
        error("len is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len > slen))
        __chk_fail();

    void *ret = nullptr;
    uint64_t simd = CPU::CheckSIMD();
    if (simd & CPU::x86SIMDType::SIMD_SSE42)
        ret = memcpy_sse4_2(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE41)
        ret = memcpy_sse4_1(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSSE3)
        ret = memcpy_ssse3(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE3)
        ret = memcpy_sse3(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE2)
        ret = memcpy_sse2(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE)
        ret = memcpy_sse(dest, src, len);
    else
        ret = memcpy_unsafe(dest, src, len);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "memcpy( %p %p %ld %ld )=%p~%p\n\r",
                dest, src, len, slen,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

EXTERNC __no_stack_protector void *__memset_chk(void *dest, int val, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx val:%#x len:%llu slen:%llu )", dest, val, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len == 0))
    {
        error("len is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len > slen))
        __chk_fail();

    void *ret = nullptr;
    uint64_t simd = CPU::CheckSIMD();
    if (simd & CPU::x86SIMDType::SIMD_SSE42)
        ret = memset_sse4_2(dest, val, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE41)
        ret = memset_sse4_1(dest, val, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSSE3)
        ret = memset_ssse3(dest, val, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE3)
        ret = memset_sse3(dest, val, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE2)
        ret = memset_sse2(dest, val, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE)
        ret = memset_sse(dest, val, len);
    else
        ret = memset_unsafe(dest, val, len);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "memset( %p %d %ld %ld )=%p~%p\n\r",
                dest, val, len, slen,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

EXTERNC __no_stack_protector void *__memmove_chk(void *dest, const void *src, size_t len, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx len:%llu slen:%llu )", dest, src, len, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len == 0))
    {
        error("len is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(len > slen))
        __chk_fail();

    void *ret = nullptr;
    uint64_t simd = CPU::CheckSIMD();
    if (simd & CPU::x86SIMDType::SIMD_SSE42)
        ret = memmove_sse4_2(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE41)
        ret = memmove_sse4_1(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSSE3)
        ret = memmove_ssse3(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE3)
        ret = memmove_sse3(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE2)
        ret = memmove_sse2(dest, src, len);
    else if (simd & CPU::x86SIMDType::SIMD_SSE)
        ret = memmove_sse(dest, src, len);
    else
        ret = memmove_unsafe(dest, src, len);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "memmove( %p %p %ld %ld )=%p~%p\n\r",
                dest, src, len, slen,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

EXTERNC __no_stack_protector char *__strcat_chk(char *dest, const char *src, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx slen:%llu )", dest, src, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    size_t dest_len = strlen(dest);
    if (unlikely(dest_len + strlen(src) + 1 > slen))
        __chk_fail();
    return strcat_unsafe(dest, src);
}

EXTERNC __no_stack_protector char *__strcpy_chk(char *dest, const char *src, size_t slen)
{
#ifdef DBG_CHK
    debug("( dest:%#lx src:%#lx slen:%llu )", dest, src, slen);
#endif
    if (unlikely(dest == NULL))
    {
        error("dest is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(src == NULL))
    {
        error("src is NULL (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    if (unlikely(slen == 0))
    {
        error("slen is 0 (for %#lx %s)", __builtin_return_address(0), KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
        __convert_chk_fail();
    }

    size_t len = strlen(src);

    if (unlikely(len >= slen))
        __chk_fail();

    return strcpy_unsafe(dest, src);
}

#undef memcpy
EXTERNC __no_stack_protector void *memcpy(void *dest, const void *src, size_t len)
{
    void *ret = __memcpy_chk(dest, src, len, __builtin_object_size(dest, 0));
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "!memcpy( %p %p %ld )=%p~%p\n\r",
                dest, src, len,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

#undef memset
EXTERNC __no_stack_protector void *memset(void *dest, int val, size_t len)
{
    void *ret = __memset_chk(dest, val, len, __builtin_object_size(dest, 0));
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "!memset( %p %d %ld )=%p~%p\n\r",
                dest, val, len,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}
