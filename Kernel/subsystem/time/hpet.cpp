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
	bool HighPrecisionEventTimer::Sleep(size_t Nanoseconds)
	{
		uint64_t target = this->GetNanoseconds() + Nanoseconds;
		while (this->GetNanoseconds() < target)
			CPU::Pause();
		return true;
	}

	uint64_t HighPrecisionEventTimer::GetNanoseconds()
	{
#if defined(__amd64__)
		uint64_t counter = mminq(&this->hpet->MainCounter);
#elif defined(__i386__)
		uint64_t counter = mminl(&this->hpet->MainCounter);
#else
		return 0;
#endif
		/* convert ticks to nanoseconds: counter * period_fs / 1e6 */
		return (counter * 1'000'000'000ULL) / this->Period;
	}

	HighPrecisionEventTimer::HighPrecisionEventTimer(void *hpet)
	{
#if defined(__amd64__) || defined(__i386__)
		ACPI::ACPI::HPETHeader *hdr = (ACPI::ACPI::HPETHeader *)hpet;
		Memory::Virtual vmm;
		vmm.Map((void *)hdr->Address.Address, (void *)hdr->Address.Address, Memory::RW | Memory::PCD | Memory::PWT);
		this->hpet = reinterpret_cast<HPET *>(hdr->Address.Address);
		debug("%s timer is at address %#lx", hdr->Header.OEMID, hdr->Address.Address);
		uint64_t period_fs = this->hpet->CapabilitiesID >> 32;
		if (period_fs == 0)
		{
			warn("HPET: Invalid period in CapabilitiesID");
			return;
		}

		/* Hz = 1e15 / period_fs */
		this->Period = 1'000'000'000'000'000ULL / period_fs;
		KPrint("HPET tick period: %lu femtoseconds -> %u Hz", period_fs, this->Period);
#ifdef __amd64__
		mmoutq(&this->hpet->Configuration, 0);
		mmoutq(&this->hpet->MainCounter, 0);
		mmoutq(&this->hpet->Configuration, 1);
#else
		mmoutl(&this->hpet->Configuration, 0);
		mmoutl(&this->hpet->MainCounter, 0);
		mmoutl(&this->hpet->Configuration, 1);
#endif

		for (int i = 0; i < 5; i++)
		{
			uint64_t val = mminq(&this->hpet->MainCounter);
			KPrint("HPET counter test %d: %llu", i, val);
		}

		uint64_t cfg = mminq(&this->hpet->Configuration);
		if (!(cfg & 1))
			warn("HPET counter is not enabled!");

		ClassCreationTime = this->GetNanoseconds();
#endif
	}

	HighPrecisionEventTimer::~HighPrecisionEventTimer()
	{
#ifdef __amd64__
		mmoutq(&this->hpet->Configuration, 0);
#else
		mmoutl(&this->hpet->Configuration, 0);
#endif
	}
}
