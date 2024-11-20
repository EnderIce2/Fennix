#ifndef __FENNIX_KERNEL_ATOMIC_H__
#define __FENNIX_KERNEL_ATOMIC_H__

#define _Atomic(T) T
#define builtin_atomic(name) __atomic_##name##_n

namespace
{
    enum MemoryBorder
    {
        Relaxed = __ATOMIC_RELAXED,
        Acquire = __ATOMIC_ACQUIRE,
        Release = __ATOMIC_RELEASE,
        AcqRel = __ATOMIC_ACQ_REL,
        SeqCst = __ATOMIC_SEQ_CST
    };

    template <typename T>
    class Atomic
    {
        _Atomic(T) Value;

    public:
        Atomic(T Init) : Value(Init) {}

        T Load(MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(load)(&Value, Order);
        }

        void Store(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(store)(&Value, v, Order);
        }

        T Exchange(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(exchange)(&Value, v, Order);
        }

        bool CompareExchange(T &Expected, T Desired, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(compare_exchange)(&Value, &Expected, Desired, true, Order, Order);
        }
        
        T FetchAdd(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(fetch_add)(&Value, v, Order);
        }

        T FetchSub(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(fetch_sub)(&Value, v, Order);
        }

        T FetchAnd(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(fetch_and)(&Value, v, Order);
        }

        T FetchOr(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(fetch_or)(&Value, v, Order);
        }

        T FetchXor(T v, MemoryBorder Order = MemoryBorder::SeqCst)
        {
            return builtin_atomic(fetch_xor)(&Value, v, Order);
        }

        T operator++()
        {
            return FetchAdd(1) + 1;
        }

        T operator++(int)
        {
            return FetchAdd(1);
        }

        T operator--()
        {
            return FetchSub(1) - 1;
        }

        T operator--(int)
        {
            return FetchSub(1);
        }

        T operator+=(T v)
        {
            return FetchAdd(v) + v;
        }

        T operator-=(T v)
        {
            return FetchSub(v) - v;
        }

        T operator&=(T v)
        {
            return FetchAnd(v) & v;
        }

        T operator|=(T v)
        {
            return FetchOr(v) | v;
        }

        T operator^=(T v)
        {
            return FetchXor(v) ^ v;
        }

        T operator=(T v)
        {
            Store(v);
            return v;
        }
    };
}
#undef builtin_atomic

#endif // !__FENNIX_KERNEL_ATOMIC_H__
