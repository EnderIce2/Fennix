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
    bool time::Sleep(uint64_t Duration, Units Unit)
    {
        switch (ActiveTimer)
        {
        case NONE:
            error("No timer is active");
            return false;
        case RTC:
            fixme("RTC sleep not implemented");
            return false;
        case PIT:
            fixme("PIT sleep not implemented");
            return false;
        case HPET:
            return hpet->Sleep(Duration, Unit);
        case ACPI:
            fixme("ACPI sleep not implemented");
            return false;
        case APIC:
            fixme("APIC sleep not implemented");
            return false;
        case TSC:
            fixme("TSC sleep not implemented");
            return false;
        default:
            error("Unknown timer");
            return false;
        }
    }

    uint64_t time::GetCounter()
    {
        switch (ActiveTimer)
        {
        case NONE:
            error("No timer is active");
            return false;
        case RTC:
            fixme("RTC sleep not implemented");
            return false;
        case PIT:
            fixme("PIT sleep not implemented");
            return false;
        case HPET:
            return hpet->GetCounter();
        case ACPI:
            fixme("ACPI sleep not implemented");
            return false;
        case APIC:
            fixme("APIC sleep not implemented");
            return false;
        case TSC:
            fixme("TSC sleep not implemented");
            return false;
        default:
            error("Unknown timer");
            return false;
        }
    }

    uint64_t time::CalculateTarget(uint64_t Target, Units Unit)
    {
        switch (ActiveTimer)
        {
        case NONE:
            error("No timer is active");
            return false;
        case RTC:
            fixme("RTC sleep not implemented");
            return false;
        case PIT:
            fixme("PIT sleep not implemented");
            return false;
        case HPET:
            return hpet->CalculateTarget(Target, Unit);
        case ACPI:
            fixme("ACPI sleep not implemented");
            return false;
        case APIC:
            fixme("APIC sleep not implemented");
            return false;
        case TSC:
            fixme("TSC sleep not implemented");
            return false;
        default:
            error("Unknown timer");
            return false;
        }
    }

    time::time(void *acpi)
    {
        /* TODO: RTC check */
        /* TODO: PIT check */

        if (acpi)
        {
            if (((ACPI::ACPI *)acpi)->HPET)
            {
                hpet = new HighPrecisionEventTimer(((ACPI::ACPI *)acpi)->HPET);
                ActiveTimer = HPET;
            }

            /* TODO: ACPI check */
            /* TODO: APIC check */
        }

        /* TODO: TSC check */
    }

    time::~time()
    {
        debug("Destructor called");
    }
}
