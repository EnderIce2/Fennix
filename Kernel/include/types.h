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

#ifndef __FENNIX_KERNEL_TYPES_H__
#define __FENNIX_KERNEL_TYPES_H__

#include <stdbool.h>

/**
 * It doesn't do anything.
 *
 * Used to specify a function that is dependent on the architecture.
 * It's architecture specific variant is defined in arch/<arch>/...
 */
#define arch

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
#endif // __cplusplus

#define asm __asm__
#define asmv __asm__ volatile

#define inf_loop while (1)
#define ilp inf_loop; /* Used for debugging */

#ifdef __cplusplus
#define forItr(itr, container)         \
	for (auto itr = container.begin(); \
		 itr != container.end(); ++itr)

#define r_cst(t, v) reinterpret_cast<t>(v)
#define c_cst(t, v) const_cast<t>(v)
#define s_cst(t, v) static_cast<t>(v)
#define d_cst(t, v) dynamic_cast<t>(v)
#endif // __cplusplus

#define UNUSED(x) (void)(x)
#define CONCAT(x, y) x##y

#ifndef __va_list__
typedef __builtin_va_list va_list;
#endif

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#define ALIGN_UP(x, align) ((__typeof__(x))(((uintptr_t)(x) + ((align) - 1)) & (~((align) - 1))))
#define ALIGN_DOWN(x, align) ((__typeof__(x))((x) & (~((align) - 1))))

#define offsetof(type, member) __builtin_offsetof(type, member)

#define RGB_TO_HEX(r, g, b) ((r << 16) | (g << 8) | (b))

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

#define ROUND_UP(x, y) (((x) + (y) - 1) & ~((y) - 1))
#define ROUND_DOWN(x, y) ((x) & ~((y) - 1))

#define VPOKE(type, address) (*((volatile type *)(address)))
#define POKE(type, address) (*((type *)(address)))

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

#if defined(__amd64__) || defined(__aarch64__)
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
#elif defined(__i386__) || defined(__arm__)
typedef int32_t off_t;
typedef long long off64_t;
typedef uint32_t mode_t;
typedef int32_t dev_t;
typedef int32_t ino64_t;
typedef int32_t ino_t;
typedef uint32_t nlink_t;
typedef int blksize_t;
typedef int32_t blkcnt_t;
typedef int32_t blkcnt64_t;
typedef int32_t time_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef long clock_t;
typedef int pid_t;
#endif

#ifdef __cplusplus
template <typename T>
class ptr_t
{
	T ptr;

public:
	ptr_t() : ptr(nullptr) {}
	ptr_t(T p) : ptr(p) {}
	ptr_t(int p) : ptr((T)(uintptr_t)p) {}
	ptr_t(const ptr_t<T> &other) : ptr(other.ptr) {}

	operator T() { return ptr; }
	operator uintptr_t() { return (uintptr_t)ptr; }

	operator bool() { return (void *)(uintptr_t)ptr != nullptr; }

	ptr_t<T> &operator=(const ptr_t<T> &other)
	{
		ptr = other.ptr;
		return *this;
	}

	ptr_t<T> &operator+=(uintptr_t offset)
	{
		ptr = (T)((uintptr_t)ptr + offset);
		return *this;
	}

	ptr_t<T> &operator-=(uintptr_t offset)
	{
		ptr = (T)((uintptr_t)ptr - offset);
		return *this;
	}

	bool operator==(const ptr_t<T> &other) const { return ptr == other.ptr; }
	bool operator==(auto other) const { return (uintptr_t)ptr == (uintptr_t)other; }

	bool operator!=(const ptr_t<T> &other) const { return ptr != other.ptr; }
	bool operator!=(auto other) const { return (uintptr_t)ptr != (uintptr_t)other; }

	bool operator>(const ptr_t<T> &other) const { return ptr > other.ptr; }
	bool operator>(auto other) const { return (uintptr_t)ptr > (uintptr_t)other; }

	bool operator<(const ptr_t<T> &other) const { return ptr < other.ptr; }
	bool operator<(auto other) const { return (uintptr_t)ptr < (uintptr_t)other; }

	bool operator>=(const ptr_t<T> &other) const { return ptr >= other.ptr; }
	bool operator>=(auto other) const { return (uintptr_t)ptr >= (uintptr_t)other; }

	bool operator<=(const ptr_t<T> &other) const { return ptr <= other.ptr; }
	bool operator<=(auto other) const { return (uintptr_t)ptr <= (uintptr_t)other; }

