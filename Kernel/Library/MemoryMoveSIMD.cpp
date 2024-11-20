#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>
#include <cpu.hpp>

/*
TODO: Replace these functions with even more optimized versions.
      The current versions are fast but not as fast as they could be and also we need implementation for avx, not only sse.
*/

// TODO: Implement these functions

EXTERNC void *memmove_sse(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}

EXTERNC void *memmove_sse2(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}

EXTERNC void *memmove_sse3(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}

EXTERNC void *memmove_ssse3(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}

EXTERNC void *memmove_sse4_1(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}

EXTERNC void *memmove_sse4_2(void *dest, const void *src, size_t n)
{
    return memmove_unsafe(dest, src, n);
}
