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

#pragma once

#include <types.h>
#include <debug.h>
#include <cassert>
#include <irq.hpp>
#include <vector>
#include <chrono>

namespace Time
{
	struct Clock
	{
		int Year, Month, Day, Hour, Minute, Second;
		size_t Counter;
	};

	Clock ReadClock();
	Clock ConvertFromUnix(uint64_t Timestamp);

	class ClockSource
	{
	public:
		virtual const char *Name() const = 0;

		virtual uint64_t Now() const = 0;
		virtual uint64_t Frequency() const = 0;

		virtual void Sleep(std::chrono::nanoseconds ns) = 0;
	};

	class TimerDevice
	{
	public:
		virtual const char *Name() const = 0;
		virtual void SetOneShot(std::chrono::nanoseconds ns, Interrupt::IRQLines Line) = 0;
		virtual void SetPeriodic(std::chrono::nanoseconds ns, Interrupt::IRQLines Line) = 0;
	};

}

extern Time::ClockSource *GlobalClock;
extern Time::TimerDevice *GlobalTimer;
