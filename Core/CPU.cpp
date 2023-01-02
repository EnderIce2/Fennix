#include <cpu.hpp>

#include <memory.hpp>
#include <convert.h>
#include <debug.h>

#include "../kernel.h"

namespace CPU
{
	char *Vendor()
	{
		static char Vendor[13];
#if defined(__amd64__)
		uint32_t rax, rbx, rcx, rdx;
		x64::cpuid(0x0, &rax, &rbx, &rcx, &rdx);
		memcpy(Vendor + 0, &rbx, 4);
		memcpy(Vendor + 4, &rdx, 4);
		memcpy(Vendor + 8, &rcx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		x32::cpuid(0x0, &rax, &rbx, &rcx, &rdx);
		memcpy(Vendor + 0, &rbx, 4);
		memcpy(Vendor + 4, &rdx, 4);
		memcpy(Vendor + 8, &rcx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Vendor[0]));
#endif
		return Vendor;
	}

	char *Name()
	{
		static char Name[49];
#if defined(__amd64__)
		uint32_t rax, rbx, rcx, rdx;
		x64::cpuid(0x80000002, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 0, &rax, 4);
		memcpy(Name + 4, &rbx, 4);
		memcpy(Name + 8, &rcx, 4);
		memcpy(Name + 12, &rdx, 4);
		x64::cpuid(0x80000003, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 16, &rax, 4);
		memcpy(Name + 20, &rbx, 4);
		memcpy(Name + 24, &rcx, 4);
		memcpy(Name + 28, &rdx, 4);
		x64::cpuid(0x80000004, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 32, &rax, 4);
		memcpy(Name + 36, &rbx, 4);
		memcpy(Name + 40, &rcx, 4);
		memcpy(Name + 44, &rdx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		x32::cpuid(0x80000002, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 0, &rax, 4);
		memcpy(Name + 4, &rbx, 4);
		memcpy(Name + 8, &rcx, 4);
		memcpy(Name + 12, &rdx, 4);
		x32::cpuid(0x80000003, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 16, &rax, 4);
		memcpy(Name + 20, &rbx, 4);
		memcpy(Name + 24, &rcx, 4);
		memcpy(Name + 28, &rdx, 4);
		x32::cpuid(0x80000004, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 32, &rax, 4);
		memcpy(Name + 36, &rbx, 4);
		memcpy(Name + 40, &rcx, 4);
		memcpy(Name + 44, &rdx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Name[0]));
#endif
		return Name;
	}

