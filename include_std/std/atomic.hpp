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

#ifndef __FENNIX_KERNEL_STD_ATOMIC_H__
#define __FENNIX_KERNEL_STD_ATOMIC_H__

#include <types.h>
#include <cstddef>
#include <debug.h>

namespace std
{
#define _atomic(T) T
#define builtin_atomic_n(name) __atomic_##name##_n
#define builtin_atomic(name) __atomic_##name

    /**
     * @brief Specifies the memory ordering constraints for atomic operations.
     *
     * This enum specifies the possible values for the memory order parameter of atomic operations.
     *
     * Possible values are:
     *
     * - memory_order_relaxed: There are no synchronization
     * or ordering constraints imposed on other reads or writes,
     * only this operation's atomicity is guaranteed.
     *
     * - memory_order_consume: A load operation with this
     * memory order performs a consume operation on the
     * affected memory location: no reads or writes in the
     * current thread dependent on the value currently loaded
     * can be reordered before this load.
     *
     * - memory_order_acquire: A load operation with this
     * memory order performs the acquire operation on the
     * affected memory location: no reads or writes in the
     * current thread can be reordered before this load.
     *
     * - memory_order_release: A store operation with this
     * memory order performs the release operation: no reads
     * or writes in the current thread can be reordered after
     * this store.
     *
     * - memory_order_acq_rel: A read-modify-write operation
     * with this memory order is both an acquire operation
     * and a release operation.
     *
     * - memory_order_seq_cst: A load operation with this
     * memory order performs an acquire operation, a store
     * performs a release operation, and read-modify-write
     * performs both an acquire operation and a release
     * operation, plus a single total order exists in which
     * all threads observe all modifications in the same order.
     */
    enum class memory_order : int
    {
        relaxed,
        consume,
        acquire,
        release,
        acq_rel,
        seq_cst
    };

    inline constexpr memory_order memory_order_relaxed = memory_order::relaxed;
    inline constexpr memory_order memory_order_consume = memory_order::consume;
    inline constexpr memory_order memory_order_acquire = memory_order::acquire;
    inline constexpr memory_order memory_order_release = memory_order::release;
    inline constexpr memory_order memory_order_acq_rel = memory_order::acq_rel;
    inline constexpr memory_order memory_order_seq_cst = memory_order::seq_cst;

    template <typename T>
    class atomic
    {
        _atomic(T) value;

    public:
        atomic() noexcept : value(0) {}
        atomic(T desired) noexcept : value(desired) {}
        // atomic(const atomic &) = delete;

        /**
         * @brief Load the value of the atomic variable
         *
         * @note Order must be one of memory_order::relaxed, memory_order::consume, memory_order::acquire or memory_order::seq_cst
         *
         * @param order Memory order constraint to use
         * @return The value of the atomic variable
         */
        T load(memory_order order = memory_order::seq_cst) const noexcept
        {
            return builtin_atomic_n(load)(&this->value, static_cast<int>(order));
        }

        /**
         * @copydoc load()
         */
        T load(memory_order order = memory_order::seq_cst) const volatile noexcept
        {
            return builtin_atomic_n(load)(&this->value, static_cast<int>(order));
        }

        /**
         * @brief Store the value of the atomic variable
         *
         * @note Order must be one of memory_order::relaxed, memory_order::release or memory_order::seq_cst
         *
         * @param desired The value to store
         * @param order Memory order constraint to use
         */
        void store(T desired, memory_order order = memory_order::seq_cst) noexcept
        {
            builtin_atomic_n(store)(&this->value, desired, static_cast<int>(order));
        }

        /**
         * @copydoc store()
         */
        void store(T desired, memory_order order = memory_order::seq_cst) volatile noexcept
        {
            builtin_atomic_n(store)(&this->value, desired, static_cast<int>(order));
        }

        /**
         * @brief Exchange the value of the atomic variable
         *
         * @param desired The value to exchange
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the exchange
         */
        T exchange(T desired, memory_order order = memory_order::seq_cst) noexcept
        {
            return builtin_atomic_n(exchange)(&this->value, desired, static_cast<int>(order));
        }

        /**
         * @copydoc exchange()
         */
        T exchange(T desired, memory_order order = memory_order::seq_cst) volatile noexcept
        {
            return builtin_atomic_n(exchange)(&this->value, desired, static_cast<int>(order));
        }

        /**
         * @brief Compare and exchange the value of the atomic variable
         *
         * @param expected The expected value
         * @param desired The desired value
         * @param success Memory order constraint to use if the exchange succeeds
         * @param failure Memory order constraint to use if the exchange fails
         * @return True if the exchange succeeded, false otherwise
         */
        bool compare_exchange_weak(T &expected, T desired, std::memory_order success, std::memory_order failure) noexcept
        {
            return builtin_atomic(compare_exchange_weak)(&this->value, &expected, desired, false, success, failure);
        }

