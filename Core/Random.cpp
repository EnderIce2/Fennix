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
    static uint64_t Seed = 0xdeadbeef;

    uint16_t rand16()
    {
        int RDRANDFlag = 0;
        if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
        {
            CPU::x86::AMD::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
            CPU::x86::Intel::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a86)
        if (RDRANDFlag)
        {
            uint16_t RDRANDValue = 0;
            asmv("1: rdrand %0; jnc 1b"
                 : "=r"(RDRANDValue));
            return RDRANDValue;
        }
#endif

        Seed = Seed * 1103515245 + 12345;
        return (uint16_t)(Seed / 65536) % __UINT16_MAX__;
    }

    uint32_t rand32()
    {
        int RDRANDFlag = 0;
        if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
        {
            CPU::x86::AMD::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
            CPU::x86::Intel::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a86)
        if (RDRANDFlag)
        {
            uint32_t RDRANDValue = 0;
            asmv("1: rdrand %0; jnc 1b"
                 : "=r"(RDRANDValue));
            return RDRANDValue;
        }
#endif

        Seed = Seed * 1103515245 + 12345;
        return (uint32_t)(Seed / 65536) % __UINT16_MAX__;
    }

    uint64_t rand64()
    {
        int RDRANDFlag = 0;
        if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
        {
            CPU::x86::AMD::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
            CPU::x86::Intel::CPUID0x00000001 cpuid;
            cpuid.Get();
            RDRANDFlag = cpuid.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a86)
        if (RDRANDFlag)
        {
            uint64_t RDRANDValue = 0;
            asmv("1: rdrand %0; jnc 1b"
                 : "=r"(RDRANDValue));
            return RDRANDValue;
        }
#endif

        Seed = Seed * 1103515245 + 12345;
        return (uint64_t)(Seed / 65536) % __UINT16_MAX__;
    }

    void ChangeSeed(uint64_t CustomSeed) { Seed = CustomSeed; }
}
