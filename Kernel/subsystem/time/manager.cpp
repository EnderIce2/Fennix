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
	void Manager::CheckActiveTimer()
	{
		if (unlikely(Timers[ActiveTimer]->IsAvailable() == false))
		{
			for (size_t i = Timers.size(); i-- > 0;)
			{
				if (Timers[i]->IsAvailable() == false)
					continue;
				ActiveTimer = i;
				break;
			}
		}
	}

	bool Manager::Sleep(size_t Nanoseconds)
	{
		if (unlikely(Timers.empty()))
			return false;

		this->CheckActiveTimer();
		debug("sleep for %d ns in timer %s", Nanoseconds, Timers[ActiveTimer]->Name());
		return Timers[ActiveTimer]->Sleep(Nanoseconds);
	}

	uint64_t Manager::GetTimeNs()
	{
		if (unlikely(Timers.empty()))
			return 0;

		this->CheckActiveTimer();
		return Timers[ActiveTimer]->GetNanoseconds();
	}

	const char *Manager::GetActiveTimerName()
	{
		if (unlikely(Timers.empty()))
			return "\0";

		this->CheckActiveTimer();
		return Timers[ActiveTimer]->Name();
	}

	void Manager::InitializeTimers()
	{
#if defined(__amd64__) || defined(__i386__)
		/* TODO: RTC check */
		/* TODO: PIT check */

		if (acpi)
		{
			if (((ACPI::ACPI *)acpi)->HPET)
			{
				ITimer *hpet = new HighPrecisionEventTimer(((ACPI::ACPI *)acpi)->HPET);
				ActiveTimer = Timers.size();
				Timers.push_back(hpet);
			}

			/* TODO: ACPI check */
			/* TODO: APIC check */
		}
		else
		{
			KPrint("\x1b[33mACPI not available");
		}

		ITimer *tsc = new TimeStampCounter;
		ActiveTimer = Timers.size();
		Timers.push_back(tsc);

		ITimer *kvmclock = new KVMClock;
		ActiveTimer = Timers.size();
		Timers.push_back(kvmclock);
#endif

		assert(Timers.empty() == false);
	}

	Manager::Manager(void *_acpi) : acpi(_acpi) {}
}
