#ifndef __FENNIX_KERNEL_GDT_H__
#define __FENNIX_KERNEL_GDT_H__

#include <types.h>

namespace GlobalDescriptorTable
{
    void Init(int Core);
}

#endif // !__FENNIX_KERNEL_GDT_H__
