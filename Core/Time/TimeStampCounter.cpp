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
	bool TimeStampCounter::Sleep(size_t Duration, Units Unit)
	{
#if defined(a86)
		size_t Target = this->GetCounter() + (Duration * ConvertUnit(Unit)) / this->clk;
		while (this->GetCounter() < Target)
			CPU::Pause();
		return true;
#endif
	}

	size_t TimeStampCounter::GetCounter()
	{
#if defined(a86)
		return CPU::Counter();
#endif
	}

	size_t TimeStampCounter::CalculateTarget(size_t Target, Units Unit)
	{
#if defined(a86)
		return this->GetCounter() + (Target * ConvertUnit(Unit)) / this->clk;
#endif
	}

	size_t TimeStampCounter::GetNanosecondsSinceClassCreation()
	{
#if defined(a86)
		return (this->GetCounter() - this->ClassCreationTime) / this->clk;
#endif
	}

	TimeStampCounter::TimeStampCounter()
	{
#if defined(a86)
		fixme(""); // FIXME: This is not a good way to measure the clock speed
		size_t Start = CPU::Counter();
		TimeManager->Sleep(1, Units::Milliseconds);
		size_t End = CPU::Counter();

		this->clk = End - Start;
		this->ClassCreationTime = this->GetCounter();
#endif
	}

	TimeStampCounter::~TimeStampCounter()
	{
	}
}
