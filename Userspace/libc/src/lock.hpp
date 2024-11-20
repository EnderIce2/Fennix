#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

#include <types.h>

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
	};
	void DeadLock(SpinLockData Lock);

private:
	SpinLockData LockData;
	bool IsLocked = false;

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

/** @brief Create a new lock (can be used with SmartCriticalSection). */
#define NewLock(Name) LockClass Name
/** @brief Simple lock that is automatically released when the scope ends. */
#define SmartLock(LockClassName) SmartLockClass CONCAT(lock##_, __COUNTER__)(LockClassName, __FUNCTION__)

#endif // __cplusplus
#endif // !__FENNIX_KERNEL_LOCK_H__
