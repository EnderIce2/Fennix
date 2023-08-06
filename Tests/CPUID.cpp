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

#include <types.h>
#include <memory.hpp>
#include <debug.h>

extern bool DebuggerIsAttached;

__constructor void TestCPUIDStructs()
{
	if (!DebuggerIsAttached)
		return;

	if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
	{
		CPU::x86::AMD::CPUID0x00000000 cpuid0;
		CPU::x86::AMD::CPUID0x00000001 cpuid1;
		CPU::x86::AMD::CPUID0x00000005 cpuid5;
		CPU::x86::AMD::CPUID0x00000006 cpuid6;
		CPU::x86::AMD::CPUID0x00000007 cpuid7;
		CPU::x86::AMD::CPUID0x0000000B_ECX_0 cpuidB_C_0;
		CPU::x86::AMD::CPUID0x0000000B_ECX_1 cpuidB_C_1;
		CPU::x86::AMD::CPUID0x0000000D_ECX_0 cpuidD_C_0;
		CPU::x86::AMD::CPUID0x0000000D_ECX_1 cpuidD_C_1;
		CPU::x86::AMD::CPUID0x0000000D_ECX_2 cpuidD_C_2;
		CPU::x86::AMD::CPUID0x0000000D_ECX_11 cpuidD_C_11;
		CPU::x86::AMD::CPUID0x0000000D_ECX_12 cpuidD_C_12;
		CPU::x86::AMD::CPUID0x0000000D_ECX_3E cpuidD_C_3E;
		CPU::x86::AMD::CPUID0x80000000 cpuid80000000;
		CPU::x86::AMD::CPUID0x80000001 cpuid80000001;
		CPU::x86::AMD::CPUID0x80000002 cpuid80000002;
		CPU::x86::AMD::CPUID0x80000003 cpuid80000003;
		CPU::x86::AMD::CPUID0x80000004 cpuid80000004;
		CPU::x86::AMD::CPUID0x80000005 cpuid80000005;
		CPU::x86::AMD::CPUID0x80000006 cpuid80000006;
		CPU::x86::AMD::CPUID0x80000007 cpuid80000007;
		CPU::x86::AMD::CPUID0x80000008 cpuid80000008;
		CPU::x86::AMD::CPUID0x8000000A cpuid8000000A;
		CPU::x86::AMD::CPUID0x80000019 cpuid80000019;
		CPU::x86::AMD::CPUID0x8000001A cpuid8000001A;
		CPU::x86::AMD::CPUID0x8000001B cpuid8000001B;
		CPU::x86::AMD::CPUID0x8000001C cpuid8000001C;
		CPU::x86::AMD::CPUID0x8000001D cpuid8000001D;
		CPU::x86::AMD::CPUID0x8000001E cpuid8000001E;
		CPU::x86::AMD::CPUID0x8000001F cpuid8000001F;
		CPU::x86::AMD::CPUID0x80000020 cpuid80000020;
		CPU::x86::AMD::CPUID0x80000021 cpuid80000021;
		CPU::x86::AMD::CPUID0x80000022 cpuid80000022;
		CPU::x86::AMD::CPUID0x80000023 cpuid80000023;
		CPU::x86::AMD::CPUID0x80000026 cpuid80000026;

		cpuid0.Get();
		cpuid1.Get();
		cpuid5.Get();
		cpuid6.Get();
		cpuid7.Get();
		cpuidB_C_0.Get();
		cpuidB_C_1.Get();
		cpuidD_C_0.Get();
		cpuidD_C_1.Get();
		cpuidD_C_2.Get();
		cpuidD_C_11.Get();
		cpuidD_C_12.Get();
		cpuidD_C_3E.Get();
		cpuid80000000.Get();
		cpuid80000001.Get();
		cpuid80000002.Get();
		cpuid80000003.Get();
		cpuid80000004.Get();
		cpuid80000005.Get();
		cpuid80000006.Get();
		cpuid80000007.Get();
		cpuid80000008.Get();
		cpuid8000000A.Get();
		cpuid80000019.Get();
		cpuid8000001A.Get();
		cpuid8000001B.Get();
		cpuid8000001C.Get();
		cpuid8000001D.Get();
		cpuid8000001E.Get();
		cpuid8000001F.Get();
		cpuid80000020.Get();
		cpuid80000021.Get();
		cpuid80000022.Get();
		cpuid80000023.Get();
		cpuid80000026.Get();

		// asmv("int3");
	}
	else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
	{
		CPU::x86::Intel::CPUID0x00000000 cpuid0;
		CPU::x86::Intel::CPUID0x00000001 cpuid1;
		CPU::x86::Intel::CPUID0x00000002 cpuid2;
		CPU::x86::Intel::CPUID0x00000003 cpuid3;
		CPU::x86::Intel::CPUID0x00000004_1 cpuid4_1;
		CPU::x86::Intel::CPUID0x00000005 cpuid5;
		CPU::x86::Intel::CPUID0x00000006 cpuid6;
		CPU::x86::Intel::CPUID0x00000007_0 cpuid7_0;
		CPU::x86::Intel::CPUID0x00000007_1 cpuid7_1;
		CPU::x86::Intel::CPUID0x0000000A cpuidA;
		CPU::x86::Intel::CPUID0x00000015 cpuid15;
		CPU::x86::Intel::CPUID0x00000016 cpuid16;
		CPU::x86::Intel::CPUID0x80000000 cpuid80000000;
		CPU::x86::Intel::CPUID0x80000001 cpuid80000001;
		CPU::x86::Intel::CPUID0x80000002 cpuid80000002;
		CPU::x86::Intel::CPUID0x80000003 cpuid80000003;
		CPU::x86::Intel::CPUID0x80000004 cpuid80000004;
		CPU::x86::Intel::CPUID0x80000006 cpuid80000006;
		CPU::x86::Intel::CPUID0x80000008 cpuid80000008;
		CPU::x86::Intel::CPUID0x8000000A cpuid8000000A;

		cpuid0.Get();
		cpuid1.Get();
		cpuid2.Get();
		cpuid3.Get();
		cpuid4_1.Get();
		cpuid5.Get();
		cpuid6.Get();
		cpuid7_0.Get();
		cpuid7_1.Get();
		cpuidA.Get();
		cpuid15.Get();
		cpuid16.Get();
		cpuid80000000.Get();
		cpuid80000001.Get();
		cpuid80000002.Get();
		cpuid80000003.Get();
		cpuid80000004.Get();
		cpuid80000006.Get();
		cpuid80000008.Get();
		cpuid8000000A.Get();

		// asmv("int3");
	}
}

#endif // DEBUG
