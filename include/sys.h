#ifndef __FENNIX_KERNEL_SYSTEM_H__
#define __FENNIX_KERNEL_SYSTEM_H__

#include <types.h>
#include <cpu.hpp>

// TODO: Add actual panic screen
#define panic(msg)   \
    {                \
        error(msg);  \
        CPU::Stop(); \
    }

#endif // !__FENNIX_KERNEL_SYSTEM_H__
