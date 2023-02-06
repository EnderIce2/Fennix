#include <lock.hpp>

#include <debug.h>
#include <smp.hpp>
#include <atomic.hpp>

#include "../kernel.h"

void LockClass::DeadLock(SpinLockData Lock)
{
    CPUData *CoreData = GetCurrentCPU();
    long CCore = 0xdead;
    if (CoreData != nullptr)
        CCore = CoreData->ID;
    warn("Potential deadlock in lock '%s' held by '%s'! %ld locks in queue. Core %ld is being held by %ld. (%ld times happened)",
         Lock.AttemptingToGet, Lock.CurrentHolder,
         Lock.Count, CCore, Lock.Core,
         this->DeadLocks);

    // TODO: Print on screen too.

    this->DeadLocks++;

    if (Config.UnlockDeadLock && this->DeadLocks > 10)
    {
        warn("Unlocking lock '%s' to prevent deadlock. (this is enabled in the kernel config)", Lock.AttemptingToGet);
        this->DeadLocks = 0;
        this->Unlock();
    }

    if (TaskManager)
        TaskManager->Schedule();
}

int LockClass::Lock(const char *FunctionName)
{
    // LockData.AttemptingToGet = FunctionName;
    // SpinLock_Lock(&LockData.LockData);
    // LockData.Count++;
    // LockData.CurrentHolder = FunctionName;
    // CPUData *CoreData = GetCurrentCPU();
    // if (CoreData != nullptr)
    // LockData.Core = CoreData->ID;
    // __sync_synchronize();

    // while (!__sync_bool_compare_and_swap(&IsLocked, false, true))
    //     CPU::Pause();
    // __sync_synchronize();

    LockData.AttemptingToGet = FunctionName;
Retry:
    unsigned int i = 0;
    while (__atomic_exchange_n(&IsLocked, true, __ATOMIC_ACQUIRE) && ++i < 0x10000000)
        CPU::Pause();
    if (i >= 0x10000000)
    {
        DeadLock(LockData);
        goto Retry;
    }
    LockData.Count++;
    LockData.CurrentHolder = FunctionName;
    CPUData *CoreData = GetCurrentCPU();
    if (CoreData != nullptr)
        LockData.Core = CoreData->ID;
    __sync_synchronize();

    return 0;
}

int LockClass::Unlock()
{
    // SpinLock_Unlock(&LockData.LockData);
    // LockData.Count--;
    // __sync_synchronize();

    // __sync_synchronize();
    // __atomic_store_n(&IsLocked, false, __ATOMIC_SEQ_CST);
    // IsLocked = false;

    __sync_synchronize();
    __atomic_store_n(&IsLocked, false, __ATOMIC_RELEASE);
    LockData.Count--;
    IsLocked = false;

    return 0;
}

void LockClass::TimeoutDeadLock(SpinLockData Lock, uint64_t Timeout)
{
    CPUData *CoreData = GetCurrentCPU();
    long CCore = 0xdead;
    if (CoreData != nullptr)
        CCore = CoreData->ID;
    warn("Potential deadlock in lock '%s' held by '%s'! %ld locks in queue. Core %ld is being held by %ld. Timeout in %ld",
         Lock.AttemptingToGet, Lock.CurrentHolder,
         Lock.Count, CCore, Lock.Core,
         Timeout);

    // TODO: Print on screen too.

    uint64_t Counter = TimeManager->GetCounter();
    if (Timeout < Counter)
    {
        warn("Unlocking lock '%s' because of timeout. (%ld < %ld)", Lock.AttemptingToGet, Timeout, Counter);
        this->Unlock();
    }

    if (TaskManager)
        TaskManager->Schedule();
}

int LockClass::TimeoutLock(const char *FunctionName, uint64_t Timeout)
{
    LockData.AttemptingToGet = FunctionName;
    Atomic<uint64_t> Target = 0;
Retry:
    unsigned int i = 0;
    while (__atomic_exchange_n(&IsLocked, true, __ATOMIC_ACQUIRE) && ++i < 0x10000000)
        CPU::Pause();
    if (i >= 0x10000000)
    {
        if (Target.Load() == 0)
            Target = TimeManager->CalculateTarget(Timeout);
        TimeoutDeadLock(LockData, Target.Load());
        goto Retry;
    }
    LockData.Count++;
    LockData.CurrentHolder = FunctionName;
    CPUData *CoreData = GetCurrentCPU();
    if (CoreData != nullptr)
        LockData.Core = CoreData->ID;
    __sync_synchronize();

    return 0;
}
