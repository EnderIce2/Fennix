#include <lock.hpp>

#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

// #define PRINT_BACKTRACE

bool ForceUnlock = false;
Atomic<size_t> LocksCount = 0;

size_t GetLocksCount() { return LocksCount; }

void LockClass::DeadLock(SpinLockData Lock)
{
    if (ForceUnlock)
    {
        warn("Unlocking lock '%s' which it was held by '%s'...", Lock.AttemptingToGet, Lock.CurrentHolder);
        this->DeadLocks = 0;
        this->Unlock();
        return;
    }

    CPUData *CoreData = GetCurrentCPU();
    long CCore = 0xdead;
    if (CoreData != nullptr)
        CCore = CoreData->ID;

    warn("Potential deadlock in lock '%s' held by '%s'! %ld %s in queue. Interrupts are %s. Core %ld held by %ld. (%ld times happened)",
         Lock.AttemptingToGet, Lock.CurrentHolder,
         Lock.Count, Lock.Count > 1 ? "locks" : "lock",
         CPU::Interrupts(CPU::Check) ? "enabled" : "disabled",
         CCore, Lock.Core, this->DeadLocks);

#ifdef PRINT_BACKTRACE
    if (KernelSymbolTable)
    {
        debug("\t\t%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s",
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(1)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(2)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(3)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(4)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(5)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(6)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(7)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(8)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(9)));
    }
#endif

    // TODO: Print on screen too.

    this->DeadLocks++;

    if (Config.UnlockDeadLock && this->DeadLocks.Load() > 10)
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
    LockData.AttemptingToGet = FunctionName;
    LockData.StackPointerAttempt = (uintptr_t)__builtin_frame_address(0);

Retry:
    int i = 0;
    while (IsLocked.Exchange(true, MemoryOrder::Acquire) && ++i < (DebuggerIsAttached ? 0x100000 : 0x10000000))
        CPU::Pause();

    if (i >= (DebuggerIsAttached ? 0x100000 : 0x10000000))
    {
        DeadLock(LockData);
        goto Retry;
    }

    LockData.Count++;
    LockData.CurrentHolder = FunctionName;
    LockData.StackPointerHolder = (uintptr_t)__builtin_frame_address(0);

    CPUData *CoreData = GetCurrentCPU();
    if (CoreData != nullptr)
        LockData.Core = CoreData->ID;

    LocksCount++;

    __sync;
    return 0;
}

int LockClass::Unlock()
{
    __sync;

    IsLocked.Store(false, MemoryOrder::Release);
    LockData.Count--;
    LocksCount--;

    return 0;
}

void LockClass::TimeoutDeadLock(SpinLockData Lock, uint64_t Timeout)
{
    CPUData *CoreData = GetCurrentCPU();
    long CCore = 0xdead;

    if (CoreData != nullptr)
        CCore = CoreData->ID;

    uint64_t Counter = TimeManager->GetCounter();

    warn("Potential deadlock in lock '%s' held by '%s'! %ld %s in queue. Interrupts are %s. Core %ld held by %ld. Timeout in %ld (%ld ticks remaining).",
         Lock.AttemptingToGet, Lock.CurrentHolder,
         Lock.Count, Lock.Count > 1 ? "locks" : "lock",
         CPU::Interrupts(CPU::Check) ? "enabled" : "disabled",
         CCore, Lock.Core, Timeout, Timeout - Counter);

#ifdef PRINT_BACKTRACE
    if (KernelSymbolTable)
    {
        debug("\t\t%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s",
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(1)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(2)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(3)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(4)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(5)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(6)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(7)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(8)),
              KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(9)));
    }
#endif

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
    if (!TimeManager)
        return Lock(FunctionName);

    LockData.AttemptingToGet = FunctionName;
    LockData.StackPointerAttempt = (uintptr_t)__builtin_frame_address(0);

    Atomic<uint64_t> Target = 0;
Retry:
    int i = 0;
    while (IsLocked.Exchange(true, MemoryOrder::Acquire) && ++i < (DebuggerIsAttached ? 0x100000 : 0x10000000))
        CPU::Pause();

    if (i >= (DebuggerIsAttached ? 0x100000 : 0x10000000))
    {
        if (Target.Load() == 0)
            Target.Store(TimeManager->CalculateTarget(Timeout));
        TimeoutDeadLock(LockData, Target.Load());
        goto Retry;
    }

    LockData.Count++;
    LockData.CurrentHolder = FunctionName;
    LockData.StackPointerHolder = (uintptr_t)__builtin_frame_address(0);

    CPUData *CoreData = GetCurrentCPU();
    if (CoreData != nullptr)
        LockData.Core = CoreData->ID;

    LocksCount++;

    __sync;
    return 0;
}
