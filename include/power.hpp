#ifndef __FENNIX_KERNEL_POWER_H__
#define __FENNIX_KERNEL_POWER_H__

#include <types.h>

namespace Power
{
    class Power
    {
    private:
        // specific for 64 and 32 bit
        void *acpi;
        void *dsdt;
        void *madt;

    public:
        void Reboot();
        void Shutdown();
        Power();
        ~Power();
    };
}

#endif // !__FENNIX_KERNEL_POWER_H__
