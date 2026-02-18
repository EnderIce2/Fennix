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

#pragma once

#include <uchar.h>

typedef enum
{
	memory_order_relaxed = __ATOMIC_RELAXED,
	memory_order_consume = __ATOMIC_CONSUME,
	memory_order_acquire = __ATOMIC_ACQUIRE,
	memory_order_release = __ATOMIC_RELEASE,
	memory_order_acq_rel = __ATOMIC_ACQ_REL,
	memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

#define atomic_store_explicit(object, desired, order) \
	__atomic_store_n(object, desired, order)

#define atomic_store(object, desired) \
	__atomic_store_n(object, desired, __ATOMIC_SEQ_CST)

#define atomic_load_explicit(object, order) \
	__atomic_load_n(object, order)

#define atomic_load(object) \
	__atomic_load_n(object, __ATOMIC_SEQ_CST)

#define atomic_exchange_explicit(object, desired, order) \
	__atomic_exchange_n(object, desired, order)

#define atomic_exchange(object, desired) \
	__atomic_exchange_n(object, desired, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_strong_explicit(object, expected, desired, success, failure) \
	__atomic_compare_exchange_n(object, expected, desired, 0, success, failure)

#define atomic_compare_exchange_strong(object, expected, desired) \
	__atomic_compare_exchange_n(object, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_weak_explicit(object, expected, desired, success, failure) \
	__atomic_compare_exchange_n(object, expected, desired, 1, success, failure)

#define atomic_compare_exchange_weak(object, expected, desired) \
	__atomic_compare_exchange_n(object, expected, desired, 1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_fetch_add(object, operand) \
	__atomic_fetch_add(object, operand, __ATOMIC_SEQ_CST)

#define atomic_fetch_add_explicit(object, operand, order) \
	__atomic_fetch_add(object, operand, order)

#define atomic_fetch_sub(object, operand) \
	__atomic_fetch_sub(object, operand, __ATOMIC_SEQ_CST)

#define atomic_fetch_sub_explicit(object, operand, order) \
	__atomic_fetch_sub(object, operand, order)

#define atomic_fetch_or(object, operand) \
	__atomic_fetch_or(object, operand, __ATOMIC_SEQ_CST)

#define atomic_fetch_or_explicit(object, operand, order) \
	__atomic_fetch_or(object, operand, order)

#define atomic_fetch_xor(object, operand) \
	__atomic_fetch_xor(object, operand, __ATOMIC_SEQ_CST)

#define atomic_fetch_xor_explicit(object, operand, order) \
	__atomic_fetch_xor(object, operand, order)

#define atomic_fetch_and(object, operand) \
	__atomic_fetch_and(object, operand, __ATOMIC_SEQ_CST)

#define atomic_fetch_and_explicit(object, operand, order) \
	__atomic_fetch_and(object, operand, order)

#ifdef __cplusplus
typedef _Atomic(bool) atomic_bool;
#else
typedef _Atomic(_Bool) atomic_bool;
#endif
typedef _Atomic(char) atomic_char;
typedef _Atomic(signed char) atomic_schar;
typedef _Atomic(unsigned char) atomic_uchar;
typedef _Atomic(short) atomic_short;
typedef _Atomic(unsigned short) atomic_ushort;
typedef _Atomic(int) atomic_int;
typedef _Atomic(unsigned int) atomic_uint;
typedef _Atomic(long) atomic_long;
typedef _Atomic(unsigned long) atomic_ulong;
typedef _Atomic(long long) atomic_llong;
typedef _Atomic(unsigned long long) atomic_ullong;
typedef _Atomic(char16_t) atomic_char16_t;
typedef _Atomic(char32_t) atomic_char32_t;
typedef _Atomic(wchar_t) atomic_wchar_t;
typedef _Atomic(int_least8_t) atomic_int_least8_t;
typedef _Atomic(uint_least8_t) atomic_uint_least8_t;
typedef _Atomic(int_least16_t) atomic_int_least16_t;
typedef _Atomic(uint_least16_t) atomic_uint_least16_t;
typedef _Atomic(int_least32_t) atomic_int_least32_t;
typedef _Atomic(uint_least32_t) atomic_uint_least32_t;
typedef _Atomic(int_least64_t) atomic_int_least64_t;
typedef _Atomic(uint_least64_t) atomic_uint_least64_t;
typedef _Atomic(int_fast8_t) atomic_int_fast8_t;
typedef _Atomic(uint_fast8_t) atomic_uint_fast8_t;
typedef _Atomic(int_fast16_t) atomic_int_fast16_t;
typedef _Atomic(uint_fast16_t) atomic_uint_fast16_t;
typedef _Atomic(int_fast32_t) atomic_int_fast32_t;
typedef _Atomic(uint_fast32_t) atomic_uint_fast32_t;
typedef _Atomic(int_fast64_t) atomic_int_fast64_t;
typedef _Atomic(uint_fast64_t) atomic_uint_fast64_t;
typedef _Atomic(intptr_t) atomic_intptr_t;
typedef _Atomic(uintptr_t) atomic_uintptr_t;
typedef _Atomic(size_t) atomic_size_t;
typedef _Atomic(ptrdiff_t) atomic_ptrdiff_t;
typedef _Atomic(intmax_t) atomic_intmax_t;
typedef _Atomic(uintmax_t) atomic_uintmax_t;

void atomic_thread_fence(memory_order order);
