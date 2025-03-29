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

#include <rand.hpp>
#include <cpu.hpp>

namespace Random
{
	uint64_t Seed = 0xdeadbeef;
	bool RDRANDFlag, RDSEEDFlag;

	__constructor void InitRandomSeed()
	{
		if (std::strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
		{
			if (std::strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
			{
				CPU::x86::AMD::CPUID0x00000007_ECX_0 cpuid;
				RDSEEDFlag = cpuid.EBX.RDSEED;
			}
			else if (std::strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
			{
				CPU::x86::Intel::CPUID0x00000007_0 cpuid;
				RDSEEDFlag = cpuid.EBX.RDSEED;
			}
		}
		else
			RDSEEDFlag = false;

		if (std::strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
		{
			if (std::strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
			{
				CPU::x86::AMD::CPUID0x00000001 cpuid;
				RDRANDFlag = cpuid.ECX.RDRAND;
			}
			else if (std::strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
			{
				CPU::x86::Intel::CPUID0x00000001 cpuid;
				RDRANDFlag = cpuid.ECX.RDRAND;
			}
		}
		else
			RDRANDFlag = false;

		if (RDSEEDFlag)
			fixme("RDSEED is not implemented yet!");
	}

	uint16_t rand16()
	{
#if defined(__amd64__) || defined(__i386__)
		if (RDRANDFlag)
		{
			uint16_t RDRANDValue = 0;
			asmv("1: rdrand %0; jnc 1b" : "=r"(RDRANDValue));
			return RDRANDValue;
		}

		Seed = Seed * 1103515245 + 12345;
		return (uint16_t)(Seed / 65536) % __UINT16_MAX__;
#endif
		return 0;
	}

	uint32_t rand32()
	{
#if defined(__amd64__) || defined(__i386__)
		if (RDRANDFlag)
		{
			uint32_t RDRANDValue = 0;
			asmv("1: rdrand %0; jnc 1b" : "=r"(RDRANDValue));
			return RDRANDValue;
		}

		Seed = Seed * 1103515245 + 12345;
		return (uint32_t)(Seed / 65536) % __UINT32_MAX__;
#endif
		return 0;
	}

	uint64_t rand64()
	{
#if defined(__amd64__) || defined(__i386__)
		if (RDRANDFlag)
		{
			uintptr_t RDRANDValue = 0;
			asmv("1: rdrand %0; jnc 1b" : "=r"(RDRANDValue));
			return RDRANDValue;
		}

		Seed = Seed * 1103515245 + 12345;
		return (uint64_t)(Seed / 65536) % __UINT64_MAX__;
#endif
		return 0;
	}

	void ChangeSeed(uint64_t CustomSeed) { Seed = CustomSeed; }
}
