#ifndef __FENNIX_KERNEL_RANDOM_H__
#define __FENNIX_KERNEL_RANDOM_H__

#include <types.h>

namespace Random
{
    uint16_t rand16();
    uint32_t rand32();
    uint64_t rand64();
    void ChangeSeed(uint64_t CustomSeed);
}

#endif // !__FENNIX_KERNEL_RANDOM_H__
