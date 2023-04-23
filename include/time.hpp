/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_KERNEL_TIME_H__
#define __FENNIX_KERNEL_TIME_H__

#include <types.h>
#include <debug.h>

namespace Time
{
    struct Clock
    {
        int Year, Month, Day, Hour, Minute, Second;
        uint64_t Counter;
    };

    Clock ReadClock();
    Clock ConvertFromUnix(int Timestamp);

    enum Units
    {
        Femtoseconds,
        Picoseconds,
        Nanoseconds,
        Microseconds,
        Milliseconds,
        Seconds,
        Minutes,
        Hours,
        Days,
        Months,
        Years
    };

    class HighPrecisionEventTimer
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

        uint32_t clk = 0;
        HPET *hpet;

        uint64_t ConvertUnit(Units Unit)
        {
            switch (Unit)
            {
            case Femtoseconds:
                return 1;
            case Picoseconds:
                return 1000;
            case Nanoseconds:
                return 1000000;
            case Microseconds:
                return 1000000000;
            case Milliseconds:
                return 1000000000000;
            case Seconds:
                return 1000000000000000;
            case Minutes:
                return 1000000000000000000;
            // case Hours:
            //     return 1000000000000000000000;
            // case Days:
            //     return 1000000000000000000000000;
            // case Months:
            //     return 1000000000000000000000000000;
            // case Years:
            //     return 1000000000000000000000000000000;
            default:
                error("Invalid time unit %d", Unit);
                return 1;
            }
        }

    public:
        bool Sleep(uint64_t Duration, Units Unit);
        uint64_t GetCounter();
        uint64_t CalculateTarget(uint64_t Target, Units Unit);

        HighPrecisionEventTimer(void *hpet);
        ~HighPrecisionEventTimer();
    };

    class time
    {
    private:
        enum _ActiveTimer
        {
            NONE,
            RTC,
            PIT,
            HPET,
            ACPI,
            APIC,
            TSC,
        } ActiveTimer = NONE;

        HighPrecisionEventTimer *hpet;

    public:
        bool Sleep(uint64_t Duration, Units Unit);
        uint64_t GetCounter();
        uint64_t CalculateTarget(uint64_t Target, Units Unit);
        time(void *acpi);
        ~time();
    };
}

#endif // !__FENNIX_KERNEL_TIME_H__
