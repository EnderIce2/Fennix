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
	static inline uint64_t rdtsc()
	{
#if defined(__amd64__) || defined(__i386__)
		unsigned int lo, hi;
		__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
		return ((uint64_t)hi << 32) | lo;
#else
		return 0;
#endif
	}

	bool TimeStampCounter::Sleep(uint64_t Nanoseconds)
	{
		uint64_t target = this->GetNanoseconds() + Nanoseconds;
		while (this->GetNanoseconds() < target)
			CPU::Pause();
		return true;
	}

	uint64_t TimeStampCounter::GetNanoseconds()
	{
		uint64_t tsc = rdtsc();
		return (tsc * 1000000000ULL) / this->clk;
	}

	TimeStampCounter::TimeStampCounter()
	{
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

		if (!TSCInvariant)
		{
			KPrint("\x1b[33mTSC is not invariant");
			return;
		}

		const int attempts = 5;
		uint64_t ns = 10000000ULL; /* 10 ms */
		uint64_t total_clk = 0;
		uint64_t overhead = 0;

		for (int i = 0; i < attempts; ++i)
		{
			uint64_t t0 = rdtsc();
			uint64_t t1 = rdtsc();
			overhead += (t1 - t0);
		}
		overhead /= attempts;

		for (int i = 0; i < attempts; ++i)
		{
			uint64_t tsc_start = rdtsc();
			uint64_t hpet_start = TimeManager->GetTimeNs();
			while (TimeManager->GetTimeNs() - hpet_start < ns)
				CPU::Pause();
			uint64_t tsc_end = rdtsc();
			total_clk += (tsc_end - tsc_start - overhead) * 1000000000ULL / ns;
		}
		this->clk = total_clk / attempts;
		KPrint("TSC frequency: %lu MHz", this->clk / 1000000);
		this->ClassCreationTime = this->GetNanoseconds();
		fixme("tsc not working as expected");
		this->clk = 0; /* disable */
	}
}
