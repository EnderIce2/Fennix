#include <lock.hpp>

#include <debug.h>

extern "C" void DeadLockHandler(LockClass *Lock)
{
    warn("Potential deadlock in lock '%s' held by '%s'",
         Lock->GetLockData()->AttemptingToGet,
         Lock->GetLockData()->CurrentHolder);
    // TODO: Print on screen too.
}