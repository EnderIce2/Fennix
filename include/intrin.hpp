#ifndef __FENNIX_KERNEL_SIMD_H__
#define __FENNIX_KERNEL_SIMD_H__

#include <types.h>
#include <debug.h>

#define MMX_FN_ATTR __always_inline __target("mmx")
#define SSE_FN_ATTR __always_inline __target("sse")
#define SSE2_FN_ATTR __always_inline __target("sse2")
#define SSE3_FN_ATTR __always_inline __target("sse3")
#define SSSE3_FN_ATTR __always_inline __target("ssse3")
#define SSE4_1_FN_ATTR __always_inline __target("sse4.1")
#define SSE4_2_FN_ATTR __always_inline __target("sse4.2")
#define AVX_FN_ATTR __always_inline __target("avx")
#define AVX2_FN_ATTR __always_inline __target("avx2")
#define ST_IN static inline

namespace FXSR
{
    void _fxsave(void *mem_addr)
    {
        __builtin_ia32_fxsave(mem_addr);
    }

    void _fxrstor(void *mem_addr)
    {
        __builtin_ia32_fxrstor(mem_addr);
    }

    void _fxsave64(void *mem_addr)
    {
        asmv("fxsaveq (%0)"
             :
             : "r"(mem_addr)
             : "memory");
    }

    void _fxrstor64(void *mem_addr)
    {
        asmv("fxrstorq (%0)"
             :
             : "r"(mem_addr)
             : "memory");
    }
}

namespace SMAP
{
    void _clac(void)
    {
        asmv("clac" ::
                 : "cc");
    }

    void _stac(void)
    {
        asmv("stac" ::
                 : "cc");
    }
}

namespace MMX
{
    typedef long long __m64 __attribute__((__vector_size__(8), __aligned__(8)));

    typedef long long __v1di __attribute__((__vector_size__(8)));
    typedef int __v2si __attribute__((__vector_size__(8)));
    typedef short __v4hi __attribute__((__vector_size__(8)));
    typedef char __v8qi __attribute__((__vector_size__(8)));

    ST_IN MMX_FN_ATTR void _mm_empty(void)
    {
        __builtin_ia32_emms();
    }
}

namespace SSE
{
    typedef int __v4si __attribute__((__vector_size__(16)));
    typedef unsigned int __v4su __attribute__((__vector_size__(16)));
    typedef float __v4sf __attribute__((__vector_size__(16)));

    typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
    typedef float __m128_u __attribute__((__vector_size__(16), __aligned__(1)));

    ST_IN SSE_FN_ATTR __m128 _mm_add_ss(__m128 a, __m128 b)
    {
        // return __builtin_ia32_addss(a, b);
        a[0] += b[0];
        return a;
    }

    ST_IN SSE_FN_ATTR __m128 _mm_add_ps(__m128 a, __m128 b)
    {
        return (__m128)((__v4sf)a + (__v4sf)b);
    }
}

namespace SSE2
{
    typedef double __v2df __attribute__((__vector_size__(16)));

    typedef long long __v2di __attribute__((__vector_size__(16)));
    typedef short __v8hi __attribute__((__vector_size__(16)));
    typedef char __v16qi __attribute__((__vector_size__(16)));
    typedef signed char __v16qs __attribute__((__vector_size__(16)));

    typedef unsigned long long __v2du __attribute__((__vector_size__(16)));
    typedef unsigned short __v8hu __attribute__((__vector_size__(16)));
    typedef unsigned char __v16qu __attribute__((__vector_size__(16)));

    typedef double __m128d __attribute__((__vector_size__(16), __aligned__(16)));
    typedef double __m128d_u __attribute__((__vector_size__(16), __aligned__(1)));
    typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));
    typedef long long __m128i_u __attribute__((__vector_size__(16), __aligned__(1)));

    ST_IN SSE2_FN_ATTR __m128i _mm_mul_epu32(__m128i a, __m128i b)
    {
        __m128i result;
        __asm__("pmuludq %1, %2 \n\t"
                "movdqu %2, %0 \n\t"
                : "=x"(result)
                : "x"(a), "x"(b));
        return result;
    }

    ST_IN SSE2_FN_ATTR __m128i _mm_set_epi32(int e3, int e2, int e1, int e0)
    {
        __m128i result;
        __asm__("movd %[e0], %[result]\n\t"
                "pinsrd $1, %[e1], %[result]\n\t"
                "pinsrd $2, %[e2], %[result]\n\t"
                "pinsrd $3, %[e3], %[result]\n\t"
                : [result] "=x"(result)
                : [e0] "r"(e0), [e1] "r"(e1), [e2] "r"(e2), [e3] "r"(e3));
        return result;
    }

    ST_IN SSE2_FN_ATTR __m128i _mm_set1_epi32(int a)
    {
        __m128i result;
        __asm__("movd %1, %%xmm0\n\t"
                "pshufd $0, %%xmm0, %0\n\t"
                : "=x"(result)
                : "r"(a)
                : "%xmm0");
        return result;
    }

    ST_IN SSE2_FN_ATTR void _mm_storeu_si128(__m128i *mem_addr, __m128i a)
    {
        asm volatile("movdqu %1, %0"
                     : "=m"(*mem_addr)
                     : "x"(a));
    }
}

namespace SSE3
{

}

namespace SSSE3
{

}

namespace SSE4_1
{
    typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));

    ST_IN SSE4_1_FN_ATTR __m128i _mm_cvtepu8_epi32(__m128i a);
    ST_IN SSE4_1_FN_ATTR __m128i _mm_mullo_epi32(__m128i a, __m128i b);
    ST_IN SSE4_1_FN_ATTR __m128i _mm_srli_epi32(__m128i a, int imm8);
    ST_IN SSE4_1_FN_ATTR int _mm_cvtsi128_si32(__m128i a);
}

namespace SSE4_2
{

}

namespace AVX
{

}

namespace AVX2
{

}

#endif // !__FENNIX_KERNEL_SIMD_H__
