#include "lock.hpp"
#include <stdio.h>

void LockClass::DeadLock(SpinLockData Lock)
{
	fprintf(stdout, "Potential deadlock in lock '%s' held by '%s'! %ld locks in queue.",
			Lock.AttemptingToGet, Lock.CurrentHolder, Lock.Count);
}

int LockClass::Lock(const char *FunctionName)
{
	LockData.AttemptingToGet = FunctionName;
Retry:
	unsigned int i = 0;
	while (__atomic_exchange_n(&IsLocked, true, __ATOMIC_ACQUIRE) && ++i < 0x10000000)
		;
	if (i >= 0x10000000)
	{
		DeadLock(LockData);
		goto Retry;
	}
	LockData.Count++;
	LockData.CurrentHolder = FunctionName;
	__sync_synchronize();
	return 0;
}

int LockClass::Unlock()
{
	__sync_synchronize();
	__atomic_store_n(&IsLocked, false, __ATOMIC_RELEASE);
	LockData.Count--;
	IsLocked = false;
	return 0;
}
