#ifndef __FENNIX_KERNEL_SMART_POINTER_H__
#define __FENNIX_KERNEL_SMART_POINTER_H__

#include <types.h>

#include <debug.h>

// show debug messages
// #define DEBUG_SMARTPOINTERS 1

#ifdef DEBUG_SMARTPOINTERS
#define spdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define spdbg(m, ...)
#endif

/**
 * @brief A smart pointer class
 *
 * This class is a smart pointer class. It is used to manage the lifetime of
 * objects. It is a reference counted pointer, so when the last reference to
 * the object is removed, the object is deleted.
 *
 * Basic Usage:
 * SmartPointer<char> pointer(new char());
 * *pointer = 'a';
 * printf("%c", *pointer); // Prints "a"
 */
template <class T>
class SmartPointer
{
    T *m_RealPointer;

public:
    explicit SmartPointer(T *Pointer = nullptr)
    {
        spdbg("Smart pointer created (%#lx)", m_RealPointer);
        m_RealPointer = Pointer;
    }

    ~SmartPointer()
    {
        spdbg("Smart pointer deleted (%#lx)", m_RealPointer);
        delete m_RealPointer, m_RealPointer = nullptr;
    }

    T &operator*()
    {
        spdbg("Smart pointer dereferenced (%#lx)", m_RealPointer);
        return *m_RealPointer;
    }

    T *operator->()
    {
        spdbg("Smart pointer dereferenced (%#lx)", m_RealPointer);
        return m_RealPointer;
    }
};

template <class T>
class AutoPointer
{
};

template <class T>
class UniquePointer
{
};

template <class T>
class WeakPointer
{
};

template <typename T>
class SharedPointer
{
private:
    class Counter
    {
    private:
        unsigned int m_RefCount{};

    public:
        Counter() : m_RefCount(0) { spdbg("Counter %#lx created", this); };
        Counter(const Counter &) = delete;
        Counter &operator=(const Counter &) = delete;
        ~Counter() { spdbg("Counter %#lx deleted", this); }
        void Reset()
        {
            m_RefCount = 0;
            spdbg("Counter reset");
        }

        unsigned int Get()
        {
            return m_RefCount;
            spdbg("Counter returned");
        }

        void operator++()
        {
            m_RefCount++;
            spdbg("Counter incremented");
        }

        void operator++(int)
        {
            m_RefCount++;
            spdbg("Counter incremented");
        }

        void operator--()
        {
            m_RefCount--;
            spdbg("Counter decremented");
        }

        void operator--(int)
        {
            m_RefCount--;
            spdbg("Counter decremented");
        }
    };

    Counter *m_ReferenceCounter;
    T *m_RealPointer;

public:
    explicit SharedPointer(T *Pointer = nullptr)
    {
        m_RealPointer = Pointer;
        m_ReferenceCounter = new Counter();
        spdbg("[%#lx] Shared pointer created (ptr=%#lx, ref=%#lx)", this, Pointer, m_ReferenceCounter);
        if (Pointer)
            (*m_ReferenceCounter)++;
    }

    SharedPointer(SharedPointer<T> &SPtr)
    {
        spdbg("[%#lx] Shared pointer copied (ptr=%#lx, ref=%#lx)", this, SPtr.m_RealPointer, SPtr.m_ReferenceCounter);
        m_RealPointer = SPtr.m_RealPointer;
        m_ReferenceCounter = SPtr.m_ReferenceCounter;
        (*m_ReferenceCounter)++;
    }

    ~SharedPointer()
    {
        spdbg("[%#lx] Shared pointer destructor called", this);
        (*m_ReferenceCounter)--;
        if (m_ReferenceCounter->Get() == 0)
        {
            spdbg("[%#lx] Shared pointer deleted (ptr=%#lx, ref=%#lx)", this, m_RealPointer, m_ReferenceCounter);
            delete m_ReferenceCounter, m_ReferenceCounter = nullptr;
            delete m_RealPointer, m_RealPointer = nullptr;
        }
    }

    unsigned int GetCount()
    {
        spdbg("[%#lx] Shared pointer count (%d)", this, m_ReferenceCounter->Get());
        return m_ReferenceCounter->Get();
    }

    T *Get()
    {
        spdbg("[%#lx] Shared pointer get (%#lx)", this, m_RealPointer);
        return m_RealPointer;
    }

    T &operator*()
    {
        spdbg("[%#lx] Shared pointer dereference (ptr*=%#lx)", this, *m_RealPointer);
        return *m_RealPointer;
    }

    T *operator->()
    {
        spdbg("[%#lx] Shared pointer dereference (ptr->%#lx)", this, m_RealPointer);
        return m_RealPointer;
    }

    void reset(T *Pointer = nullptr)
    {
        if (m_RealPointer == Pointer)
            return;
        spdbg("[%#lx] Shared pointer reset (ptr=%#lx, ref=%#lx)", this, Pointer, m_ReferenceCounter);
        (*m_ReferenceCounter)--;
        if (m_ReferenceCounter->Get() == 0)
        {
            delete m_RealPointer;
            delete m_ReferenceCounter;
        }
        m_RealPointer = Pointer;
        m_ReferenceCounter = new Counter();
        if (Pointer)
            (*m_ReferenceCounter)++;
    }

    void reset()
    {
        spdbg("[%#lx] Shared pointer reset (ptr=%#lx, ref=%#lx)", this, m_RealPointer, m_ReferenceCounter);
        if (m_ReferenceCounter->Get() == 1)
        {
            delete m_RealPointer, m_RealPointer = nullptr;
            delete m_ReferenceCounter, m_ReferenceCounter = nullptr;
        }
        else
        {
            (*m_ReferenceCounter)--;
        }
    }

    void swap(SharedPointer<T> &Other)
    {
        spdbg("[%#lx] Shared pointer swap (ptr=%#lx, ref=%#lx <=> ptr=%#lx, ref=%#lx)",
              this, m_RealPointer, m_ReferenceCounter, Other.m_RealPointer, Other.m_ReferenceCounter);
        T *tempRealPointer = m_RealPointer;
        Counter *tempReferenceCounter = m_ReferenceCounter;
        m_RealPointer = Other.m_RealPointer;
        m_ReferenceCounter = Other.m_ReferenceCounter;
        Other.m_RealPointer = tempRealPointer;
        Other.m_ReferenceCounter = tempReferenceCounter;
    }
};

template <typename T>
struct RemoveReference
{
    typedef T type;
};

template <typename T>
struct RemoveReference<T &>
{
    typedef T type;
};

template <typename T>
struct RemoveReference<T &&>
{
    typedef T type;
};

template <typename T>
using RemoveReference_t = typename RemoveReference<T>::type;

template <typename T>
T &&forward(RemoveReference_t<T> &t)
{
    return static_cast<T &&>(t);
};

template <typename T>
T &&forward(RemoveReference_t<T> &&t)
{
    return static_cast<T &&>(t);
};

template <typename T, typename... Args>
SharedPointer<T> MakeShared(Args &&...args)
{
    return SharedPointer<T>(new T(forward<Args>(args)...));
};

#endif // !__FENNIX_KERNEL_SMART_POINTER_H__
