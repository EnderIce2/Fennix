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

#include <cpu.hpp>

#include <memory.hpp>
#include <convert.h>
#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

#if defined(a64)
using namespace CPU::x64;
#elif defined(a32)
using namespace CPU::x32;
#elif defined(aa64)
#endif

namespace CPU
{
	static bool SSEEnabled = false;

	const char *Vendor()
	{
		static char Vendor[13] = {0};
		if (Vendor[0] != 0)
			return Vendor;
#if defined(a64)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x0, &eax, &ebx, &ecx, &edx);
		memcpy(Vendor + 0, &ebx, 4);
		memcpy(Vendor + 4, &edx, 4);
		memcpy(Vendor + 8, &ecx, 4);
#elif defined(a32)
		uint32_t eax, ebx, ecx, edx;
		x32::cpuid(0x0, &eax, &ebx, &ecx, &edx);
		memcpy(Vendor + 0, &ebx, 4);
		memcpy(Vendor + 4, &edx, 4);
		memcpy(Vendor + 8, &ecx, 4);
#elif defined(aa64)
#error "Not implemented"
#endif
		return Vendor;
	}

	const char *Name()
	{
		static char Name[49] = {0};
		if (Name[0] != 0)
			return Name;
#if defined(a64)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 0, &eax, 4);
		memcpy(Name + 4, &ebx, 4);
		memcpy(Name + 8, &ecx, 4);
		memcpy(Name + 12, &edx, 4);
		x64::cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 16, &eax, 4);
		memcpy(Name + 20, &ebx, 4);
		memcpy(Name + 24, &ecx, 4);
		memcpy(Name + 28, &edx, 4);
		x64::cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 32, &eax, 4);
		memcpy(Name + 36, &ebx, 4);
		memcpy(Name + 40, &ecx, 4);
		memcpy(Name + 44, &edx, 4);
#elif defined(a32)
		uint32_t eax, ebx, ecx, edx;
		x32::cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 0, &eax, 4);
		memcpy(Name + 4, &ebx, 4);
		memcpy(Name + 8, &ecx, 4);
		memcpy(Name + 12, &edx, 4);
		x32::cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 16, &eax, 4);
		memcpy(Name + 20, &ebx, 4);
		memcpy(Name + 24, &ecx, 4);
		memcpy(Name + 28, &edx, 4);
		x32::cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
		memcpy(Name + 32, &eax, 4);
		memcpy(Name + 36, &ebx, 4);
		memcpy(Name + 40, &ecx, 4);
		memcpy(Name + 44, &edx, 4);
