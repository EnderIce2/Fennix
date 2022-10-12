#ifndef __FENNIX_KERNEL_SMP_H__
#define __FENNIX_KERNEL_SMP_H__

#include <types.h>

/** @brief Maximum supported number of CPU cores by the kernel */
#define MAX_CPU 256

#define CPU_DATA_CHECKSUM 0xC0FFEE

struct CPUData
{
    /**
     * @brief CPU ID.
     */
    uint64_t ID;
    /**
     * @brief Local CPU error code.
     */
    long ErrorCode;
    /**
     * @brief Is CPU online?
     */
    bool IsActive;
    /**
     * @brief Architecture-specific CPU data.
     */
    void *Data;
    /**
     * @brief Checksum. Used to verify the integrity of the data. Must be equal to CPU_DATA_CHECKSUM (0xC0FFEE).
     */
    int Checksum;
} __attribute__((packed));

CPUData *GetCurrentCPU();
CPUData *GetCPU(uint64_t ID);

namespace SMP
{
    void Initialize(void *madt);
}

#endif // !__FENNIX_KERNEL_SMP_H__
