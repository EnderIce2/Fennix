#ifndef __FENNIX_KERNEL_TYPES_H__
#define __FENNIX_KERNEL_TYPES_H__

#ifdef __cplusplus
#define EXTERNC extern "C"
#define START_EXTERNC \
    EXTERNC           \
    {
#define END_EXTERNC \
    }
#else // __cplusplus
#define EXTERNC
#define START_EXTERNC
#define END_EXTERNC
#endif // __cplusplus

#ifdef __cplusplus
#define NULL 0
#else // __cplusplus
#define NULL ((void *)0)
#define bool _Bool
#endif // __cplusplus

#define asm __asm__
#define asmv __asm__ volatile

#define true 1
#define false 0

#define inf_loop while (1)
#define ilp inf_loop; /* Used for debugging */

#ifdef __cplusplus
#define foreach for
#define in :

#define r_cst(t, v) reinterpret_cast<t>(v)
#define c_cst(t, v) const_cast<t>(v)
#define s_cst(t, v) static_cast<t>(v)
#define d_cst(t, v) dynamic_cast<t>(v)
#endif // __cplusplus

#define UNUSED(x) (void)(x)
#define CONCAT(x, y) x##y

#ifndef __cplusplus /* This conflicts with std */
#define toupper(c) ((c)-0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#endif

#ifndef __va_list__
typedef __builtin_va_list va_list;
#endif

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#define ALIGN_UP(x, align) ((__typeof__(x))(((uintptr_t)(x) + ((align)-1)) & (~((align)-1))))
#define ALIGN_DOWN(x, align) ((__typeof__(x))((x) & (~((align)-1))))

#define offsetof(type, member) __builtin_offsetof(type, member)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define VPOKE(type, address) (*((volatile type *)(address)))
#define POKE(type, address) (*((type *)(address)))

#ifndef __cplusplus

#ifdef __STDC__
#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ >= 201710L)
#define C_LANGUAGE_STANDARD 2018
#elif (__STDC_VERSION__ >= 201112L)
#define C_LANGUAGE_STANDARD 2011
#elif (__STDC_VERSION__ >= 199901L)
#define C_LANGUAGE_STANDARD 1999
#elif (__STDC_VERSION__ >= 199409L)
#define C_LANGUAGE_STANDARD 1995
#endif
#else
#define C_LANGUAGE_STANDARD 1990
#endif
#else
#define C_LANGUAGE_STANDARD 1972
#endif

#else

#ifdef __STDC__
#ifdef __cplusplus
#if (__cplusplus >= 202100L)
#define CPP_LANGUAGE_STANDARD 2023
#elif (__cplusplus >= 202002L)
#define CPP_LANGUAGE_STANDARD 2020
#elif (__cplusplus >= 201703L)
#define CPP_LANGUAGE_STANDARD 2017
#elif (__cplusplus >= 201402L)
#define CPP_LANGUAGE_STANDARD 2014
#elif (__cplusplus >= 201103L)
#define CPP_LANGUAGE_STANDARD 2011
#elif (__cplusplus >= 199711L)
#define CPP_LANGUAGE_STANDARD 1998
#endif
#else
#define CPP_LANGUAGE_STANDARD __cplusplus
#endif
#else
#define CPP_LANGUAGE_STANDARD __cplusplus
#endif

#endif // __cplusplus

#ifndef __SIG_ATOMIC_TYPE__
#define __SIG_ATOMIC_TYPE__ int
#endif

typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

typedef __INT_LEAST8_TYPE__ int_least8_t;
typedef __INT_LEAST16_TYPE__ int_least16_t;
typedef __INT_LEAST32_TYPE__ int_least32_t;
typedef __INT_LEAST64_TYPE__ int_least64_t;

typedef __UINT_LEAST8_TYPE__ uint_least8_t;
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
typedef __UINT_LEAST64_TYPE__ uint_least64_t;

typedef __INT_FAST8_TYPE__ int_fast8_t;
typedef __INT_FAST16_TYPE__ int_fast16_t;
typedef __INT_FAST32_TYPE__ int_fast32_t;
typedef __INT_FAST64_TYPE__ int_fast64_t;

typedef __UINT_FAST8_TYPE__ uint_fast8_t;
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
typedef __UINT_FAST64_TYPE__ uint_fast64_t;

typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

typedef __INTMAX_TYPE__ intmax_t;
typedef __UINTMAX_TYPE__ uintmax_t;

typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
#ifndef __cplusplus
typedef __WCHAR_TYPE__ wchar_t;
#endif
typedef __WINT_TYPE__ wint_t;
typedef __SIG_ATOMIC_TYPE__ sig_atomic_t;
// TODO: ssize_t
typedef intptr_t ssize_t;

#define INT8_MAX __INT8_MAX__
#define INT8_MIN (-INT8_MAX - 1)
#define UINT8_MAX __UINT8_MAX__
#define INT16_MAX __INT16_MAX__
#define INT16_MIN (-INT16_MAX - 1)
#define UINT16_MAX __UINT16_MAX__
#define INT32_MAX __INT32_MAX__
#define INT32_MIN (-INT32_MAX - 1)
#define UINT32_MAX __UINT32_MAX__
#define INT64_MAX __INT64_MAX__
#define INT64_MIN (-INT64_MAX - 1)
#define UINT64_MAX __UINT64_MAX__

#define INT_LEAST8_MAX __INT_LEAST8_MAX__
#define INT_LEAST8_MIN (-INT_LEAST8_MAX - 1)
#define UINT_LEAST8_MAX __UINT_LEAST8_MAX__
#define INT_LEAST16_MAX __INT_LEAST16_MAX__
#define INT_LEAST16_MIN (-INT_LEAST16_MAX - 1)
#define UINT_LEAST16_MAX __UINT_LEAST16_MAX__
#define INT_LEAST32_MAX __INT_LEAST32_MAX__
#define INT_LEAST32_MIN (-INT_LEAST32_MAX - 1)
#define UINT_LEAST32_MAX __UINT_LEAST32_MAX__
#define INT_LEAST64_MAX __INT_LEAST64_MAX__
#define INT_LEAST64_MIN (-INT_LEAST64_MAX - 1)
#define UINT_LEAST64_MAX __UINT_LEAST64_MAX__

#define INT_FAST8_MAX __INT_FAST8_MAX__
#define INT_FAST8_MIN (-INT_FAST8_MAX - 1)
#define UINT_FAST8_MAX __UINT_FAST8_MAX__
#define INT_FAST16_MAX __INT_FAST16_MAX__
#define INT_FAST16_MIN (-INT_FAST16_MAX - 1)
#define UINT_FAST16_MAX __UINT_FAST16_MAX__
#define INT_FAST32_MAX __INT_FAST32_MAX__
#define INT_FAST32_MIN (-INT_FAST32_MAX - 1)
#define UINT_FAST32_MAX __UINT_FAST32_MAX__
#define INT_FAST64_MAX __INT_FAST64_MAX__
#define INT_FAST64_MIN (-INT_FAST64_MAX - 1)
#define UINT_FAST64_MAX __UINT_FAST64_MAX__

#define INTPTR_MAX __INTPTR_MAX__
#define INTPTR_MIN (-INTPTR_MAX - 1)
#define UINTPTR_MAX __UINTPTR_MAX__

#define INTMAX_MAX __INTMAX_MAX__
#define INTMAX_MIN (-INTMAX_MAX - 1)
#define UINTMAX_MAX __UINTMAX_MAX__

#define PTRDIFF_MAX __PTRDIFF_MAX__
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)

#define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__
#define SIG_ATOMIC_MIN __SIG_ATOMIC_MIN__

#define SIZE_MAX __SIZE_MAX__

#define WCHAR_MAX __WCHAR_MAX__
#define WCHAR_MIN __WCHAR_MIN__

#define WINT_MAX __WINT_MAX__
#define WINT_MIN __WINT_MIN__

#define BREAK __asm__ __volatile__("int $0x3" \
                                   :          \
                                   :          \
                                   : "memory");

#ifdef __INT48_TYPE__
typedef __INT48_TYPE__ int48_t;
typedef __UINT48_TYPE__ uint48_t;
typedef int48_t int_least48_t;
typedef uint48_t uint_least48_t;
typedef int48_t int_fast48_t;
typedef uint48_t uint_fast48_t;
#else  // __INT48_TYPE__
typedef __INT64_TYPE__ int48_t;
typedef __UINT64_TYPE__ uint48_t;
typedef int48_t int_least48_t;
typedef uint48_t uint_least48_t;
typedef int48_t int_fast48_t;
typedef uint48_t uint_fast48_t;
#endif // __INT48_TYPE__