	ptr_t<T> operator+(auto offset) const { return ptr_t<T>((void *)((uintptr_t)ptr + offset)); }
	ptr_t<T> operator-(auto offset) const { return ptr_t<T>((void *)((uintptr_t)ptr - offset)); }

	T operator->() { return ptr; }
	T operator*() { return *ptr; }
};
#endif // __cplusplus

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

#if defined(__amd64__)
#define BREAK __asm__ __volatile__("int $0x3" \
								   :          \
								   :          \
								   : "memory");
#elif defined(__i386__)
#define BREAK __asm__ __volatile__("int $0x3" \
								   :          \
								   :          \
								   : "memory");
#elif defined(__aarch64__)
#define BREAK __asm__ __volatile__("brk #0" \
								   :        \
								   :        \
								   : "memory");
#endif

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
#define b8(x) ((x) & 0xFF)
#define b16(x) __builtin_bswap16(x)
#define b32(x) __builtin_bswap32(x)
#define b48(x) (((((x) & 0x0000000000ff) << 40) | \
				 (((x) & 0x00000000ff00) << 24) | \
				 (((x) & 0x000000ff0000) << 8) |  \
				 (((x) & 0x0000ff000000) >> 8) |  \
				 (((x) & 0x00ff00000000) >> 24) | \
				 (((x) & 0xff0000000000) >> 40)))
#define b64(x) __builtin_bswap64(x)

/* https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html */

/** No optimization (the default); generates
 *  unoptimized code but has the fastest compilation time.
 */
#define O0 __attribute__((optimize("O0")))

/** Moderate optimization;
 *  optimizes reasonably well but does not degrade
 *  compilation time significantly. */
#define O1 __attribute__((optimize("O1")))

/** Full optimization; generates highly
 *  optimized code and has the slowest compilation time.
 */
#define O2 __attribute__((optimize("O2")))

/** Full optimization as in -O2;
 *  also uses more aggressive automatic inlining of
 *  subprograms within a unit (Inlining of Subprograms)
 *  and attempts to vectorize loops. */
#define O3 __attribute__((optimize("O3")))

/** Optimize space usage (code and data)
 *  of resulting program.
 */
#define Os __attribute__((optimize("Os")))

/** Disregard strict standards compliance.
 * -Ofast enables all -O3 optimizations.
 * It also enables optimizations that are not valid for
 *  all standard-compliant programs.
 */
#define Ofast __attribute__((optimize("Ofast")))

/** Optimize for size.
 * -Oz enables all -Os optimizations that do not typically
 * increase code size.
 */
#define Oz __attribute__((optimize("Oz")))

/** Optimize for debugging.
 * -Og enables optimizations that do not interfere with
 * debugging.
 */
#define Og __attribute__((optimize("Og")))

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
#define __no_sanitize(x) __attribute__((no_sanitize(x)))
#define __no_sanitize_address __attribute__((no_sanitize_address))
#define __no_sanitize_thread __attribute__((no_sanitize_thread))
#define __no_sanitize_undefined __attribute__((no_sanitize_undefined))
#define __no_sanitize_coverage __attribute__((no_sanitize_coverage))

#define __synchronize __sync_synchronize()
#define __sync __synchronize

#define __unreachable __builtin_unreachable()

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define hot __attribute__((hot))
#define cold __attribute__((cold))

#define NoSecurityAnalysis __no_stack_protector __no_sanitize_address __no_sanitize_undefined __no_sanitize_thread
#define nsa NoSecurityAnalysis

#define NIF __no_instrument_function

#define int3                    \
	__asm__ __volatile__("int3" \
						 :      \
						 :      \
						 : "memory")

#define StackPush(stack, type, value) \
	*((type *)--stack) = value

#define StackPop(stack, type) \
	*((type *)stack++)

#define ReturnLogError(ret, format, ...) \
	{                                    \
		trace(format, ##__VA_ARGS__);    \
		return ret;                      \
	}                                    \
	while (0)                            \
	__builtin_unreachable()

#define AssertReturnError(condition, ret)          \
	do                                             \
	{                                              \
		if (__builtin_expect(!!(!(condition)), 0)) \
		{                                          \
			error("\"%s\" failed!", #condition);   \
			return ret;                            \
		}                                          \
	} while (0)

#endif // !__FENNIX_KERNEL_TYPES_H__
