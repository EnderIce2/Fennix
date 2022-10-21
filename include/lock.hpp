#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

/*
TODO: Add deadlock detection.
*/

#include <types.h>
#include <cpu.hpp>

#ifdef __cplusplus
/** @brief Please use this macro to create a new lock. */
class LockClass
{
private:
    bool IsLocked = false;

public:
    int Lock()
    {
        while (!__sync_bool_compare_and_swap(&IsLocked, false, true))
            CPU::Pause();
        __sync_synchronize();
        return 0;
    }

    int Unlock()
    {
        __sync_synchronize();
        __atomic_store_n(&IsLocked, false, __ATOMIC_SEQ_CST);
        IsLocked = false;
        return 0;
    }
};
/** @brief Please use this macro to create a new smart lock. */
class SmartLockClass
{
private:
    LockClass *LockPointer = nullptr;

public:
    SmartLockClass(LockClass &Lock)
    {
        this->LockPointer = &Lock;
        this->LockPointer->Lock();
    }
    ~SmartLockClass() { this->LockPointer->Unlock(); }
};
/** @brief Please use this macro to create a new smart critical section lock. */
class SmartCriticalSectionClass
{
private:
    LockClass *LockPointer = nullptr;
    bool InterruptsEnabled = false;

public:
    SmartCriticalSectionClass(LockClass &Lock)
    {
        if (CPU::Interrupts(CPU::Check))
            InterruptsEnabled = true;
        CPU::Interrupts(CPU::Disable);
        this->LockPointer = &Lock;
        this->LockPointer->Lock();
    }
    ~SmartCriticalSectionClass()
    {
        if (InterruptsEnabled)
            CPU::Interrupts(CPU::Enable);
        this->LockPointer->Unlock();
    }
};

/** @brief Create a new lock (can be used with SmartCriticalSection). */
#define NewLock(Name) LockClass Name
/** @brief Simple lock that is automatically released when the scope ends. */
#define SmartLock(LockClassName) SmartLockClass CONCAT(lock##_, __COUNTER__)(LockClassName)
/** @brief Simple critical section that is automatically released when the scope ends and interrupts are restored if they were enabled. */
#define SmartCriticalSection(LockClassName) SmartCriticalSectionClass CONCAT(lock##_, __COUNTER__)(LockClassName)

#endif // __cplusplus
#endif // !__FENNIX_KERNEL_LOCK_H__
