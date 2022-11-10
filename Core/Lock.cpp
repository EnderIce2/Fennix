#include <lock.hpp>

#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

static unsigned long DeadLocks = 0;

extern "C" void DeadLockHandler(LockClass *Lock)
{
    CPUData *CoreData = GetCurrentCPU();
    long CCore = 0xdead;
    if (CoreData != nullptr)
        CCore = CoreData->ID;
    warn("Potential deadlock in lock '%s' held by '%s' (%ld) [%#lx-%ld] [%ld->%ld]",
         Lock->GetLockData()->AttemptingToGet,
         Lock->GetLockData()->CurrentHolder,
         DeadLocks++,
         Lock->GetLockData()->LockData,
         Lock->GetLockData()->Count,
         CCore,
         Lock->GetLockData()->Core);

    if (TaskManager)
        TaskManager->Schedule();

    // TODO: Print on screen too.
}

int LockClass::Lock(const char *FunctionName)
{
    LockData.AttemptingToGet = FunctionName;
    SpinLock_Lock(&LockData.LockData);
    LockData.CurrentHolder = FunctionName;
    LockData.Count++;
    CPUData *CoreData = GetCurrentCPU();
    if (CoreData != nullptr)
        LockData.Core = CoreData->ID;
    CPU::MemBar::Barrier();

    // while (!__sync_bool_compare_and_swap(&IsLocked, false, true))
    // CPU::Pause();
    // __sync_synchronize();
    return 0;
}

int LockClass::Unlock()
{
    SpinLock_Unlock(&LockData.LockData);
    LockData.Count--;
    CPU::MemBar::Barrier();

    // __sync_synchronize();
    // __atomic_store_n(&IsLocked, false, __ATOMIC_SEQ_CST);
    // IsLocked = false;
    return 0;
}
