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
#if defined(a64)
            CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(a32)
            CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1amd.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
#if defined(a64)
            CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(a32)
            CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1intel.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a64) || defined(a32)
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
#if defined(a64)
            CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(a32)
            CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1amd.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
#if defined(a64)
            CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(a32)
            CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1intel.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a64) || defined(a32)
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
#if defined(a64)
            CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(a32)
            CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1amd.ECX.RDRAND;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
#if defined(a64)
            CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(a32)
            CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
                 : "a"(0x1));
#endif
            RDRANDFlag = cpuid1intel.ECX.RDRAND;
        }

        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
            RDRANDFlag = 0;

#if defined(a64) || defined(a32)
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