        /**
         * @copydoc compare_exchange_weak()
         */
        bool compare_exchange_weak(T &expected, T desired, std::memory_order success, std::memory_order failure) volatile noexcept
        {
            return builtin_atomic(compare_exchange_weak)(&this->value, &expected, desired, false, success, failure);
        }

        /**
         * @brief Compare and exchange the value of the atomic variable
         *
         * @param expected The expected value
         * @param desired The desired value
         * @param order Memory order constraint to use
         * @return True if the exchange succeeded, false otherwise
         */
        bool compare_exchange_weak(T &expected, T desired, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(compare_exchange_weak)(&this->value, &expected, desired, false, order, static_cast<int>(order));
        }

        /**
         * @copydoc compare_exchange_weak()
         */
        bool compare_exchange_weak(T &expected, T desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(compare_exchange_weak)(&this->value, &expected, desired, false, order, static_cast<int>(order));
        }

        /**
         * @brief Compare and exchange the value of the atomic variable
         *
         * @param expected The expected value
         * @param desired The desired value
         * @param success Memory order constraint to use if the exchange succeeds
         * @param failure Memory order constraint to use if the exchange fails
         * @return True if the exchange succeeded, false otherwise
         */
        bool compare_exchange_strong(T &expected, T desired, std::memory_order success, std::memory_order failure) noexcept
        {
            return builtin_atomic(compare_exchange_strong)(&this->value, &expected, desired, true, success, failure);
        }

        /**
         * @copydoc compare_exchange_strong()
         */
        bool compare_exchange_strong(T &expected, T desired, std::memory_order success, std::memory_order failure) volatile noexcept
        {
            return builtin_atomic(compare_exchange_strong)(&this->value, &expected, desired, true, success, failure);
        }

        /**
         * @brief Compare and exchange the value of the atomic variable
         *
         * @param expected The expected value
         * @param desired The desired value
         * @param order Memory order constraint to use
         * @return True if the exchange succeeded, false otherwise
         */
        bool compare_exchange_strong(T &expected, T desired, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(compare_exchange_strong)(&this->value, &expected, desired, true, order, static_cast<int>(order));
        }

        /**
         * @copydoc compare_exchange_strong()
         */
        bool compare_exchange_strong(T &expected, T desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(compare_exchange_strong)(&this->value, &expected, desired, true, order, static_cast<int>(order));
        }

