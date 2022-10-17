#include <cpu.hpp>

#include <memory.hpp>
#include <string.h>
#include <debug.h>

#include "../kernel.h"

namespace CPU
{
	char *Vendor()
	{
		static char Vendor[13];
#if defined(__amd64__)
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x0, &rax, &rbx, &rcx, &rdx);
		memcpy(Vendor + 0, &rbx, 4);
		memcpy(Vendor + 4, &rdx, 4);
		memcpy(Vendor + 8, &rcx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x0, &rax, &rbx, &rcx, &rdx);
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
		static char Name[48];
#if defined(__amd64__)
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x80000002, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 0, &rax, 4);
		memcpy(Name + 4, &rbx, 4);
		memcpy(Name + 8, &rcx, 4);
		memcpy(Name + 12, &rdx, 4);
		CPU::x64::cpuid(0x80000003, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 16, &rax, 4);
		memcpy(Name + 20, &rbx, 4);
		memcpy(Name + 24, &rcx, 4);
		memcpy(Name + 28, &rdx, 4);
		CPU::x64::cpuid(0x80000004, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 32, &rax, 4);
		memcpy(Name + 36, &rbx, 4);
		memcpy(Name + 40, &rcx, 4);
		memcpy(Name + 44, &rdx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x80000002, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 0, &rax, 4);
		memcpy(Name + 4, &rbx, 4);
		memcpy(Name + 8, &rcx, 4);
		memcpy(Name + 12, &rdx, 4);
		CPU::x64::cpuid(0x80000003, &rax, &rbx, &rcx, &rdx);
		memcpy(Name + 16, &rax, 4);
		memcpy(Name + 20, &rbx, 4);
		memcpy(Name + 24, &rcx, 4);
		memcpy(Name + 28, &rdx, 4);
		CPU::x64::cpuid(0x80000004, &rax, &rbx, &rcx, &rdx);
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
		CPU::x64::cpuid(0x40000000, &rax, &rbx, &rcx, &rdx);
		memcpy(Hypervisor + 0, &rbx, 4);
		memcpy(Hypervisor + 4, &rcx, 4);
		memcpy(Hypervisor + 8, &rdx, 4);
#elif defined(__i386__)
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x40000000, &rax, &rbx, &rcx, &rdx);
		memcpy(Hypervisor + 0, &rbx, 4);
		memcpy(Hypervisor + 4, &rcx, 4);
		memcpy(Hypervisor + 8, &rdx, 4);
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Hypervisor[0]));
#endif
		return Hypervisor;
	}

	void Pause()
	{
#if defined(__amd64__) || defined(__i386__)
		asmv("pause");
#elif defined(__aarch64__)
		asmv("yield");
#endif
	}

	void Stop()
	{
		while (1)
		{
#if defined(__amd64__) || defined(__i386__)
			asmv("cli");
			asmv("hlt");
#elif defined(__aarch64__)
			asmv("msr daifset, #2");
			asmv("wfe");
#endif
		}
	}

	void Halt()
	{
#if defined(__amd64__) || defined(__i386__)
		asmv("hlt");
#elif defined(__aarch64__)
		asmv("wfe");
#endif
	}

	bool Interrupts(InterruptsType Type)
	{
		switch (Type)
		{
		case Check:
		{
#if defined(__amd64__)
			uint64_t rflags;
			asmv("pushfq");
			asmv("popq %0"
				 : "=r"(rflags));
			return rflags & (1 << 9);
#elif defined(__i386__)
			uint32_t rflags;
			asmv("pushfl");
			asmv("popl %0"
				 : "=r"(rflags));
			return rflags & (1 << 9);
#elif defined(__aarch64__)
			uint64_t daif;
			asmv("mrs %0, daif"
				 : "=r"(daif));
			return !(daif & (1 << 2));
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
		if (!BSP)
			CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, 0);
		CPU::x64::CR0 cr0 = CPU::x64::readcr0();
		CPU::x64::CR4 cr4 = CPU::x64::readcr4();
		uint32_t rax, rbx, rcx, rdx;
		CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
		if (rdx & CPU::x64::CPUID_FEAT_RDX_SSE)
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
		// Enable cpu cache but... how to use it?
		cr0.NW = 0;
		cr0.CD = 0;

		CPU::x64::writecr0(cr0);

		debug("Enabling UMIP, SMEP & SMAP support...");
		CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
		if (rdx & CPU::x64::CPUID_FEAT_RDX_UMIP)
		{
			if (!BSP)
				KPrint("UMIP is supported.");
			fixme("Not going to enable UMIP.");
			// cr4.UMIP = 1;
		}
		if (rdx & CPU::x64::CPUID_FEAT_RDX_SMEP)
		{
			if (!BSP)
				KPrint("SMEP is supported.");
			cr4.SMEP = 1;
		}
		if (rdx & CPU::x64::CPUID_FEAT_RDX_SMAP)
		{
			if (!BSP)
				KPrint("SMAP is supported.");
			cr4.SMAP = 1;
		}
		if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0 &&
			strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
			CPU::x64::writecr4(cr4);
		else
		{
			if (!BSP)
			{
				if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) != 0)
					KPrint("VirtualBox detected. Not using UMIP, SMEP & SMAP");
				else if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
					KPrint("QEMU (TCG) detected. Not using UMIP, SMEP & SMAP");
			}
		}
		debug("Enabling PAT support...");
		CPU::x64::wrmsr(CPU::x64::MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
		if (!BSP++)
			trace("Features for BSP initialized.");
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
	}
}
