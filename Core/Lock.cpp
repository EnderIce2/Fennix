#include <lock.hpp>

#include <debug.h>

#include "../kernel.h"

static unsigned long DeadLocks = 0;

extern "C" void DeadLockHandler(LockClass *Lock)
{
    warn("Potential deadlock in lock '%s' held by '%s' (%ld)",
         Lock->GetLockData()->AttemptingToGet,
         Lock->GetLockData()->CurrentHolder,
         DeadLocks++);

    if (TaskManager)
        TaskManager->Schedule();

    // TODO: Print on screen too.
}