#define b4(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
#define b8(x) ((x)&0xFF)
#define b16(x) __builtin_bswap16(x)
#define b32(x) __builtin_bswap32(x)
#define b48(x) (((((x)&0x0000000000ff) << 40) | \
                 (((x)&0x00000000ff00) << 24) | \
                 (((x)&0x000000ff0000) << 8) |  \
                 (((x)&0x0000ff000000) >> 8) |  \
                 (((x)&0x00ff00000000) >> 24) | \
                 (((x)&0xff0000000000) >> 40)))
#define b64(x) __builtin_bswap64(x)

/* https://gcc.gnu.org/onlinedocs/gcc-9.5.0/gnat_ugn/Optimization-Levels.html */

/** @brief No optimization (the default); generates unoptimized code but has the fastest compilation time. */
#define O0 __attribute__((optimize("O0")))
/** @brief Moderate optimization; optimizes reasonably well but does not degrade compilation time significantly. */
#define O1 __attribute__((optimize("O1")))
/** @brief Full optimization; generates highly optimized code and has the slowest compilation time. */
#define O2 __attribute__((optimize("O2")))
/** @brief Full optimization as in -O2; also uses more aggressive automatic inlining of subprograms within a unit (Inlining of Subprograms) and attempts to vectorize loops. */
#define O3 __attribute__((optimize("O3")))
/** @brief Optimize space usage (code and data) of resulting program. */
#define Os __attribute__((optimize("Os")))
/** @brief Disregard strict standards compliance. -Ofast enables all -O3 optimizations. It also enables optimizations that are not valid for all standard-compliant programs. */
#define Ofast __attribute__((optimize("Ofast")))

#define __unused __attribute__((unused))
#define __packed __attribute__((packed))
#define __naked __attribute__((naked))
#define __aligned(x) __attribute__((aligned(x)))
#define __section(x) __attribute__((section(x)))
#define __noreturn __attribute__((noreturn))
#define __weak __attribute__((weak))
#define __alias(x) __attribute__((alias(x)))
#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __pure __attribute__((pure))
#define __const __attribute__((const))
#define __malloc __attribute__((malloc))
#define __returns_twice __attribute__((returns_twice))
#define __used __attribute__((used))
#define __deprecated __attribute__((deprecated))
#define __deprecated_msg(x) __attribute__((deprecated(x)))
#define __weakref(x) __attribute__((weakref(x)))
#define __weakrefalias(x) __attribute__((weakref(#x)))
#define __visibility(x) __attribute__((visibility(x)))
#define __constructor __attribute__((constructor))
#define __destructor __attribute__((destructor))
#define __cleanup(x) __attribute__((cleanup(x)))
#define __fallthrough __attribute__((fallthrough))
#define __nonnull(x) __attribute__((nonnull x))
#define __nonnull_all __attribute__((nonnull))
#define __returns_nonnull __attribute__((returns_nonnull))
#define __sentinel __attribute__((sentinel))
#define __sentinel_all __attribute__((sentinel(0)))
#define __format(x, y, z) __attribute__((format(x, y, z)))
#define __format_arg(x) __attribute__((format_arg(x)))
#define __nonnull_params(x) __attribute__((nonnull x))
#define __nonnull_all __attribute__((nonnull))
#define __warn_unused_result __attribute__((warn_unused_result))
#define __no_stack_protector __attribute__((no_stack_protector))
#define __no_instrument_function __attribute__((no_instrument_function))
#define __no_debug __attribute__((no_debug))
#define __target(x) __attribute__((target(x)))
#define __min_vector_width(x) __attribute__((min_vector_width(x)))

// sanitizer
#define __no_sanitize_address __attribute__((no_sanitize_address))
#define __no_sanitize_undefined __attribute__((no_sanitize_undefined))
#define __no_address_safety_analysis __attribute__((no_address_safety_analysis))
#define __no_sanitize_thread __attribute__((no_sanitize_thread))

#define __synchronize __sync_synchronize()
#define __sync __synchronize

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SafeFunction __no_stack_protector __no_sanitize_address __no_sanitize_undefined __no_address_safety_analysis __no_sanitize_thread

#define NIF __no_instrument_function

#define int1                        \
    __asm__ __volatile__("int $0x1" \
                         :          \
                         :          \
                         : "memory")

#define int3                    \
    __asm__ __volatile__("int3" \
                         :      \
                         :      \
                         : "memory")

#endif // !__FENNIX_KERNEL_TYPES_H__
