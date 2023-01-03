#include <convert.h>

#include <memory.hpp>
#include <limits.h>
#include <debug.h>
#include <cpu.hpp>

// TODO: Implement these functions properly

EXTERNC void *memset_sse(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movaps (%0), %%xmm0\n"
                 "movaps %%xmm0, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}

EXTERNC void *memset_sse2(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movdqa (%0), %%xmm0\n"
                 "movdqa %%xmm0, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}

EXTERNC void *memset_sse3(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0x7) == 0)
    {
        size_t num_vectors = n / 8;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movq (%0), %%xmm0\n"
                 "movddup %%xmm0, %%xmm1\n"
                 "movq %%xmm1, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0", "xmm1");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}

EXTERNC void *memset_ssse3(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movdqa (%0), %%xmm0\n"
                 "movdqa 16(%0), %%xmm1\n"
                 "palignr $8, %%xmm0, %%xmm1\n"
                 "movdqa %%xmm1, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0", "xmm1");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}

EXTERNC void *memset_sse4_1(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movdqa (%0), %%xmm0\n"
                 "movdqa %%xmm0, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}

EXTERNC void *memset_sse4_2(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movdqa (%0), %%xmm0\n"
                 "pcmpistri $0, (%0), %%xmm0\n"
                 "movdqa %%xmm0, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
    return dest;
}
