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

#include <time.hpp>

#include <memory.hpp>
#include <debug.h>
#include <io.h>

#if defined(a64)
#include "../../Architecture/amd64/acpi.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

namespace Time
{
    bool HighPrecisionEventTimer::Sleep(uint64_t Duration, Units Unit)
    {
#if defined(a86)
        uint64_t Target = mminq(&((HPET *)hpet)->MainCounterValue) + (Duration * ConvertUnit(Unit)) / clk;
        while (mminq(&((HPET *)hpet)->MainCounterValue) < Target)
            CPU::Pause();
        return true;
#endif
    }

    uint64_t HighPrecisionEventTimer::GetCounter()
    {
#if defined(a86)
        return mminq(&((HPET *)hpet)->MainCounterValue);
#endif
    }

    uint64_t HighPrecisionEventTimer::CalculateTarget(uint64_t Target, Units Unit)
    {
#if defined(a86)
        return mminq(&((HPET *)hpet)->MainCounterValue) + (Target * ConvertUnit(Unit)) / clk;
#endif
    }

    uint64_t HighPrecisionEventTimer::GetMillisecondsSinceClassCreation()
    {
#if defined(a86)
        return (this->GetCounter() - this->ClassCreationTime) / (this->clk / this->ConvertUnit(Units::Milliseconds));
#endif
    }

    HighPrecisionEventTimer::HighPrecisionEventTimer(void *hpet)
    {
#if defined(a86)
        ACPI::ACPI::HPETHeader *HPET_HDR = (ACPI::ACPI::HPETHeader *)hpet;
        Memory::Virtual().Remap((void *)HPET_HDR->Address.Address,
                                (void *)HPET_HDR->Address.Address,
                                Memory::PTFlag::RW | Memory::PTFlag::PCD);
        this->hpet = (HPET *)HPET_HDR->Address.Address;
        trace("%s timer is at address %016p", HPET_HDR->Header.OEMID, (void *)HPET_HDR->Address.Address);
        clk = s_cst(uint32_t, this->hpet->GeneralCapabilities >> 32);
        mmoutq(&this->hpet->GeneralConfiguration, 0);
        mmoutq(&this->hpet->MainCounterValue, 0);
        mmoutq(&this->hpet->GeneralConfiguration, 1);
        ClassCreationTime = this->GetCounter();
#endif
    }

    HighPrecisionEventTimer::~HighPrecisionEventTimer()
    {
    }
}
