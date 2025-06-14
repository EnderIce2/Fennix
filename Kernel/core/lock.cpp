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

#include <lock.hpp>

#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

/* This might end up in a deadlock in the deadlock handler.
	Nobody can escape the deadlock, not even the
	deadlock handler itself. */
// #define PRINT_BACKTRACE 1

#ifdef PRINT_BACKTRACE
#pragma GCC diagnostic ignored "-Wframe-address"

void PrintStacktrace(LockClass::SpinLockData *Lock)
{
	if (!KernelSymbolTable)
	{
		warn("Symbol table not available.");
		return;
	}

	struct StackFrame
	{
		uintptr_t BasePointer;
		uintptr_t ReturnAddress;
	};

	// char DbgAttempt[1024] = "\0";
	// char DbgHolder[1024] = "\0";
	std::string DbgAttempt = "\0";
	std::string DbgHolder = "\0";

	StackFrame *FrameAttempt = (StackFrame *)Lock->StackPointerAttempt.load();
	StackFrame *FrameHolder = (StackFrame *)Lock->StackPointerHolder.load();

	while (Memory::Virtual().Check(FrameAttempt))
	{
		DbgAttempt.concat(KernelSymbolTable->GetSymbol(FrameAttempt->ReturnAddress));
		DbgAttempt.concat("<-");
		FrameAttempt = (StackFrame *)FrameAttempt->BasePointer;
	}
	warn("Attempt: %s", DbgAttempt.c_str());

	while (Memory::Virtual().Check(FrameHolder))
	{
		DbgHolder.concat(KernelSymbolTable->GetSymbol(FrameHolder->ReturnAddress));
		DbgHolder.concat("<-");
		FrameHolder = (StackFrame *)FrameHolder->BasePointer;
	}

	warn("Holder: %s", DbgHolder.c_str());

	// warn("\t\t%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s<-%s",
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(0)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(1)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(2)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(3)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(4)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(5)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(6)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(7)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(8)),
	// 	 KernelSymbolTable->GetSymbol((uintptr_t)__builtin_return_address(9)));
}
#endif

#ifdef DEBUG
#define DEADLOCK_TIMEOUT 0x1000
#else
#define DEADLOCK_TIMEOUT 0x10000000
#endif

bool ForceUnlock = false;
std::atomic_size_t LocksCount = 0;

size_t GetLocksCount() { return LocksCount.load(); }

void LockClass::Yield()
{
	if (CPU::Interrupts(CPU::Check) && TaskManager &&
		!TaskManager->IsPanic())
	{
		TaskManager->Yield();
	}

	CPU::Pause();
}

void LockClass::DeadLock(SpinLockData &Lock)
{
	if (ForceUnlock)
	{
		warn("Unlocking lock '%s' which it was held by '%s'...",
			 Lock.AttemptingToGet, Lock.CurrentHolder);
		this->DeadLocks = 0;
		this->Unlock();
		return;
	}

	CPUData *CoreData = GetCurrentCPU();
	long CCore = 0xdead;
	if (CoreData != nullptr)
		CCore = CoreData->ID;

	warn("Potential deadlock in lock '%s' held by '%s'! %ld %s in queue. Interrupts are %s. Core %ld held by %ld. (%ld times happened)",
		 Lock.AttemptingToGet, Lock.CurrentHolder, Lock.Count, Lock.Count > 1 ? "locks" : "lock",
		 CPU::Interrupts(CPU::Check) ? "enabled" : "disabled", CCore, Lock.Core, this->DeadLocks);

#ifdef PRINT_BACKTRACE
	PrintStacktrace(&Lock);
#endif

	// TODO: Print on screen too.

	this->DeadLocks++;

	if (Config.UnlockDeadLock && this->DeadLocks.load() > 10)
	{
		warn("Unlocking lock '%s' to prevent deadlock. (this is enabled in the kernel config)", Lock.AttemptingToGet);
		this->DeadLocks = 0;
		this->Unlock();
	}

	this->Yield();
}

int LockClass::Lock(const char *FunctionName)
{
	LockData.AttemptingToGet = FunctionName;
	LockData.StackPointerAttempt = (uintptr_t)__builtin_frame_address(0);

Retry:
	int i = 0;
	while (IsLocked.exchange(true, std::memory_order_acquire) &&
		   ++i < DEADLOCK_TIMEOUT)
	{
		this->Yield();
	}

	if (i >= DEADLOCK_TIMEOUT)
	{
		DeadLock(LockData);
		goto Retry;
	}

	LockData.Count.fetch_add(1);
	LockData.CurrentHolder.store(FunctionName);
	LockData.StackPointerHolder.store((uintptr_t)__builtin_frame_address(0));

	CPUData *CoreData = GetCurrentCPU();
	if (CoreData != nullptr)
		LockData.Core.store(CoreData->ID);

	LocksCount.fetch_add(1);

	__sync;
	return 0;
}

int LockClass::Unlock()
{
	__sync;

	IsLocked.store(false, std::memory_order_release);
	LockData.Count.fetch_sub(1);
	LocksCount.fetch_sub(1);

	return 0;
}

void LockClass::TimeoutDeadLock(SpinLockData &Lock, uint64_t Timeout)
{
	CPUData *CoreData = GetCurrentCPU();
	long CCore = 0xdead;

	if (CoreData != nullptr)
		CCore = CoreData->ID;

	uint64_t Counter = TimeManager->GetTimeNs();

	warn("Potential deadlock in lock '%s' held by '%s'! %ld %s in queue. Interrupts are %s. Core %ld held by %ld. Timeout in %ld (%ld ticks remaining).",
		 Lock.AttemptingToGet, Lock.CurrentHolder, Lock.Count, Lock.Count > 1 ? "locks" : "lock",
		 CPU::Interrupts(CPU::Check) ? "enabled" : "disabled", CCore, Lock.Core, Timeout, Timeout - Counter);

#ifdef PRINT_BACKTRACE
	PrintStacktrace(&Lock);
#endif

	if (Timeout < Counter)
	{
		warn("Unlocking lock '%s' because of timeout. (%ld < %ld)",
			 Lock.AttemptingToGet, Timeout, Counter);
		this->Unlock();
	}

	this->Yield();
}

int LockClass::TimeoutLock(const char *FunctionName, uint64_t Timeout)
{
	if (!TimeManager)
		return Lock(FunctionName);

	LockData.AttemptingToGet.store(FunctionName);
	LockData.StackPointerAttempt.store((uintptr_t)__builtin_frame_address(0));

	std::atomic_uint64_t Target = 0;
Retry:
	int i = 0;
	while (IsLocked.exchange(true, std::memory_order_acquire) &&
		   ++i < DEADLOCK_TIMEOUT)
	{
		this->Yield();
	}

	if (i >= DEADLOCK_TIMEOUT)
	{
		if (Target.load() == 0)
			Target.store(TimeManager->GetTimeNs() + Timeout);
		TimeoutDeadLock(LockData, Target.load());
		goto Retry;
	}

	LockData.Count.fetch_add(1);
	LockData.CurrentHolder.store(FunctionName);
	LockData.StackPointerHolder.store((uintptr_t)__builtin_frame_address(0));

	CPUData *CoreData = GetCurrentCPU();
	if (CoreData != nullptr)
		LockData.Core.store(CoreData->ID);

	LocksCount.fetch_add(1);

	__sync;
	return 0;
}
