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

#ifndef __FENNIX_KERNEL_SMP_H__
#define __FENNIX_KERNEL_SMP_H__

#include <task.hpp>
#include <kexcept/cxxabi.h>
#include <types.h>
#include <atomic>

/** @brief Maximum supported number of CPU cores by the kernel */
#define MAX_CPU 255
#define CPU_DATA_CHECKSUM 0xC0FFEE

struct CPUArchData
{
#if defined(__amd64__)
	__aligned(16) CPU::x64::FXState FPU{};
#elif defined(__i386__)
	__aligned(16) CPU::x32::FXState FPU{};
#elif defined(__aarch64__)
#endif
};

struct CPUData
{
	/** Used by CPU */
	uintptr_t Stack;

	/** CPU ID. */
	int ID;

	/** Local CPU error code. */
	long ErrorCode;

	/** Current running process */
	std::atomic<Tasking::PCB *> CurrentProcess;

	/** Current running thread */
	std::atomic<Tasking::TCB *> CurrentThread;

	/** Exception information. */
	ExceptionInfo Exception;

	/** Architecture-specific data. */
	CPUArchData Data;

	/** Checksum. Used to verify the integrity of the data. Must be equal to CPU_DATA_CHECKSUM (0xC0FFEE). */
	int Checksum;

	/** Is CPU online? */
	bool IsActive;
} __aligned(16);

CPUData *GetCurrentCPU();
CPUData *GetCPU(long ID);

namespace SMP
{
	extern int CPUCores;
	void Initialize(void *madt);
}

#endif // !__FENNIX_KERNEL_SMP_H__
