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

#pragma once

#include <task.hpp>
#include <kexcept/cxxabi.h>
#include <time.hpp>
#include <types.h>
#include <log.hpp>
#include <atomic>

/** @brief Maximum supported number of CPU cores by the kernel */
#define MAX_CPU 255

struct CPUArchData
{
#if defined(__amd64__)
	__aligned(16) CPU::x64::FXState FPU {};
#elif defined(__i386__)
	__aligned(16) CPU::x32::FXState FPU {};
#elif defined(__aarch64__)
#endif
};

struct CPUData
{
	/** Used by CPU */
	uintptr_t Stack = 0;

	/** CPU ID. */
	int ID = -1;

	/** Local CPU error code. */
	long ErrorCode = 0;

	/** Current running process */
	std::atomic<Tasking::PCB *> CurrentProcess = nullptr;

	/** Current running thread */
	std::atomic<Tasking::TCB *> CurrentThread = nullptr;

	/** Exception information. */
	ExceptionInfo Exception{};

	/** Architecture-specific data. */
	CPUArchData Data;

	Log::LogRecord *LogRecords = nullptr;
	size_t LogEntries = 0;
	std::atomic_uint32_t LogWriteIndex = 0;

	Time::ClockSource *Clock = nullptr;
	Time::TimerDevice *Timer = nullptr;

	/** Is CPU online? */
	bool IsActive = false;
} __aligned(16);

CPUData *GetCurrentCPU();
CPUData *GetCPU(int ID);

/** Current CPU */
#define thisCPU GetCurrentCPU()
/** Local CPU Clock */
#define thisClock thisCPU->Clock
/** Local CPU Timer */
#define thisTimer thisCPU->Timer

namespace SMP
{
	extern int CPUCores;
	void Initialize();
}
