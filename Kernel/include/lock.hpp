/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_LOCK_H__
#define __FENNIX_KERNEL_LOCK_H__

#include <types.h>

#include <cpu.hpp>
#include <atomic>

#ifdef __cplusplus

/* Enabled ONLY on crash. */
extern bool ForceUnlock;

/**
 * @brief Get how many locks are currently in use.
 *
 * @return size_t
 */
size_t GetLocksCount();

/** @brief Please use this macro to create a new lock. */
class LockClass
{
    struct SpinLockData
    {
        std::atomic_uint64_t LockData = 0x0;
        std::atomic<const char *> CurrentHolder = "(nul)";
        std::atomic<const char *> AttemptingToGet = "(nul)";
        std::atomic_uintptr_t StackPointerHolder = 0;
        std::atomic_uintptr_t StackPointerAttempt = 0;
        std::atomic_size_t Count = 0;
        std::atomic_long Core = 0;
    };

private:
    SpinLockData LockData;
    std::atomic_bool IsLocked = false;
    std::atomic_ulong DeadLocks = 0;

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
