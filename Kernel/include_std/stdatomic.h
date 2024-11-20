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