#elif defined(aa64)
#error "Not implemented"
#endif
		return Name;
	}

	const char *Hypervisor()
	{
		static char Hypervisor[13] = {0};
		if (Hypervisor[0] != 0)
			return Hypervisor;
#if defined(a64)
		uint32_t eax, ebx, ecx, edx;
		x64::cpuid(0x1, &eax, &ebx, &ecx, &edx);
		if (!(ecx & (1 << 31))) /* Intel & AMD are the same */
		{
			Hypervisor[0] = 'N';
			Hypervisor[1] = 'o';
			Hypervisor[2] = 'n';
			Hypervisor[3] = 'e';
			return Hypervisor;
		}

		x64::cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
		memcpy(Hypervisor + 0, &ebx, 4);
		memcpy(Hypervisor + 4, &ecx, 4);
		memcpy(Hypervisor + 8, &edx, 4);
#elif defined(a32)
		uint32_t eax, ebx, ecx, edx;
		x32::cpuid(0x1, &eax, &ebx, &ecx, &edx);
		if (!(ecx & (1 << 31))) /* Intel & AMD are the same */
		{
			Hypervisor[0] = 'N';
			Hypervisor[1] = 'o';
			Hypervisor[2] = 'n';
			Hypervisor[3] = 'e';
			return Hypervisor;
		}

		x32::cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
		memcpy(Hypervisor + 0, &ebx, 4);
		memcpy(Hypervisor + 4, &ecx, 4);
		memcpy(Hypervisor + 8, &edx, 4);
#elif defined(aa64)
#error "Not implemented"
#endif
		return Hypervisor;
	}

	bool Interrupts(InterruptsType Type)
	{
		switch (Type)
		{
		case Check:
		{
			uintptr_t Flags;
#if defined(a64)
			asmv("pushfq");
			asmv("popq %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(a32)
			asmv("pushfl");
			asmv("popl %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(aa64)
			asmv("mrs %0, cpsr"
				 : "=r"(Flags));
			return Flags & (1 << 7);
#endif
		}
		case Enable:
		{
#if defined(a86)
			asmv("sti");
#elif defined(aa64)
			asmv("cpsie i");
#endif
			return true;
		}
		case Disable:
		{
#if defined(a86)
			asmv("cli");
#elif defined(aa64)
			asmv("cpsid i");
#endif
			return true;
		}
		default:
			assert(!"Unknown InterruptsType");
			break;
		}
		return false;
	}

	void *PageTable(void *PT)
	{
		void *ret;
#if defined(a64)
		asmv("movq %%cr3, %0"
			 : "=r"(ret));

		if (PT)
		{
			asmv("movq %0, %%cr3"
				 :
				 : "r"(PT)
				 : "memory");
		}
#elif defined(a32)
		asmv("movl %%cr3, %0"
			 : "=r"(ret));

		if (PT)
		{
			asmv("movl %0, %%cr3"
				 :
				 : "r"(PT)
				 : "memory");
		}
#elif defined(aa64)
		asmv("mrs %0, ttbr0_el1"
			 : "=r"(ret));

		if (PT)
		{
			asmv("msr ttbr0_el1, %0"
				 :
				 : "r"(PT)
				 : "memory");
		}
#endif
		return ret;
	}

	struct SupportedFeat
	{
		bool PGE = false;
		bool SSE = false;
		bool UMIP = false;
		bool SMEP = false;
		bool SMAP = false;
	};

	SupportedFeat GetCPUFeat()
	{
		SupportedFeat feat{};

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid1;
			CPU::x86::AMD::CPUID0x00000007_ECX_0 cpuid7;

			feat.PGE = cpuid1.EDX.PGE;
			feat.SSE = cpuid1.EDX.SSE;
			feat.SMEP = cpuid7.EBX.SMEP;
			feat.SMAP = cpuid7.EBX.SMAP;
			feat.UMIP = cpuid7.ECX.UMIP;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid1;
			CPU::x86::Intel::CPUID0x00000007_0 cpuid7_0;

			feat.PGE = cpuid1.EDX.PGE;
			feat.SSE = cpuid1.EDX.SSE;
			feat.SMEP = cpuid7_0.EBX.SMEP;
			feat.SMAP = cpuid7_0.EBX.SMAP;
			feat.UMIP = cpuid7_0.ECX.UMIP;
		}

		return feat;
	}

	void InitializeFeatures(int Core)
	{
		static int BSP = 0;
		SupportedFeat feat = GetCPUFeat();

		CR0 cr0 = readcr0();
		CR4 cr4 = readcr4();

		if (Config.SIMD == false)
		{
			debug("Disabling SSE support...");
			feat.SSE = false;
		}

		if (feat.PGE)
		{
			debug("Enabling global pages support...");
			if (!BSP)
				KPrint("Global Pages is supported.");
			cr4.PGE = true;
		}

		bool SSEEnableAfter = false;

		/* Not sure if my code is not working properly or something else is the issue. */
		if ((strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0) &&
			feat.SSE)
		{
			debug("Enabling SSE support...");
			if (!BSP)
				KPrint("SSE is supported.");
			cr0.EM = false;
			cr0.MP = true;
			cr4.OSFXSR = true;
			cr4.OSXMMEXCPT = true;

			CPUData *CoreData = GetCPU(Core);
			CoreData->Data.FPU.mxcsr = 0b0001111110000000;
			CoreData->Data.FPU.mxcsrmask = 0b1111111110111111;
			CoreData->Data.FPU.fcw = 0b0000001100111111;
			fxrstor(&CoreData->Data.FPU);

			SSEEnableAfter = true;
		}

		/* More info in AMD64 Architecture Programmer's Manual
			Volume 2: 3.1.1 CR0 Register */

		/* Not Write-Through
			This is ignored on recent processors.
		*/
		cr0.NW = false;

		/* Cache Disable
			Wether the CPU should cache memory or not.
			If it's enabled, PWT and PCD are ignored.
		*/
		cr0.CD = false;

		/* Write Protect
			When set, the supervisor can't write to read-only pages.
		*/
		cr0.WP = true;

		/* Alignment check
			The CPU checks the alignment of memory operands
			and generates #AC if the alignment is incorrect.

			The condition for an alignment check is:
			- The AM flag in CR0 is set.
			- The AC flag in the RFLAGS register is set.
			- CPL is 3.
		*/
		cr0.AM = true;

		debug("Updating CR0...");
		writecr0(cr0);
		debug("Updated CR0.");

		debug("CPU Prevention Features:%s%s%s",
			  feat.SMEP ? " SMEP" : "",
			  feat.SMAP ? " SMAP" : "",
			  feat.UMIP ? " UMIP" : "");
		/* User-Mode Instruction Prevention
			This prevents user-mode code from executing these instructions:
				SGDT, SIDT, SLDT, SMSW, STR
			If any of these instructions are executed with CPL > 0, a #GP is generated.
		*/
		// cr4.UMIP = feat.UMIP;

		/* Supervisor Mode Execution Prevention
			This prevents user-mode code from executing code in the supervisor mode.
		*/
		cr4.SMEP = feat.SMEP;

		/* Supervisor Mode Access Prevention
			This prevents supervisor-mode code from accessing user-mode pages.
		*/
		cr4.SMAP = feat.SMAP;

		debug("Updating CR4...");
		writecr4(cr4);
		debug("Updated CR4.");

		debug("Enabling PAT support...");
		wrmsr(MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
		if (!BSP++)
			trace("Features for BSP initialized.");
		if (SSEEnableAfter)
		{
			SSEEnabled = true;
			debug("SSE support enabled.");
		}
	}

	uint64_t Counter()
	{
		// TODO: Get the counter from the x2APIC or any other timer that is available. (TSC is not available on all CPUs)
		uint64_t Counter;
#if defined(a86)
		uint32_t eax, edx;
		asmv("rdtsc"
			 : "=a"(eax),
			   "=d"(edx));
		Counter = ((uint64_t)eax) | (((uint64_t)edx) << 32);
#elif defined(aa64)
		asmv("mrs %0, cntvct_el0"
			 : "=r"(Counter));
#endif
		return Counter;
	}

	uint64_t CheckSIMD()
	{
		if (unlikely(!SSEEnabled))
			return SIMD_NONE;

#warning "TODO: Proper SIMD support"
		return SIMD_NONE;

#if defined(a86)
		static uint64_t SIMDType = SIMD_NONE;

		if (likely(SIMDType != SIMD_NONE))
			return SIMDType;

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid;
			asmv("cpuid"
				 : "=a"(cpuid.EAX.raw), "=b"(cpuid.EBX.raw),
				   "=c"(cpuid.ECX.raw), "=d"(cpuid.EDX.raw)
				 : "a"(0x1));

			if (cpuid.ECX.SSE42)
				SIMDType |= SIMD_SSE42;
			else if (cpuid.ECX.SSE41)
				SIMDType |= SIMD_SSE41;
			else if (cpuid.ECX.SSSE3)
				SIMDType |= SIMD_SSSE3;
			else if (cpuid.ECX.SSE3)
				SIMDType |= SIMD_SSE3;
			else if (cpuid.EDX.SSE2)
				SIMDType |= SIMD_SSE2;
			else if (cpuid.EDX.SSE)
				SIMDType |= SIMD_SSE;

#ifdef DEBUG
			if (cpuid.ECX.SSE42)
				debug("SSE4.2 is supported.");
			if (cpuid.ECX.SSE41)
				debug("SSE4.1 is supported.");
			if (cpuid.ECX.SSSE3)
				debug("SSSE3 is supported.");
			if (cpuid.ECX.SSE3)
				debug("SSE3 is supported.");
			if (cpuid.EDX.SSE2)
				debug("SSE2 is supported.");
			if (cpuid.EDX.SSE)
				debug("SSE is supported.");
#endif

			return SIMDType;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
			asmv("cpuid"
				 : "=a"(cpuid.EAX.raw), "=b"(cpuid.EBX.raw), "=c"(cpuid.ECX.raw), "=d"(cpuid.EDX.raw)
				 : "a"(0x1));

			if (cpuid.ECX.SSE4_2)
				SIMDType |= SIMD_SSE42;
			else if (cpuid.ECX.SSE4_1)
				SIMDType |= SIMD_SSE41;
			else if (cpuid.ECX.SSSE3)
				SIMDType |= SIMD_SSSE3;
			else if (cpuid.ECX.SSE3)
				SIMDType |= SIMD_SSE3;
			else if (cpuid.EDX.SSE2)
				SIMDType |= SIMD_SSE2;
			else if (cpuid.EDX.SSE)
				SIMDType |= SIMD_SSE;

#ifdef DEBUG
			if (cpuid.ECX.SSE4_2)
				debug("SSE4.2 is supported.");
			if (cpuid.ECX.SSE4_1)
				debug("SSE4.1 is supported.");
			if (cpuid.ECX.SSSE3)
				debug("SSSE3 is supported.");
			if (cpuid.ECX.SSE3)
				debug("SSE3 is supported.");
			if (cpuid.EDX.SSE2)
				debug("SSE2 is supported.");
			if (cpuid.EDX.SSE)
				debug("SSE is supported.");
#endif
			return SIMDType;
		}

		debug("No SIMD support.");
#endif // a64 || a32
		return SIMD_NONE;
	}

	bool CheckSIMD(x86SIMDType Type)
	{
		if (unlikely(!SSEEnabled))
			return false;

#if defined(a86)
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid;
			asmv("cpuid"
				 : "=a"(cpuid.EAX.raw), "=b"(cpuid.EBX.raw), "=c"(cpuid.ECX.raw), "=d"(cpuid.EDX.raw)
				 : "a"(0x1));

			if (Type == SIMD_SSE42)
				return cpuid.ECX.SSE42;
			else if (Type == SIMD_SSE41)
				return cpuid.ECX.SSE41;
			else if (Type == SIMD_SSSE3)
				return cpuid.ECX.SSSE3;
			else if (Type == SIMD_SSE3)
				return cpuid.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid.EDX.SSE;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
			asmv("cpuid"
				 : "=a"(cpuid.EAX.raw), "=b"(cpuid.EBX.raw), "=c"(cpuid.ECX.raw), "=d"(cpuid.EDX.raw)
				 : "a"(0x1));

			if (Type == SIMD_SSE42)
				return cpuid.ECX.SSE4_2;
			else if (Type == SIMD_SSE41)
				return cpuid.ECX.SSE4_1;
			else if (Type == SIMD_SSSE3)
				return cpuid.ECX.SSSE3;
			else if (Type == SIMD_SSE3)
				return cpuid.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid.EDX.SSE;
		}
#endif // a64 || a32
		return false;
	}
}
