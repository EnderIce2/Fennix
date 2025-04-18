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
#include <acpi.hpp>
#include <debug.h>
#include <io.h>


#include "../../kernel.h"

namespace Time
{
	bool time::Sleep(size_t Duration, Units Unit)
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
			return this->hpet->Sleep(Duration, Unit);
		case ACPI:
			fixme("ACPI sleep not implemented");
			return false;
		case APIC:
			fixme("APIC sleep not implemented");
			return false;
		case TSC:
			return this->tsc->Sleep(Duration, Unit);
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
			return 0;
		case RTC:
			fixme("RTC sleep not implemented");
			return 0;
		case PIT:
			fixme("PIT sleep not implemented");
			return 0;
		case HPET:
			return this->hpet->GetCounter();
		case ACPI:
			fixme("ACPI sleep not implemented");
			return 0;
		case APIC:
			fixme("APIC sleep not implemented");
			return 0;
		case TSC:
			return this->tsc->GetCounter();
		default:
			error("Unknown timer");
			return 0;
		}
	}

	uint64_t time::CalculateTarget(uint64_t Target, Units Unit)
	{
		switch (ActiveTimer)
		{
		case NONE:
			error("No timer is active");
			return 0;
		case RTC:
			fixme("RTC sleep not implemented");
			return 0;
		case PIT:
			fixme("PIT sleep not implemented");
			return 0;
		case HPET:
			return this->hpet->CalculateTarget(Target, Unit);
		case ACPI:
			fixme("ACPI sleep not implemented");
			return 0;
		case APIC:
			fixme("APIC sleep not implemented");
			return 0;
		case TSC:
			return this->tsc->CalculateTarget(Target, Unit);
		default:
			error("Unknown timer");
			return 0;
		}
	}

	uint64_t time::GetNanosecondsSinceClassCreation()
	{
		switch (ActiveTimer)
		{
		case NONE:
			error("No timer is active");
			return 0;
		case RTC:
			fixme("RTC sleep not implemented");
			return 0;
		case PIT:
			fixme("PIT sleep not implemented");
			return 0;
		case HPET:
			return this->hpet->GetNanosecondsSinceClassCreation();
		case ACPI:
			fixme("ACPI sleep not implemented");
			return 0;
		case APIC:
			fixme("APIC sleep not implemented");
			return 0;
		case TSC:
			return this->tsc->GetNanosecondsSinceClassCreation();
		default:
			error("Unknown timer");
			return 0;
		}
	}

	void time::FindTimers(void *acpi)
	{
#if defined(__amd64__) || defined(__i386__)
		/* TODO: RTC check */
		/* TODO: PIT check */

		if (acpi)
		{
			if (((ACPI::ACPI *)acpi)->HPET)
			{
				hpet = new HighPrecisionEventTimer(((ACPI::ACPI *)acpi)->HPET);
				ActiveTimer = HPET;
				SupportedTimers |= HPET;
				KPrint("HPET found");
			}
			else
			{
				KPrint("\x1b[33mHPET not found");
			}

			/* TODO: ACPI check */
			/* TODO: APIC check */
		}
		else
		{
			KPrint("\x1b[33mACPI not found");
		}

		bool TSCInvariant = false;
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x80000007 cpuid80000007;
			if (cpuid80000007.EDX.TscInvariant)
				TSCInvariant = true;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			// TODO: Intel 0x80000007
			CPU::x86::AMD::CPUID0x80000007 cpuid80000007;
			if (cpuid80000007.EDX.TscInvariant)
				TSCInvariant = true;
		}

		if (TSCInvariant)
		{
			tsc = new TimeStampCounter;
			// FIXME: ActiveTimer = TSC;
			SupportedTimers |= TSC;
			KPrint("Invariant TSC found");
		}
		else
			KPrint("\x1b[33mTSC is not invariant");
#endif
	}

	time::time()
	{
	}

	time::~time()
	{
	}
}
