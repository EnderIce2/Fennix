#ifndef __FENNIX_KERNEL_CRASH_HANDLER_H__
#define __FENNIX_KERNEL_CRASH_HANDLER_H__

#include <types.h>

#include <ints.hpp>
#include <cpu.hpp>

namespace CrashHandler
{
    extern uintptr_t PageFaultAddress;
    extern void *EHIntFrames[INT_FRAMES_MAX];

    void EHPrint(const char *Format, ...);
    void Handle(void *Data);
}

#endif // !__FENNIX_KERNEL_CRASH_HANDLER_H__
