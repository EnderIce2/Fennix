#ifndef __FENNIX_KERNEL_CRASH_HANDELR_H__
#define __FENNIX_KERNEL_CRASH_HANDELR_H__

#include <types.h>

#include <interrupts.hpp>
#include <cpu.hpp>

namespace CrashHandler
{
    extern uintptr_t PageFaultAddress;
    extern void *EHIntFrames[INT_FRAMES_MAX];

    void EHPrint(const char *Format, ...);
    void Handle(void *Data);
}

#endif // !__FENNIX_KERNEL_CRASH_HANDELR_H__
