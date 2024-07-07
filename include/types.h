/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_TYPES_H__
#define __FENNIX_API_TYPES_H__

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
typedef intptr_t ssize_t;

#if defined(__LP64__)
typedef int64_t off_t;
typedef int64_t off64_t;
typedef uint32_t mode_t;
typedef uint64_t dev_t;
typedef uint64_t ino64_t;
typedef uint64_t ino_t;
typedef uint32_t nlink_t;
typedef int64_t blksize_t;
typedef int64_t blkcnt_t;
typedef int64_t blkcnt64_t;
typedef int64_t time_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int64_t clock_t;
typedef int32_t pid_t;
#elif defined(__LP32__)
typedef int32_t off_t;
typedef long long off64_t;
typedef __INT32_TYPE__ mode_t;
typedef int32_t dev_t;
typedef int32_t ino64_t;
typedef int32_t ino_t;
typedef unsigned int nlink_t;
typedef int blksize_t;
typedef int32_t blkcnt_t;
typedef int32_t blkcnt64_t;
typedef int32_t time_t;
typedef unsigned uid_t;
typedef unsigned gid_t;
typedef long clock_t;
typedef int pid_t;
#endif

#define UNUSED(x) (void)(x)

#ifdef __cplusplus
#define EXTERNC extern "C"
#define NULL 0
#else // __cplusplus
#define NULL ((void *)0)
#define bool _Bool
#define EXTERNC
#endif // __cplusplus

#define true 1
#define false 0

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MAX(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a > _b ? _a : _b;      \
	})

#define MIN(a, b)               \
	({                          \
		__typeof__(a) _a = (a); \
		__typeof__(b) _b = (b); \
		_a < _b ? _a : _b;      \
	})

#ifndef __va_list__
typedef __builtin_va_list va_list;
#endif

#define asm __asm__
#define asmv __asm__ volatile

#if __STDC_VERSION__ >= 201112L && !defined(__cplusplus)
#define static_assert _Static_assert
#endif

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#define TO_PAGES(d) (((d) + PAGE_SIZE - 1) / PAGE_SIZE)
#define FROM_PAGES(d) ((d) * PAGE_SIZE)

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

#define __sync __sync_synchronize()
#define __unreachable __builtin_unreachable()
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // !__FENNIX_API_TYPES_H__
