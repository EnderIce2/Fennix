#include <smp.hpp>

#include <ints.hpp>
#include <memory.hpp>
#include <assert.h>
#include <cpu.hpp>

#include "../../../kernel.h"

volatile bool CPUEnabled = false;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static __attribute__((aligned(PAGE_SIZE))) CPUData CPUs[MAX_CPU] = {0};

CPUData *GetCPU(uint64_t id) { return &CPUs[id]; }

CPUData *GetCurrentCPU()
{
    uint64_t ret = 0;
    if (!(&CPUs[ret])->IsActive)
    {
        error("CPU %d is not active!", ret);
        return &CPUs[0];
    }
    assert((&CPUs[ret])->Checksum == CPU_DATA_CHECKSUM);
    return &CPUs[ret];
}

namespace SMP
{
    int CPUCores = 0;

    void Initialize(void *madt)
    {
        fixme("SMP::Initialize() is not implemented!");
    }
}
