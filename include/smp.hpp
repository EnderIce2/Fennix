#ifndef __FENNIX_KERNEL_SMP_H__
#define __FENNIX_KERNEL_SMP_H__

#include <types.h>
#include <atomic.hpp>
#include <task.hpp>

/** @brief Maximum supported number of CPU cores by the kernel */
#define MAX_CPU 256
#define CPU_DATA_CHECKSUM 0xC0FFEE

struct CPUArchData
{
#if defined(__amd64__)
    CPU::x64::FXState *FPU;
    /* TODO */
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
};

struct CPUData
{
    /** @brief Used by syscall handler */
    uint8_t *SystemCallStack; /* gs+0x0 */
    /** @brief Used by syscall handler */
    uintptr_t TempStack; /* gs+0x8 */
    /** @brief Used by CPU */
    uintptr_t Stack;
    /** @brief CPU ID. */
    long ID;
    /** @brief Local CPU error code. */
    long ErrorCode;
    /** @brief Is CPU online? */
    bool IsActive;

    /** @brief Current running process */
    Atomic<Tasking::PCB *> CurrentProcess;
    /** @brief Current running thread */
    Atomic<Tasking::TCB *> CurrentThread;

    /** @brief Architecture-specific data. */
    CPUArchData Data;
    /** @brief Checksum. Used to verify the integrity of the data. Must be equal to CPU_DATA_CHECKSUM (0xC0FFEE). */
    int Checksum;
};

CPUData *GetCurrentCPU();
CPUData *GetCPU(long ID);

namespace SMP
{
    extern int CPUCores;
    void Initialize(void *madt);
}

#endif // !__FENNIX_KERNEL_SMP_H__
