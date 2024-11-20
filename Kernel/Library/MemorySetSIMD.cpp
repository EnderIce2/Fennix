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

/*
TODO: Replace these functions with even more optimized versions.
      The current versions are fast but not as fast as they could be and also we need implementation for avx, not only sse.
*/

// TODO: Implement these functions

EXTERNC void *memset_sse(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
}

EXTERNC void *memset_sse2(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
}

EXTERNC void *memset_sse3(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
}

EXTERNC void *memset_ssse3(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
}

EXTERNC void *memset_sse4_1(void *dest, int c, size_t n)
{
    return memset_unsafe(dest, c, n);
}

EXTERNC void *memset_sse4_2(void *dest, int c, size_t n)
{
#if defined(a64)
    char *d = (char *)dest;

    if (((uintptr_t)d & 0xF) == 0)
    {
        size_t num_vectors = n / 16;
        for (size_t i = 0; i < num_vectors; i++)
        {
            asmv("movd %0, %%xmm0\n"
                 "pshufd $0, %%xmm0, %%xmm0\n"
                 "movdqa %%xmm0, (%1)\n"
                 :
                 : "r"(c), "r"(d)
                 : "xmm0");
            d += 16;
        }
        n -= num_vectors * 16;
    }

    memset_unsafe(d, c, n);
#endif
    return dest;
}