	char *Hypervisor()
	{
		static char Hypervisor[13];
#if defined(__amd64__)
		uint32_t rax, rbx, rcx, rdx;
		x64::cpuid(0x40000000, &rax, &rbx, &rcx, &rdx);
		memcpy(Hypervisor + 0, &rbx, 4);
		memcpy(Hypervisor + 4, &rcx, 4);
		memcpy(Hypervisor + 8, &rdx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		x64::cpuid(0x40000000, &rax, &rbx, &rcx, &rdx);
		memcpy(Hypervisor + 0, &rbx, 4);
		memcpy(Hypervisor + 4, &rcx, 4);
		memcpy(Hypervisor + 8, &rdx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Hypervisor[0]));
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
#if defined(__amd64__)
			asmv("pushfq");
			asmv("popq %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(__i386__)
			asmv("pushfl");
			asmv("popl %0"
				 : "=r"(Flags));
			return Flags & (1 << 9);
#elif defined(__aarch64__)
			asmv("mrs %0, daif"
				 : "=r"(Flags));
			return !(Flags & (1 << 2));
#endif
		}
		case Enable:
		{
#if defined(__amd64__) || defined(__i386__)
			asmv("sti");
#elif defined(__aarch64__)
			asmv("msr daifclr, #2");
#endif
			return true;
		}
		case Disable:
		{
#if defined(__amd64__) || defined(__i386__)
			asmv("cli");
#elif defined(__aarch64__)
			asmv("msr daifset, #2");
#endif
			return true;
		}
		}
		return false;
	}

	void *PageTable(void *PT)
	{
#if defined(__amd64__)
		if (PT)
			asmv("movq %0, %%cr3"
				 :
				 : "r"(PT));
		else
			asmv("movq %%cr3, %0"
				 : "=r"(PT));
#elif defined(__i386__)
		if (PT)
			asmv("movl %0, %%cr3"
				 :
				 : "r"(PT));
		else
			asmv("movl %%cr3, %0"
				 : "=r"(PT));
#elif defined(__aarch64__)
		if (PT)
			asmv("msr ttbr0_el1, %0"
				 :
				 : "r"(PT));
		else
			asmv("mrs %0, ttbr0_el1"
				 : "=r"(PT));
#endif
		return PT;
	}

	void InitializeFeatures()
	{
#if defined(__amd64__)
		static int BSP = 0;
		x64::CR0 cr0 = x64::readcr0();
		x64::CR4 cr4 = x64::readcr4();
		uint32_t rax, rbx, rcx, rdx;
		x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
		if (rdx & x64::CPUID_FEAT_RDX_PGE)
		{
			debug("Enabling global pages support...");
			if (!BSP)
				KPrint("Global Pages is supported.");
			cr4.PGE = 1;
		}

		if (rdx & x64::CPUID_FEAT_RDX_SSE)
		{
			debug("Enabling SSE support...");
			if (!BSP)
				KPrint("SSE is supported.");
			cr0.EM = 0;
			cr0.MP = 1;
			cr4.OSFXSR = 1;
			cr4.OSXMMEXCPT = 1;
		}

		if (!BSP)
			KPrint("Enabling CPU cache.");

		cr0.NW = 0;
		cr0.CD = 0;
		cr0.WP = 1;

		x64::writecr0(cr0);

		debug("Enabling UMIP, SMEP & SMAP support...");
		x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
		if (rdx & x64::CPUID_FEAT_RDX_UMIP) // https://en.wikipedia.org/wiki/Control_register
		{
			if (!BSP)
				KPrint("UMIP is supported.");
			debug("UMIP is supported.");
			// cr4.UMIP = 1;
		}
		if (rdx & x64::CPUID_FEAT_RDX_SMEP) // https://en.wikipedia.org/wiki/Control_register#SMEP
											// https://web.archive.org/web/20160312223150/http://ncsi.com/nsatc11/presentations/wednesday/emerging_technologies/fischer.pdf
		{
			if (!BSP)
				KPrint("SMEP is supported.");
			debug("SMEP is supported.");
			// cr4.SMEP = 1;
		}
		if (rdx & x64::CPUID_FEAT_RDX_SMAP) // https://en.wikipedia.org/wiki/Supervisor_Mode_Access_Prevention
		{
			if (!BSP)
				KPrint("SMAP is supported.");
			debug("SMAP is supported.");
			// cr4.SMAP = 1;
		}
		if (strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0 &&
			strcmp(Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
		{
			debug("Writing CR4...");
			x64::writecr4(cr4);
			debug("Wrote CR4.");
		}
		else
		{
			if (!BSP)
			{
				if (strcmp(Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) == 0)
					KPrint("VirtualBox detected. Not using UMIP, SMEP & SMAP");
				else if (strcmp(Hypervisor(), x86_CPUID_VENDOR_TCG) == 0)
					KPrint("QEMU (TCG) detected. Not using UMIP, SMEP & SMAP");
			}
		}
		debug("Enabling PAT support...");
		x64::wrmsr(x64::MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
		if (!BSP++)
			trace("Features for BSP initialized.");
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
	}

	uintptr_t Counter()
	{
		// TODO: Get the counter from the x2APIC or any other timer that is available. (TSC is not available on all CPUs)
		uintptr_t Counter;
#if defined(__amd64__)
		asmv("rdtsc"
			 : "=A"(Counter));
#elif defined(__i386__)
		asmv("rdtsc"
			 : "=A"(Counter));
#elif defined(__aarch64__)
		asmv("mrs %0, cntvct_el0"
			 : "=r"(Counter));
#endif
		return Counter;
	}

	x86SIMDType CheckSIMD()
	{
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
#if defined(__amd64__)
			CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(__i386__)
			CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1amd.ECX.SSE4_2)
				return SIMD_SSE42;
			else if (cpuid1amd.ECX.SSE4_1)
				return SIMD_SSE41;
			else if (cpuid1amd.ECX.SSE3)
				return SIMD_SSE3;
			else if (cpuid1amd.EDX.SSE2)
				return SIMD_SSE2;
			else if (cpuid1amd.EDX.SSE)
				return SIMD_SSE;
		}
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
#if defined(__amd64__)
			CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(__i386__)
			CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
				 : "a"(0x1));
#endif
			if (cpuid1intel.ECX.SSE4_2)
				return SIMD_SSE42;
			else if (cpuid1intel.ECX.SSE4_1)
				return SIMD_SSE41;
			else if (cpuid1intel.ECX.SSE3)
				return SIMD_SSE3;
			else if (cpuid1intel.EDX.SSE2)
				return SIMD_SSE2;
			else if (cpuid1intel.EDX.SSE)
				return SIMD_SSE;
		}

		return SIMD_NONE;
	}

	bool CheckSIMD(x86SIMDType Type)
	{
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
#if defined(__amd64__)
			CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(__i386__)
			CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
				 : "a"(0x1));
#endif
			if (Type == SIMD_SSE42)
				return cpuid1amd.ECX.SSE4_2;
			else if (Type == SIMD_SSE41)
				return cpuid1amd.ECX.SSE4_1;
			else if (Type == SIMD_SSE3)
				return cpuid1amd.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid1amd.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid1amd.EDX.SSE;
		}
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
#if defined(__amd64__)
			CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(__i386__)
			CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(__amd64__) || defined(__i386__)
			asmv("cpuid"
				 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
				 : "a"(0x1));
#endif
			if (Type == SIMD_SSE42)
				return cpuid1intel.ECX.SSE4_2;
			else if (Type == SIMD_SSE41)
				return cpuid1intel.ECX.SSE4_1;
			else if (Type == SIMD_SSE3)
				return cpuid1intel.ECX.SSE3;
			else if (Type == SIMD_SSE2)
				return cpuid1intel.EDX.SSE2;
			else if (Type == SIMD_SSE)
				return cpuid1intel.EDX.SSE;
		}

		return false;
	}
}
