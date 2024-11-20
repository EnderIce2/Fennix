#ifndef __FENNIX_KERNEL_TIME_H__
#define __FENNIX_KERNEL_TIME_H__

#include <types.h>

namespace Time
{
    struct Clock
    {
        uint64_t Year, Month, Day, Hour, Minute, Second;
        uint64_t Counter;
    };

    Clock ReadClock();

    class time
    {
    private:
        struct HPET
        {
            uint64_t GeneralCapabilities;
            uint64_t Reserved0;
            uint64_t GeneralConfiguration;
            uint64_t Reserved1;
            uint64_t GeneralIntStatus;
            uint64_t Reserved2;
            uint64_t Reserved3[24];
            uint64_t MainCounterValue;
            uint64_t Reserved4;
        };

        void *acpi;
        void *hpet;
        uint32_t clk = 0;

    public:
        void Sleep(uint64_t Milliseconds);
        time(void *acpi);
        ~time();
    };
}

#endif // !__FENNIX_KERNEL_TIME_H__
