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
        void *GetACPI() { return this->acpi; }
        void *GetDSDT() { return this->dsdt; }
        void *GetMADT() { return this->madt; }
        void Reboot();
        void Shutdown();
        Power();
        ~Power();
    };
}

#endif // !__FENNIX_KERNEL_POWER_H__
