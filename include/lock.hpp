#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

#include <types.h>
#include <cpu.hpp>

#pragma GCC diagnostic ignored "-Wvolatile"

EXTERNC void SpinLock_Lock(volatile uint64_t *LockData);
EXTERNC void SpinLock_Unlock(volatile uint64_t *LockData);
EXTERNC uint64_t SpinLock_CheckAndLock(volatile uint64_t *LockData);
EXTERNC uint64_t SpinLock_WithTimeout(volatile uint64_t *LockData, volatile uint64_t Iterations);

#ifdef __cplusplus
/** @brief Please use this macro to create a new lock. */
class LockClass
{
    struct SpinLockData
    {
        uint64_t LockData = 0x0;
        const char *CurrentHolder = "(nul)";
        const char *AttemptingToGet = "(nul)";
        uint64_t Count = 0;
        long Core = 0;
    };

private:
    SpinLockData LockData;
    // bool IsLocked = false;

public:
    SpinLockData *GetLockData() { return &LockData; }
    int Lock(const char *FunctionName);
    int Unlock();
};
/** @brief Please use this macro to create a new smart lock. */
class SmartLockClass
{
private:
    LockClass *LockPointer = nullptr;

public:
    SmartLockClass(LockClass &Lock, const char *FunctionName)
    {
        this->LockPointer = &Lock;
        this->LockPointer->Lock(FunctionName);
    }
    ~SmartLockClass() { this->LockPointer->Unlock(); }
};
/** @brief Please use this macro to create a new smart critical section lock. */
class SmartLockCriticalSectionClass
{
private:
    LockClass *LockPointer = nullptr;
    bool InterruptsEnabled = false;

public:
    SmartLockCriticalSectionClass(LockClass &Lock, const char *FunctionName)
    {
        if (CPU::Interrupts(CPU::Check))
            InterruptsEnabled = true;
        CPU::Interrupts(CPU::Disable);
        this->LockPointer = &Lock;
        this->LockPointer->Lock(FunctionName);
    }
    ~SmartLockCriticalSectionClass()
    {
        if (InterruptsEnabled)
            CPU::Interrupts(CPU::Enable);
        this->LockPointer->Unlock();
    }
};
/** @brief Please use this macro to create a new critical section. */
class SmartCriticalSectionClass
{
private:
    bool InterruptsEnabled = false;

public:
    bool IsInterruptsEnabled() { return InterruptsEnabled; }

    SmartCriticalSectionClass()
    {
        if (CPU::Interrupts(CPU::Check))
            InterruptsEnabled = true;
        CPU::Interrupts(CPU::Disable);
    }
    ~SmartCriticalSectionClass()
    {
        if (InterruptsEnabled)
            CPU::Interrupts(CPU::Enable);
    }
};

/** @brief Create a new lock (can be used with SmartCriticalSection). */
#define NewLock(Name) LockClass Name
/** @brief Simple lock that is automatically released when the scope ends. */
#define SmartLock(LockClassName) SmartLockClass CONCAT(lock##_, __COUNTER__)(LockClassName, __FUNCTION__)
/** @brief Simple critical section that is automatically released when the scope ends and interrupts are restored if they were enabled. */
#define SmartCriticalSection(LockClassName) SmartLockCriticalSectionClass CONCAT(lock##_, __COUNTER__)(LockClassName, __FUNCTION__)
/** @brief Automatically disable interrupts and restore them when the scope ends. */
#define CriticalSection SmartCriticalSectionClass

#endif // __cplusplus
#endif // !__FENNIX_KERNEL_LOCK_H__
