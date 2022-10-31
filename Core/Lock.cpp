#include <lock.hpp>

#include <debug.h>

static unsigned long DeadLocks = 0;

extern "C" void DeadLockHandler(LockClass *Lock)
{
    warn("Potential deadlock in lock '%s' held by '%s' (%ld)",
         Lock->GetLockData()->AttemptingToGet,
         Lock->GetLockData()->CurrentHolder,
         DeadLocks++);
    // TODO: Print on screen too.
}
