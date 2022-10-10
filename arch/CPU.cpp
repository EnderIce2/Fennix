#include <cpu.hpp>
#include <memory.hpp>

namespace CPU
{
	char *Vendor()
	{
		static char Vendor[16];
#if defined(__amd64__)
		asmv("cpuid"
			 : "=a"(Vendor[0]), "=b"(Vendor[4]), "=c"(Vendor[8]), "=d"(Vendor[12])
			 : "a"(0));
#elif defined(__i386__)
		asmv("cpuid"
			 : "=a"(Vendor[0]), "=b"(Vendor[4]), "=c"(Vendor[8]), "=d"(Vendor[12])
			 : "a"(0));
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
		asmv("cpuid"
			 : "=a"(Name[0]), "=b"(Name[4]), "=c"(Name[8]), "=d"(Name[12])
			 : "a"(0x80000002));
		asmv("cpuid"
			 : "=a"(Name[16]), "=b"(Name[20]), "=c"(Name[24]), "=d"(Name[28])
			 : "a"(0x80000003));
		asmv("cpuid"
			 : "=a"(Name[32]), "=b"(Name[36]), "=c"(Name[40]), "=d"(Name[44])
			 : "a"(0x80000004));
#elif defined(__i386__)
		asmv("cpuid"
			 : "=a"(Name[0]), "=b"(Name[4]), "=c"(Name[8]), "=d"(Name[12])
			 : "a"(0x80000002));
		asmv("cpuid"
			 : "=a"(Name[16]), "=b"(Name[20]), "=c"(Name[24]), "=d"(Name[28])
			 : "a"(0x80000003));
		asmv("cpuid"
			 : "=a"(Name[32]), "=b"(Name[36]), "=c"(Name[40]), "=d"(Name[44])
			 : "a"(0x80000004));
#elif defined(__aarch64__)
		asmv("mrs %0, MIDR_EL1"
			 : "=r"(Name[0]));
#endif
		return Name;
	}

	char *Hypervisor()
	{
		static char Hypervisor[16];
#if defined(__amd64__)
		asmv("cpuid"
			 : "=a"(Hypervisor[0]), "=b"(Hypervisor[4]), "=c"(Hypervisor[8]), "=d"(Hypervisor[12])
			 : "a"(0x40000000));
#elif defined(__i386__)
		asmv("cpuid"
			 : "=a"(Hypervisor[0]), "=b"(Hypervisor[4]), "=c"(Hypervisor[8]), "=d"(Hypervisor[12])
			 : "a"(0x40000000));
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
}
