#ifndef __FENNIX_KERNEL_POWER_H__
#define __FENNIX_KERNEL_POWER_H__

#include <types.h>

namespace Power
{
    class Power
    {
    private:
        void *acpi = nullptr;
        void *dsdt = nullptr;
        void *madt = nullptr;

    public:
        /**
         * @brief Get Advanced Configuration and Power Interface. (Available only on x32 and x64)
         *
         * @return void*
         */
        void *GetACPI() { return this->acpi; }

        /**
         * @brief Get Differentiated System Description Table. (Available only on x32 and x64)
         *
         * @return void*
         */
        void *GetDSDT() { return this->dsdt; }

        /**
         * @brief Get Multiple APIC Description Table. (Available only on x32 and x64)
         *
         * @return void*
         */
        void *GetMADT() { return this->madt; }

        /**
         * @brief Reboot the system.
         */
        void Reboot();

        /**
         * @brief Shutdown the system.
         */
        void Shutdown();

        Power();
        ~Power();
    };
}

#endif // !__FENNIX_KERNEL_POWER_H__
