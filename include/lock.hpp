#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

#include <types.h>

#include <atomic.hpp>
#include <cpu.hpp>

#ifdef __cplusplus

/* Enabled ONLY on crash. */
extern bool ForceUnlock;

/** @brief Please use this macro to create a new lock. */
class LockClass
{
    struct SpinLockData
    {
        Atomic<uint64_t> LockData = 0x0;
        Atomic<const char *> CurrentHolder = "(nul)";
        Atomic<const char *> AttemptingToGet = "(nul)";
        Atomic<uintptr_t> StackPointerHolder = 0;
        Atomic<uintptr_t> StackPointerAttempt = 0;
        Atomic<size_t> Count = 0;
        Atomic<long> Core = 0;
    };

private:
    SpinLockData LockData;
    Atomic<bool> IsLocked = false;
    Atomic<unsigned long> DeadLocks = 0;

    void DeadLock(SpinLockData Lock);
    void TimeoutDeadLock(SpinLockData Lock, uint64_t Timeout);

public:
    SpinLockData *GetLockData() { return &LockData; }
    int Lock(const char *FunctionName);
    int Unlock();

    int TimeoutLock(const char *FunctionName, uint64_t Timeout);
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

class SmartTimeoutLockClass
{
private:
    LockClass *LockPointer = nullptr;

public:
    SmartTimeoutLockClass(LockClass &Lock, const char *FunctionName, uint64_t Timeout)
    {
        this->LockPointer = &Lock;
        this->LockPointer->TimeoutLock(FunctionName, Timeout);
    }
    ~SmartTimeoutLockClass() { this->LockPointer->Unlock(); }
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
        this->LockPointer->Unlock();
        if (InterruptsEnabled)
            CPU::Interrupts(CPU::Enable);
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

/** @brief Simple lock with timeout that is automatically released when the scope ends. */
#define SmartTimeoutLock(LockClassName, Timeout) SmartTimeoutLockClass CONCAT(lock##_, __COUNTER__)(LockClassName, __FUNCTION__, Timeout)

/** @brief Simple critical section that is automatically released when the scope ends and interrupts are restored if they were enabled. */
#define SmartCriticalSection(LockClassName) SmartLockCriticalSectionClass CONCAT(lock##_, __COUNTER__)(LockClassName, __FUNCTION__)

/** @brief Automatically disable interrupts and restore them when the scope ends. */
#define CriticalSection SmartCriticalSectionClass

#endif // __cplusplus
#endif // !__FENNIX_KERNEL_LOCK_H__
