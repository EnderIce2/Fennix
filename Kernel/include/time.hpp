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
#include <cassert>

namespace Time
{
	struct Clock
	{
		int Year, Month, Day, Hour, Minute, Second;
		size_t Counter;
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

	/** @deprecated this shouldn't be used */
	inline uint64_t ConvertUnit(const Units Unit)
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
			assert(!"Invalid time unit");
		}
	}

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
		HPET *hpet = nullptr;
		uint64_t ClassCreationTime = 0;

	public:
		bool Sleep(size_t Duration, Units Unit);
		uint64_t GetCounter();
		uint64_t CalculateTarget(uint64_t Target, Units Unit);
		uint64_t GetNanosecondsSinceClassCreation();

		HighPrecisionEventTimer(void *hpet);
		~HighPrecisionEventTimer();
	};

	class TimeStampCounter
	{
	private:
		uint64_t clk = 0;
		uint64_t ClassCreationTime = 0;

	public:
		bool Sleep(size_t Duration, Units Unit);
		uint64_t GetCounter();
		uint64_t CalculateTarget(uint64_t Target, Units Unit);
		uint64_t GetNanosecondsSinceClassCreation();

		TimeStampCounter();
		~TimeStampCounter();
	};

	class time
	{
	public:
		enum TimeActiveTimer
		{
			NONE = 0b0,
			RTC = 0b1,
			PIT = 0b10,
			HPET = 0b100,
			ACPI = 0b1000,
			APIC = 0b10000,
			TSC = 0b100000
		};

	private:
		int SupportedTimers = 0;
		TimeActiveTimer ActiveTimer = NONE;

		HighPrecisionEventTimer *hpet;
		TimeStampCounter *tsc;

	public:
		int GetSupportedTimers() { return SupportedTimers; }
		TimeActiveTimer GetActiveTimer() { return ActiveTimer; }
		bool ChangeActiveTimer(TimeActiveTimer Timer)
		{
			if (!(SupportedTimers & Timer))
				return false;
			ActiveTimer = Timer;
			return true;
		}

		bool Sleep(size_t Duration, Units Unit);
		uint64_t GetCounter();
		uint64_t CalculateTarget(uint64_t Target, Units Unit);
		uint64_t GetNanosecondsSinceClassCreation();
		void FindTimers(void *acpi);
		time();
		~time();
	};
}

#endif // !__FENNIX_KERNEL_TIME_H__
