#ifndef __FENNIX_KERNEL_IDT_H__
#define __FENNIX_KERNEL_IDT_H__

#include <types.h>

namespace InterruptDescriptorTable
{
    void Init(int Core);
}

#endif // !__FENNIX_KERNEL_IDT_H__
