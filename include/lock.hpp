#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

#include <types.h>
#include <cpu.hpp>

#ifdef __cplusplus

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

#define NEWLOCK(Name) LockClass Name

class SmartLock
{
private:
    LockClass *LockPointer = nullptr;

public:
    SmartLock(LockClass &Lock)
    {
        this->LockPointer = &Lock;
        this->LockPointer->Lock();
    }
    ~SmartLock() { this->LockPointer->Unlock(); }
};

#define SL_CONCAT(x, y) x##y
#define SMARTLOCK(LockClassName) SmartLock SL_CONCAT(lock##_, __COUNTER__)(LockClassName)

#endif

#endif // !__FENNIX_KERNEL_LOCK_H__
