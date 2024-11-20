#ifndef __FENNIX_LIBC_TYPES_H__
#define __FENNIX_LIBC_TYPES_H__

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#define START_EXTERNC \
	EXTERNC           \
	{
#define END_EXTERNC \
	}
#else
#define EXTERNC
#define START_EXTERNC
#define END_EXTERNC
#endif

#define asm __asm__
#define asmv __asm__ volatile

#define true 1
#define false 0

#ifdef __cplusplus
#define foreach for
#define in :
#endif

#define UNUSED(x) (void)(x)
#define CONCAT(x, y) x##y

#define toupper(c) ((c)-0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))

#define ALIGN_UP(x, align) ((__typeof__(x))(((uint64_t)(x) + ((align)-1)) & (~((align)-1))))
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

#define b4(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
#define b8(x) ((x)&0xFF)
#define b16(x) __builtin_bswap16(x)
#define b32(x) __builtin_bswap32(x)
#define b48(x) (((((x)&0x0000000000ff) << 40) | (((x)&0x00000000ff00) << 24) | (((x)&0x000000ff0000) << 8) | (((x)&0x0000ff000000) >> 8) | (((x)&0x00ff00000000) >> 24) | (((x)&0xff0000000000) >> 40)))
#define b64(x) __builtin_bswap64(x)

#ifndef PUBLIC
#define PUBLIC __attribute__((visibility("default")))
#endif // !PUBLIC

#ifndef PRIVATE
#define PRIVATE __attribute__((visibility("hidden")))
#endif // !PRIVATE

#endif // !__FENNIX_LIBC_TYPES_H__
