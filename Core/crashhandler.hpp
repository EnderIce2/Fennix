#ifndef __FENNIX_KERNEL_CRASH_HANDELR_H__
#define __FENNIX_KERNEL_CRASH_HANDELR_H__

#include <types.h>
#include <cpu.hpp>

namespace CrashHandler
{
    void EHPrint(const char *Format, ...);
    void Handle(void *Data);
}

#endif // !__FENNIX_KERNEL_CRASH_HANDELR_H__
