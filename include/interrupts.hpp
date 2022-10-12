#ifndef __FENNIX_KERNEL_INTERRUPTS_H__
#define __FENNIX_KERNEL_INTERRUPTS_H__

#include <types.h>

namespace Interrupts
{
    extern void *apic;
    void Initialize(int Core);
    void Enable();
}

#endif // !__FENNIX_KERNEL_INTERRUPTS_H__
