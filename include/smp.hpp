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
#include <cxxabi.h>
#include <types.h>
#include <atomic>

/** @brief Maximum supported number of CPU cores by the kernel */
#define MAX_CPU 256
#define CPU_DATA_CHECKSUM 0xC0FFEE

struct CPUArchData
{
#if defined(a64)
    CPU::x64::FXState *FPU;
    /* TODO */
#elif defined(a32)
    CPU::x32::FXState *FPU;
    /* TODO */
#elif defined(aa64)
#endif
};

struct CPUData
{
    /** @brief Used by CPU */
    uintptr_t Stack;

    /** @brief CPU ID. */
    int ID;

    /** @brief Local CPU error code. */
    long ErrorCode;

    /** @brief Current running process */
    std::atomic<Tasking::PCB *> CurrentProcess;

    /** @brief Current running thread */
    std::atomic<Tasking::TCB *> CurrentThread;

    /** @brief Unwind data */
    __cxa_eh_globals EHGlobals;

    /** @brief Architecture-specific data. */
    CPUArchData Data;

    /** @brief Checksum. Used to verify the integrity of the data. Must be equal to CPU_DATA_CHECKSUM (0xC0FFEE). */
    int Checksum;

    /** @brief Is CPU online? */
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