        /**
         * @brief Fetch and add the value of the atomic variable
         *
         * @param arg The value to add
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the addition
         */
        T fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_add)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_add()
         */
        T fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_add)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Fetch and subtract the value of the atomic variable
         *
         * @param arg The value to subtract
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the subtraction
         */
        T fetch_sub(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_sub)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_sub()
         */
        T fetch_sub(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_sub)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Fetch and bitwise AND the value of the atomic variable
         *
         * @param arg The value to AND
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the AND
         */
        T fetch_and(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_and)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_and()
         */
        T fetch_and(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_and)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Fetch and bitwise OR the value of the atomic variable
         *
         * @param arg The value to OR
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the OR
         */
        T fetch_or(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_or)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_or()
         */
        T fetch_or(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_or)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Fetch and bitwise XOR the value of the atomic variable
         *
         * @param arg The value to XOR
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the XOR
         */
        T fetch_xor(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_xor)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_xor()
         */
        T fetch_xor(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_xor)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Fetch and bitwise NAND the value of the atomic variable
         *
         * @param arg The value to NAND
         * @param order Memory order constraint to use
         * @return The value of the atomic variable before the NAND
         */
        T fetch_nand(T arg, std::memory_order order = std::memory_order_seq_cst) noexcept
        {
            return builtin_atomic(fetch_nand)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @copydoc fetch_nand()
         */
        T fetch_nand(T arg, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
        {
            return builtin_atomic(fetch_nand)(&this->value, arg, static_cast<int>(order));
        }

        /**
         * @brief Notify all threads waiting on this atomic variable
         */
        void notify_all() noexcept
        {
            fixme("not implemented");
        }

        /**
         * @copydoc notify_all()
         */
        void notify_all() volatile noexcept
        {
            fixme("not implemented");
        }

        /**
         * @brief Notify one thread waiting on this atomic variable
         */
        void notify_one() noexcept
        {
            fixme("not implemented");
        }

        /**
         * @copydoc notify_one()
         */
        void notify_one() volatile noexcept
        {
            fixme("not implemented");
        }

        /**
         * @brief Wait for the atomic variable to change
         *
         * @param old The value to wait for
         * @param order Memory order constraint to use
         */
        void wait(T old, std::memory_order order = std::memory_order::seq_cst) const noexcept
        {
            fixme("not implemented");
        }

        /**
         * @copydoc wait()
         */
        void wait(T old, std::memory_order order = std::memory_order::seq_cst) const volatile noexcept
        {
            fixme("not implemented");
        }

        /**
         * @brief Check whether this atomic type is lock-free
         * @return True if this atomic type is lock-free
         */
        bool is_lock_free() const noexcept
        {
            fixme("not implemented");
            return true;
        }

        /**
         * @copydoc is_lock_free()
         */
        bool is_lock_free() const volatile noexcept
        {
            fixme("not implemented");
            return true;
        }

        /**
         * @brief Equals true if this atomic type is always lock-free
         */
        static constexpr bool is_always_lock_free = true;

        T operator++() noexcept { return this->fetch_add(1) + 1; }
        T operator--() noexcept { return this->fetch_sub(1) - 1; }
        T operator++(int) noexcept { return this->fetch_add(1); }
        T operator--(int) noexcept { return this->fetch_sub(1); }

        T operator+=(T desired) noexcept { return this->fetch_add(desired) + desired; }
        T operator-=(T desired) noexcept { return this->fetch_sub(desired) - desired; }
        // T operator+=(std::ptrdiff_t desired) noexcept { return this->fetch_add(desired) + desired; }
        // T operator-=(std::ptrdiff_t desired) noexcept { return this->fetch_sub(desired) - desired; }

        T operator&=(T desired) noexcept { return this->fetch_and(desired) & desired; }
        T operator|=(T desired) noexcept { return this->fetch_or(desired) | desired; }
        T operator^=(T desired) noexcept { return this->fetch_xor(desired) ^ desired; }

        T operator->() noexcept { return this->load(); }
        T operator~() noexcept { return this->fetch_nand(-1); }

        bool operator==(const atomic &other) const noexcept { return this->load() == other.load(); }
        bool operator==(T other) const noexcept { return this->load() == other; }

        atomic &operator=(const atomic &) = delete;
        T operator=(T desired) noexcept
        {
            this->store(desired);
            return desired;
        }

        operator bool() noexcept { return this->load() != 0; }
        // operator T() noexcept { return this->load(); }
        operator T() const noexcept { return this->load(); }
    };

    typedef atomic<bool> atomic_bool;
    typedef atomic<char> atomic_char;
    typedef atomic<signed char> atomic_schar;
    typedef atomic<unsigned char> atomic_uchar;
    typedef atomic<short> atomic_short;
    typedef atomic<unsigned short> atomic_ushort;
    typedef atomic<int> atomic_int;
    typedef atomic<unsigned int> atomic_uint;
    typedef atomic<long> atomic_long;
    typedef atomic<unsigned long> atomic_ulong;
    typedef atomic<long long> atomic_llong;
    typedef atomic<unsigned long long> atomic_ullong;
    typedef atomic<char16_t> atomic_char16_t;
    typedef atomic<char32_t> atomic_char32_t;
    typedef atomic<wchar_t> atomic_wchar_t;
    typedef atomic<int8_t> atomic_int8_t;
    typedef atomic<uint8_t> atomic_uint8_t;
    typedef atomic<int16_t> atomic_int16_t;
    typedef atomic<uint16_t> atomic_uint16_t;
    typedef atomic<int32_t> atomic_int32_t;
    typedef atomic<uint32_t> atomic_uint32_t;
    typedef atomic<int64_t> atomic_int64_t;
    typedef atomic<uint64_t> atomic_uint64_t;
    typedef atomic<int_least8_t> atomic_int_least8_t;
    typedef atomic<uint_least8_t> atomic_uint_least8_t;
    typedef atomic<int_least16_t> atomic_int_least16_t;
    typedef atomic<uint_least16_t> atomic_uint_least16_t;
    typedef atomic<int_least32_t> atomic_int_least32_t;
    typedef atomic<uint_least32_t> atomic_uint_least32_t;
    typedef atomic<int_least64_t> atomic_int_least64_t;
    typedef atomic<uint_least64_t> atomic_uint_least64_t;
    typedef atomic<int_fast8_t> atomic_int_fast8_t;
    typedef atomic<uint_fast8_t> atomic_uint_fast8_t;
    typedef atomic<int_fast16_t> atomic_int_fast16_t;
    typedef atomic<uint_fast16_t> atomic_uint_fast16_t;
    typedef atomic<int_fast32_t> atomic_int_fast32_t;
    typedef atomic<uint_fast32_t> atomic_uint_fast32_t;
    typedef atomic<int_fast64_t> atomic_int_fast64_t;
    typedef atomic<uint_fast64_t> atomic_uint_fast64_t;
    typedef atomic<intptr_t> atomic_intptr_t;
    typedef atomic<uintptr_t> atomic_uintptr_t;
    typedef atomic<size_t> atomic_size_t;
    typedef atomic<ptrdiff_t> atomic_ptrdiff_t;
    typedef atomic<intmax_t> atomic_intmax_t;
    typedef atomic<uintmax_t> atomic_uintmax_t;
}

#undef builtin_atomic_n
#undef builtin_atomic

#endif // !__FENNIX_KERNEL_STD_ATOMIC_H__
