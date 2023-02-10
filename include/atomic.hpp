#ifndef __FENNIX_KERNEL_ATOMIC_H__
#define __FENNIX_KERNEL_ATOMIC_H__

#define _Atomic(T) T
#define builtin_atomic_n(name) __atomic_##name##_n
#define builtin_atomic(name) __atomic_##name

enum MemoryBorder
{
    /**
     * @brief Relaxed memory border
     *
     * This memory ordering specifies that the
     * operation on the atomic variable has no
     * synchronization with other memory accesses.
     * This is the most relaxed ordering and provides
     * the least synchronization.
     */
    Relaxed = __ATOMIC_RELAXED,

    /**
     * @brief Acquire memory border
     *
     * This memory ordering specifies that subsequent
     * memory accesses after the atomic operation
     * cannot be reordered before the atomic operation.
     * This ordering provides synchronization with
     * subsequent loads.
     */
    Acquire = __ATOMIC_ACQUIRE,

    /**
     * @brief Release memory border
     *
     * This memory ordering specifies that previous
     * memory accesses before the atomic operation
     * cannot be reordered after the atomic operation.
     * This ordering provides synchronization with
     * previous stores.
     */
    Release = __ATOMIC_RELEASE,

    /**
     * @brief Acquire and release memory border
     *
     * This memory ordering combines both the acquire
     * and release memory orderings. This ordering
     * provides synchronization with both previous
     * stores and subsequent loads.
     */
    AcqRel = __ATOMIC_ACQ_REL,

    /**
     * @brief Sequentially consistent memory border
     *
     * This memory ordering is a combination of
     * @see AcqRel and the additional
     * guarantee of a single total order of all
     * @see SeqCst operations on the same object.
     */
    SeqCst = __ATOMIC_SEQ_CST
};

template <typename T>
class Atomic
{
    _Atomic(T) m_Value;

public:
    Atomic(T Init) : m_Value(Init) {}

    /**
     * @brief Load the value of the atomic variable
     *
     * @param Order The memory border to use
     * @return T The value of the atomic variable
     */
    T Load(MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic_n(load)(&m_Value, Order);
    }

    /**
     * @brief Store a value to the atomic variable
     *
     * @param v The value to store
     * @param Order The memory border to use
     */
    void Store(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic_n(store)(&m_Value, v, Order);
    }

    /**
     * @brief Exchange the value of the atomic variable
     *
     * @param v The value to exchange
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T Exchange(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic_n(exchange)(&m_Value, v, Order);
    }

    /**
     * @brief Compare and exchange the value of the atomic variable
     *
     * @param Expected The expected value
     * @param Desired The desired value
     * @param Order The memory border to use
     * @return true If the exchange was successful
     * @return false If the exchange was not successful
     */
    bool CompareExchange(T &Expected, T Desired, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic_n(compare_exchange)(&m_Value, &Expected, Desired, true, Order, Order);
    }

    /**
     * @brief Fetch and add the value of the atomic variable
     *
     * @param v The value to add
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchAdd(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_add)(&m_Value, v, Order);
    }

    /**
     * @brief Fetch and subtract the value of the atomic variable
     *
     * @param v The value to subtract
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchSub(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_sub)(&m_Value, v, Order);
    }

    /**
     * @brief Fetch and and the value of the atomic variable
     *
     * @param v The value to and
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchAnd(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_and)(&m_Value, v, Order);
    }

    /**
     * @brief Fetch and or the value of the atomic variable
     *
     * @param v The value to or
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchOr(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_or)(&m_Value, v, Order);
    }

    /**
     * @brief Fetch and xor the value of the atomic variable
     *
     * @param v The value to xor
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchXor(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_xor)(&m_Value, v, Order);
    }

    /**
     * @brief Fetch and nand the value of the atomic variable
     *
     * @param v The value to nand
     * @param Order The memory border to use
     * @return T The old value of the atomic variable
     */
    T FetchNand(T v, MemoryBorder Order = MemoryBorder::SeqCst)
    {
        return builtin_atomic(fetch_nand)(&m_Value, v, Order);
    }

    operator bool() { return this->Load() != 0; }
    T operator->() { return this->Load(); }
    T operator++() { return this->FetchAdd(1) + 1; }
    T operator--() { return this->FetchSub(1) - 1; }
    T operator++(int) { return this->FetchAdd(1); }
    T operator--(int) { return this->FetchSub(1); }
    T operator+=(T v) { return this->FetchAdd(v) + v; }
    T operator-=(T v) { return this->FetchSub(v) - v; }
    T operator&=(T v) { return this->FetchAnd(v) & v; }
    T operator|=(T v) { return this->FetchOr(v) | v; }
    T operator^=(T v) { return this->FetchXor(v) ^ v; }
    T operator~() { return this->FetchNand(-1); }
    T operator=(T v)
    {
        this->Store(v);
        return v;
    }
};

#undef builtin_atomic_n
#undef builtin_atomic

#endif // !__FENNIX_KERNEL_ATOMIC_H__
