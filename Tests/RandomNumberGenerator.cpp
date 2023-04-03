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

#ifdef DEBUG

#include <rand.hpp>
#include <debug.h>
#include <cpu.hpp>

__constructor void TestRandom()
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
        uint64_t RDSEEDValue = 0;
        asmv("1: rdseed %0; jnc 1b"
             : "=r"(RDSEEDValue));
        debug("RDSEED: %ld", RDSEEDValue);
    }
#endif

    Random::ChangeSeed(0xdeadbeef);
    uint16_t Seeds16[16];
    uint32_t Seeds32[16];
    uint64_t Seeds64[16];
    for (short i = 0; i < 16; i++)
    {
        Seeds16[i] = Random::rand16();
        Seeds32[i] = Random::rand32();
        Seeds64[i] = Random::rand64();
    }
    debug("Random 16: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld", Seeds16[0], Seeds16[1], Seeds16[2], Seeds16[3], Seeds16[4], Seeds16[5], Seeds16[6], Seeds16[7], Seeds16[8], Seeds16[9], Seeds16[10], Seeds16[11], Seeds16[12], Seeds16[13], Seeds16[14], Seeds16[15]);
    debug("Random 32: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld", Seeds32[0], Seeds32[1], Seeds32[2], Seeds32[3], Seeds32[4], Seeds32[5], Seeds32[6], Seeds32[7], Seeds32[8], Seeds32[9], Seeds32[10], Seeds32[11], Seeds32[12], Seeds32[13], Seeds32[14], Seeds32[15]);
    debug("Random 64: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld", Seeds64[0], Seeds64[1], Seeds64[2], Seeds64[3], Seeds64[4], Seeds64[5], Seeds64[6], Seeds64[7], Seeds64[8], Seeds64[9], Seeds64[10], Seeds64[11], Seeds64[12], Seeds64[13], Seeds64[14], Seeds64[15]);
}

#endif // DEBUG
