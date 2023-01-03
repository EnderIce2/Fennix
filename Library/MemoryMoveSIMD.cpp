#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>
#include <cpu.hpp>

// TODO: Implement these functions

EXTERNC void *memmove_sse(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}

EXTERNC void *memmove_sse2(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}

EXTERNC void *memmove_sse3(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}

EXTERNC void *memmove_ssse3(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}

EXTERNC void *memmove_sse4_1(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}

EXTERNC void *memmove_sse4_2(void *dest, const void *src, size_t n)
{
    memmove_unsafe(dest, src, n);
    return dest;
}
