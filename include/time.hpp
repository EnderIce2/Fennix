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

namespace Time
{
    struct Clock
    {
        int Year, Month, Day, Hour, Minute, Second;
        uint64_t Counter;
    };

    Clock ReadClock();
    Clock ConvertFromUnix(int Timestamp);

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
        uint64_t GetCounter();
        uint64_t CalculateTarget(uint64_t Milliseconds);
        time(void *acpi);
        ~time();
    };
}

#endif // !__FENNIX_KERNEL_TIME_H__
