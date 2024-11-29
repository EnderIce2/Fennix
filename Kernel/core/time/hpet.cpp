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
	bool HighPrecisionEventTimer::Sleep(size_t Duration, Units Unit)
	{
#if defined(__amd64__)
		uint64_t Target = mminq(&hpet->MainCounterValue) + (Duration * ConvertUnit(Unit)) / clk;
		while (mminq(&hpet->MainCounterValue) < Target)
			CPU::Pause();
		return true;
#elif defined(__i386__)
		uint64_t Target = mminl(&hpet->MainCounterValue) + (Duration * ConvertUnit(Unit)) / clk;
		while (mminl(&hpet->MainCounterValue) < Target)
			CPU::Pause();
		return true;
#endif
		return false;
	}

	uint64_t HighPrecisionEventTimer::GetCounter()
	{
#if defined(__amd64__)
		return mminq(&hpet->MainCounterValue);
#elif defined(__i386__)
		return mminl(&hpet->MainCounterValue);
#endif
	}

	uint64_t HighPrecisionEventTimer::CalculateTarget(uint64_t Target, Units Unit)
	{
#if defined(__amd64__)
		return mminq(&hpet->MainCounterValue) + (Target * ConvertUnit(Unit)) / clk;
#elif defined(__i386__)
		return mminl(&hpet->MainCounterValue) + (Target * ConvertUnit(Unit)) / clk;
#endif
	}

	uint64_t HighPrecisionEventTimer::GetNanosecondsSinceClassCreation()
	{
#if defined(__amd64__) || defined(__i386__)
		uint64_t Subtraction = this->GetCounter() - this->ClassCreationTime;
		if (Subtraction <= 0 || this->clk <= 0)
			return 0;

		Subtraction *= ConvertUnit(Units::Nanoseconds);
		return uint64_t(Subtraction / this->clk);
#endif
	}

	HighPrecisionEventTimer::HighPrecisionEventTimer(void *hpet)
	{
#if defined(__amd64__) || defined(__i386__)
		ACPI::ACPI::HPETHeader *HPET_HDR = (ACPI::ACPI::HPETHeader *)hpet;
		Memory::Virtual vmm;
		vmm.Map((void *)HPET_HDR->Address.Address,
				(void *)HPET_HDR->Address.Address,
				Memory::PTFlag::RW | Memory::PTFlag::PCD);
		this->hpet = (HPET *)HPET_HDR->Address.Address;
		trace("%s timer is at address %016p",
			  HPET_HDR->Header.OEMID,
			  (void *)HPET_HDR->Address.Address);
		clk = s_cst(uint32_t, (uint64_t)this->hpet->GeneralCapabilities >> 32);
		KPrint("HPET clock is %u Hz", clk);
#ifdef __amd64__
		mmoutq(&this->hpet->GeneralConfiguration, 0);
		mmoutq(&this->hpet->MainCounterValue, 0);
		mmoutq(&this->hpet->GeneralConfiguration, 1);
#else
		mmoutl(&this->hpet->GeneralConfiguration, 0);
		mmoutl(&this->hpet->MainCounterValue, 0);
		mmoutl(&this->hpet->GeneralConfiguration, 1);
#endif
		ClassCreationTime = this->GetCounter();
#endif
	}

	HighPrecisionEventTimer::~HighPrecisionEventTimer()
	{
	}
}
