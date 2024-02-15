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

#ifndef __FENNIX_KERNEL_CPU_H__
#define __FENNIX_KERNEL_CPU_H__

#include <types.h>

#include <cpu/x86/cpuid_intel.hpp>
#include <cpu/x86/cpuid_amd.hpp>
#include <cpu/x86/x32/cr.hpp>
#include <cpu/x86/x32/msr.hpp>
#include <cpu/x86/x64/cr.hpp>
#include <cpu/x86/x64/msr.hpp>
#include <cpu/x86/exceptions.hpp>
#include <cpu/x86/interrupts.hpp>
#include <cpu/signatures.hpp>
#include <cpu/membar.hpp>
#include <cstring>

/**
 * @brief CPU related functions.
 */
namespace CPU
{
	/**
	 * @brief Enum for CPU::Interrupts() function.
	 */
	enum InterruptsType
	{
		/**
		 * @brief Check if interrupts are enabled.
		 */
		Check,
		/**
		 * @brief Enable interrupts.
		 */
		Enable,
		/**
		 * @brief Disable interrupts.
		 */
		Disable
	};

	enum x86SIMDType
	{
		SIMD_NONE = (1 << 0),

		SIMD_SSE = (1 << 1),
		SIMD_SSE2 = (1 << 2),
		SIMD_SSE3 = (1 << 3),
		SIMD_SSSE3 = (1 << 4),
		SIMD_SSE41 = (1 << 5),
		SIMD_SSE42 = (1 << 6),

		SIMD_AVX = (1 << 7),
		SIMD_AVX2 = (1 << 8),
		SIMD_AVX512 = (1 << 9),

		SIMD_AVX512F = (1 << 10),
		SIMD_AVX512CD = (1 << 11),
		SIMD_AVX512ER = (1 << 12),
		SIMD_AVX512PF = (1 << 13),

		SIMD_AVX512VL = (1 << 14),
		SIMD_AVX512DQ = (1 << 16),
		SIMD_AVX512BW = (1 << 15),

		SIMD_AVX512IFMA = (1 << 17),
		SIMD_AVX512VBMI = (1 << 18),

		SIMD_AVX5124VNNIW = (1 << 19),
		SIMD_AVX5124FMAPS = (1 << 20),

		SIMD_AVX512VPOPCNTDQ = (1 << 21),

		SIMD_AVX512VNNI = (1 << 22),
		SIMD_AVX512VBMI2 = (1 << 23),
		SIMD_AVX512BITALG = (1 << 24),

		SIMD_AVX512VP2INTERSECT = (1 << 25),

		SIMD_AVX512GFNI = (1 << 26),
		SIMD_AVX512VPCLMULQDQ = (1 << 27),
		SIMD_AVX512VAES = (1 << 28),
	};

	/**
	 * @brief Get CPU vendor identifier.
	 *
	 * @return CPU Vendor ID.
	 */
	const char *Vendor();

	/**
	 * @brief Get CPU name.
	 *
	 * @return CPU Name.
	 */
	const char *Name();

	/**
	 * @brief Get CPU hypervisor vendor.
	 *
	 * @return Hypervisor vendor.
	 */
	const char *Hypervisor();

	/**
	 * @brief Check SIMD support. It will return the highest supported SIMD type.
	 *
	 * @return x86SIMDType flags.
	 */
	uint64_t CheckSIMD();

	/**
	 * @brief Check SIMD support.
	 *
	 * @param Type SIMD type.
	 * @return true if supported.
	 * @return false if not supported.
	 */
	bool CheckSIMD(x86SIMDType Type);

	/**
	 * @brief Pause the CPU
	 */
	nsa static __always_inline inline void Pause(bool Loop = false)
	{
		do
		{
#if defined(a86)
			asmv("pause");
#elif defined(aa64)
			asmv("yield");
#endif
		} while (Loop);
	}

	/**
	 * @brief Stop the CPU (infinite loop)
	 */
	nsa __noreturn __used inline void Stop()
	{
#if defined(a86)
		asmv("CPUStopLoop:\n"
			 "cli\n"
			 "hlt\n"
			 "jmp CPUStopLoop");
#elif defined(aa64)
		asmv("CPUStopLoop:\n"
			 "cpsid i\n"
			 "wfe\n"
			 "wfi\n"
			 "b CPUStopLoop");
#endif
		__builtin_unreachable();
	}

	/**
	 * @brief Halt the CPU
	 */
	nsa static __always_inline inline void Halt(bool Loop = false)
	{
		do
		{
#if defined(a86)
			asmv("hlt");
#elif defined(aa64)
			asmv("wfe");
#endif
		} while (Loop);
	}

	/**
	 * @brief Check if interrupts are enabled
	 *
	 * @return true If InterruptsType::Check and interrupts are enabled, or if other InterruptsType were executed successfully
	 * @return false If InterruptsType::Check and interrupts are disabled, or if other InterruptsType failed
	 */
	bool Interrupts(InterruptsType Type = Check);

	/**
	 * @brief Get/Set the CPU's page table
	 *
	 * @param PT The new page table, if empty, the current page table will be returned
	 * @return Get: The current page table
	 * @return Set: The old page table
	 */
	void *PageTable(void *PT = nullptr);

#define thisPageTable (Memory::PageTable *)CPU::PageTable()

	/** @brief To be used only once. */
	void InitializeFeatures(int Core);

	/** @brief Get CPU counter value. */
	uint64_t Counter();

	namespace x32
	{
		/**
		 * @brief MSR_APIC_BASE structure
		 * @see MSR_APIC_BASE
		 */
		typedef union
		{
			struct
			{
				/** @brief Reserved */
				uint32_t Reserved0 : 8;
				/**
				 * @brief BSP Flag
				 * @details If the BSP flag is set to 1, the processor is the bootstrap processor.
				 */
				uint32_t BSP : 1;
				/** @brief Reserved */
				uint32_t Reserved1 : 1;
				/** @brief Enable x2APIC mode */
				uint32_t EXTD : 1;
				/** @brief APIC Global Enable */
				uint32_t EN : 1;
				/** @brief APIC Base Low Address */
				uint32_t ApicBaseLo : 20;
				/** @brief APIC Base High Address */
				uint32_t ApicBaseHi : 32;
			};
			uint64_t raw;
		} __packed APIC_BASE;

		typedef union
		{
			struct
			{
				/** @brief Carry Flag */
				uint32_t CF : 1;
				/** @brief Reserved */
				uint32_t AlwaysOne : 1;
				/** @brief Parity Flag */
				uint32_t PF : 1;
				/** @brief Reserved */
				uint32_t Reserved0 : 1;
				/** @brief Auxiliary Carry Flag */
				uint32_t AF : 1;
				/** @brief Reserved */
				uint32_t Reserved1 : 1;
				/** @brief Zero Flag */
				uint32_t ZF : 1;
				/** @brief Sign Flag */
				uint32_t SF : 1;
				/** @brief Trap Flag */
				uint32_t TF : 1;
				/** @brief Interrupt Enable Flag */
				uint32_t IF : 1;
				/** @brief Direction Flag */
				uint32_t DF : 1;
				/** @brief Overflow Flag */
				uint32_t OF : 1;
				/** @brief I/O Privilege Level */
				uint32_t IOPL : 2;
				/** @brief Nested Task */
				uint32_t NT : 1;
				/** @brief Reserved */
				uint32_t Reserved2 : 1;
				/** @brief Resume Flag */
				uint32_t RF : 1;
				/** @brief Virtual 8086 Mode */
				uint32_t VM : 1;
				/** @brief Alignment Check */
				uint32_t AC : 1;
				/** @brief Virtual Interrupt Flag */
				uint32_t VIF : 1;
				/** @brief Virtual Interrupt Pending */
				uint32_t VIP : 1;
				/** @brief ID Flag */
				uint32_t ID : 1;
			};
			uint32_t raw;
		} EFLAGS;

		struct TrapFrame
		{
			uint32_t edi; // Destination index for string operations
			uint32_t esi; // Source index for string operations
			uint32_t ebp; // Base Pointer (meant for stack frames)
			uint32_t esp; // Stack Pointer
			uint32_t ebx; // Base
			uint32_t edx; // Data (commonly extends the A register)
			uint32_t ecx; // Counter
			uint32_t eax; // Accumulator

			uint32_t InterruptNumber; // Interrupt Number
			uint32_t ErrorCode;		  // Error code

			uint32_t eip;  // Instruction Pointer
			uint32_t cs;   // Code Segment
			EFLAGS eflags; // Register Flags

			uint32_t r3_esp; // Stack Pointer
			uint32_t r3_ss;	 // Stack Segment
		};

		typedef union DR6
		{
			struct
			{
				/** @brief Breakpoint #0 Condition Detected */
				uint64_t B0 : 1;
				/** @brief Breakpoint #1 Condition Detected */
				uint64_t B1 : 1;
				/** @brief Breakpoint #2 Condition Detected */
				uint64_t B2 : 1;
				/** @brief Breakpoint #3 Condition Detected */
				uint64_t B3 : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 8;
				/** @brief Reserved */
				uint64_t Reserved1 : 1;
				/** @brief Breakpoint Debug Access Detected */
				uint64_t BD : 1;
				/** @brief Breakpoint Single Step */
				uint64_t BS : 1;
				/** @brief Breakpoint Task Switch */
				uint64_t BT : 1;
				/** @brief Reserved */
				uint64_t Reserved2 : 15;
			};
			uint64_t raw;
		} DR6;

		typedef union DR7
		{
			struct
			{
				/** @brief Local Exact Breakpoint #0 Enabled */
				uint32_t L0 : 1;
				/** @brief Global Exact Breakpoint #0 Enabled */
				uint32_t G0 : 1;
				/** @brief Local Exact Breakpoint #1 Enabled */
				uint32_t L1 : 1;
				/** @brief Global Exact Breakpoint #1 Enabled */
				uint32_t G1 : 1;
				/** @brief Local Exact Breakpoint #2 Enabled */
				uint32_t L2 : 1;
				/** @brief Global Exact Breakpoint #2 Enabled */
				uint32_t G2 : 1;
				/** @brief Local Exact Breakpoint #3 Enabled */
				uint32_t L3 : 1;
				/** @brief Global Exact Breakpoint #3 Enabled */
				uint32_t G3 : 1;
				/** @brief Local Exact Breakpoint Enabled */
				uint32_t LE : 1;
				/** @brief Global Exact Breakpoint Enabled */
				uint32_t GE : 1;
				/** @brief Reserved */
				uint32_t Reserved0 : 1;
				/** @brief Reserved */
				uint32_t Reserved1 : 2;
				/** @brief General Detect Enabled */
				uint32_t GD : 1;
				/** @brief Reserved */
				uint32_t Reserved2 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint32_t RW0 : 2;
				/** @brief Length of Breakpoint #0 */
				uint32_t LEN0 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint32_t RW1 : 2;
				/** @brief Length of Breakpoint #1 */
				uint32_t LEN1 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint32_t RW2 : 2;
				/** @brief Length of Breakpoint #2 */
				uint32_t LEN2 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint32_t RW3 : 2;
				/** @brief Length of Breakpoint #3 */
				uint32_t LEN3 : 2;
			};
			uint32_t raw;
		} DR7;

		struct FXState
		{
			/** @brief FPU control word */
			uint16_t fcw;
			/** @brief FPU status word */
			uint16_t fsw;
			/** @brief FPU tag words */
			uint8_t ftw;
			/** @brief Reserved (zero) */
			uint8_t Reserved;
			/** @brief FPU opcode */
			uint16_t fop;
			/** @brief PFU instruction pointer */
			uint64_t rip;
			/** @brief FPU data pointer */
			uint64_t rdp;
			/** @brief SSE control register */
			uint32_t mxcsr;
			/** @brief SSE control register mask */
			uint32_t mxcsrmask;
			/** @brief FPU registers (last 6 bytes reserved) */
			uint8_t st[8][16];
			/** @brief XMM registers */
			uint8_t xmm[16][16];
		} __packed;

		/**
		 * @brief CPUID
		 *
		 * @param Function Leaf
		 * @param eax EAX
		 * @param ebx EBX
		 * @param ecx ECX
		 * @param edx EDX
		 */
		static inline void cpuid(uint32_t Function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
		{
#ifdef a32
			asmv("cpuid"
				 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
				 : "a"(Function));
#else
			UNUSED(Function);
			UNUSED(eax);
			UNUSED(ebx);
			UNUSED(ecx);
			UNUSED(edx);
#endif
		}

		nsa static inline void lgdt(void *gdt)
		{
#ifdef a32
			asmv("lgdt (%0)"
				 :
				 : "r"(gdt));
#else
			UNUSED(gdt);
#endif
		}

		nsa static inline void lidt(void *idt)
		{
#ifdef a32
			asmv("lidt (%0)"
				 :
				 : "r"(idt));
#else
			UNUSED(idt);
#endif
		}

		nsa static inline void ltr(uint16_t Segment)
		{
#ifdef a32
			asmv("ltr %0"
				 :
				 : "r"(Segment));
#else
			UNUSED(Segment);
#endif
		}

		nsa static inline void invlpg(void *Address)
		{
#ifdef a32
			asmv("invlpg (%0)"
				 :
				 : "r"(Address)
				 : "memory");
#else
			UNUSED(Address);
#endif
		}

		nsa static inline void fxsave(void *FXSaveArea)
		{
#ifdef a32
			if (!FXSaveArea)
				return;

			asmv("fxsave (%0)"
				 :
				 : "r"(FXSaveArea)
				 : "memory");
#else
			UNUSED(FXSaveArea);
#endif
		}

		nsa static inline void fxrstor(void *FXRstorArea)
		{
#ifdef a32
			if (!FXRstorArea)
				return;

			asmv("fxrstor (%0)"
				 :
				 : "r"(FXRstorArea)
				 : "memory");
#else
			UNUSED(FXRstorArea);
#endif
		}
	}

	namespace x64
	{
		/**
		 * @brief MSR_APIC_BASE structure
		 * @see MSR_APIC_BASE
		 */
		typedef union
		{
			struct
			{
				/** Reserved */
				uint64_t Reserved0 : 8;
				/** Boot Strap CPU Core */
				uint64_t BSC : 1;
				/** Reserved */
				uint64_t Reserved1 : 1;
				/** x2APIC Mode Enable */
				uint64_t EXTD : 1;
				/** APIC Enable */
				uint64_t AE : 1;
				/** @brief APIC Base Low Address */
				uint64_t ABALow : 20;
				/** @brief APIC Base High Address */
				uint64_t ABAHigh : 32;
				/** Reserved */
				uint64_t Reserved2 : 12;
			};
			uint64_t raw;
		} __packed APIC_BASE;

		typedef union
		{
			struct
			{
				uint64_t Reserved : 24;
				uint64_t AID : 8;
			};
			uint32_t raw;
		} __packed APIC_ID;

		typedef union
		{
			struct
			{
				/** @brief Carry Flag */
				uint64_t CF : 1;
				/** @brief Reserved */
				uint64_t AlwaysOne : 1;
				/** @brief Parity Flag */
				uint64_t PF : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 1;
				/** @brief Auxiliary Carry Flag */
				uint64_t AF : 1;
				/** @brief Reserved */
				uint64_t Reserved1 : 1;
				/** @brief Zero Flag */
				uint64_t ZF : 1;
				/** @brief Sign Flag */
				uint64_t SF : 1;
				/** @brief Trap Flag */
				uint64_t TF : 1;
				/** @brief Interrupt Enable Flag */
				uint64_t IF : 1;
				/** @brief Direction Flag */
				uint64_t DF : 1;
				/** @brief Overflow Flag */
				uint64_t OF : 1;
				/** @brief I/O Privilege Level */
				uint64_t IOPL : 2;
				/** @brief Nested Task */
				uint64_t NT : 1;
				/** @brief Reserved */
				uint64_t Reserved2 : 1;
				/** @brief Resume Flag */
				uint64_t RF : 1;
				/** @brief Virtual 8086 Mode */
				uint64_t VM : 1;
				/** @brief Alignment Check */
				uint64_t AC : 1;
				/** @brief Virtual Interrupt Flag */
				uint64_t VIF : 1;
				/** @brief Virtual Interrupt Pending */
				uint64_t VIP : 1;
				/** @brief ID Flag (Allow using CPUID instruction) */
				uint64_t ID : 1;
				/** @brief Reserved */
				uint64_t Reserved3 : 10;
			};
			uint64_t raw;
		} RFLAGS;

		struct TrapFrame
		{
			uint64_t r15; /* General purpose */
			uint64_t r14; /* General purpose */
			uint64_t r13; /* General purpose */
			uint64_t r12; /* General purpose */
			uint64_t r11; /* General purpose */
			uint64_t r10; /* General purpose */
			uint64_t r9;  /* General purpose */
			uint64_t r8;  /* General purpose */

			uint64_t rbp; /* Base Pointer (meant for stack frames) */
			uint64_t rdi; /* Destination index for string operations */
			uint64_t rsi; /* Source index for string operations */
			uint64_t rdx; /* Data (commonly extends the A register) */
			uint64_t rcx; /* Counter */
			uint64_t rbx; /* Base */
			uint64_t rax; /* Accumulator */

			uint64_t InterruptNumber; /* Interrupt Number */
			uint64_t ErrorCode;		  /* Error code */

			uint64_t rip;  /* Instruction Pointer */
			uint64_t cs;   /* Code Segment */
			RFLAGS rflags; /* Register Flags */
			uint64_t rsp;  /* Stack Pointer */
			uint64_t ss;   /* Stack Segment */
		};

		struct ExceptionFrame
		{
			uint64_t cr0; /* Control Register 0 (system control) */
			uint64_t cr2; /* Control Register 2 (page fault linear address) */
			uint64_t cr3; /* Control Register 3 (page directory base) */
			uint64_t cr4; /* Control Register 4 (system control) */
			uint64_t cr8; /* Control Register 8 (task priority) */

			uint64_t dr0; /* Debug Register */
			uint64_t dr1; /* Debug Register */
			uint64_t dr2; /* Debug Register */
			uint64_t dr3; /* Debug Register */
			uint64_t dr6; /* Debug Register */
			uint64_t dr7; /* Debug Register */

			uint64_t gs; /* General purpose */
			uint64_t fs; /* General purpose */
			uint64_t es; /* Extra Segment */
			uint64_t ds; /* Data Segment */

			uint64_t r15; /* General purpose */
			uint64_t r14; /* General purpose */
			uint64_t r13; /* General purpose */
			uint64_t r12; /* General purpose */
			uint64_t r11; /* General purpose */
			uint64_t r10; /* General purpose */
			uint64_t r9;  /* General purpose */
			uint64_t r8;  /* General purpose */

			uint64_t rbp; /* Base Pointer (meant for stack frames) */
			uint64_t rdi; /* Destination index for string operations */
			uint64_t rsi; /* Source index for string operations */
			uint64_t rdx; /* Data (commonly extends the A register) */
			uint64_t rcx; /* Counter */
			uint64_t rbx; /* Base */
			uint64_t rax; /* Accumulator */

			uint64_t InterruptNumber; /* Interrupt Number */
			uint64_t ErrorCode;		  /* Error code */

			uint64_t rip;  /* Instruction Pointer */
			uint64_t cs;   /* Code Segment */
			RFLAGS rflags; /* Register Flags */
			uint64_t rsp;  /* Stack Pointer */
			uint64_t ss;   /* Stack Segment */
		};

		typedef union EFER
		{
			struct
			{
				/** @brief Enable syscall & sysret instructions in 64-bit mode. */
				uint64_t SCE : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 7;
				/** @brief Enable long mode. */
				uint64_t LME : 1;
				/** @brief Reserved */
				uint64_t Reserved1 : 1;
				/** @brief Indicates long. */
				uint64_t LMA : 1;
				/** @brief Enable No-Execute Bit */
				uint64_t NXE : 1;
				/** @brief Enable Secure Virtual Machine */
				uint64_t SVME : 1;
				/** @brief Enable Long Mode Segment Limit */
				uint64_t LMSLE : 1;
				/** @brief Enable Fast FXSAVE/FXRSTOR */
				uint64_t FFXSR : 1;
				/** @brief Enable Translation Cache Extension */
				uint64_t TCE : 1;
				/** @brief Reserved */
				uint64_t Reserved2 : 32;
			};
			uint64_t raw;
		} __packed EFER;

		typedef union DR6
		{
			struct
			{
				/** @brief Breakpoint #0 Condition Detected */
				uint64_t B0 : 1;
				/** @brief Breakpoint #1 Condition Detected */
				uint64_t B1 : 1;
				/** @brief Breakpoint #2 Condition Detected */
				uint64_t B2 : 1;
				/** @brief Breakpoint #3 Condition Detected */
				uint64_t B3 : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 8;
				/** @brief Reserved */
				uint64_t Reserved1 : 1;
				/** @brief Breakpoint Debug Access Detected */
				uint64_t BD : 1;
				/** @brief Breakpoint Single Step */
				uint64_t BS : 1;
				/** @brief Breakpoint Task Switch */
				uint64_t BT : 1;
				/** @brief Reserved */
				uint64_t Reserved2 : 15;
				/** @brief Reserved */
				uint64_t Reserved3 : 32;
			};
			uint64_t raw;
		} DR6;

		typedef union DR7
		{
			struct
			{
				/** @brief Local Exact Breakpoint #0 Enabled */
				uint64_t L0 : 1;
				/** @brief Global Exact Breakpoint #0 Enabled */
				uint64_t G0 : 1;
				/** @brief Local Exact Breakpoint #1 Enabled */
				uint64_t L1 : 1;
				/** @brief Global Exact Breakpoint #1 Enabled */
				uint64_t G1 : 1;
				/** @brief Local Exact Breakpoint #2 Enabled */
				uint64_t L2 : 1;
				/** @brief Global Exact Breakpoint #2 Enabled */
				uint64_t G2 : 1;
				/** @brief Local Exact Breakpoint #3 Enabled */
				uint64_t L3 : 1;
				/** @brief Global Exact Breakpoint #3 Enabled */
				uint64_t G3 : 1;
				/** @brief Local Exact Breakpoint Enabled */
				uint64_t LE : 1;
				/** @brief Global Exact Breakpoint Enabled */
				uint64_t GE : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 1;
				/** @brief Reserved */
				uint64_t Reserved1 : 2;
				/** @brief General Detect Enabled */
				uint64_t GD : 1;
				/** @brief Reserved */
				uint64_t Reserved2 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint64_t RW0 : 2;
				/** @brief Length of Breakpoint #0 */
				uint64_t LEN0 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint64_t RW1 : 2;
				/** @brief Length of Breakpoint #1 */
				uint64_t LEN1 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint64_t RW2 : 2;
				/** @brief Length of Breakpoint #2 */
				uint64_t LEN2 : 2;
				/** @brief Type of Transaction(s) to Trap */
				uint64_t RW3 : 2;
				/** @brief Length of Breakpoint #3 */
				uint64_t LEN3 : 2;
				/** @brief Reserved */
				uint64_t Reserved3 : 32;
			};
			uint64_t raw;
		} DR7;

		typedef union PageFaultErrorCode
		{
			struct
			{
				/** @brief When set, the page fault was caused by a page-protection violation. When not set, it was caused by a non-present page. */
				uint64_t P : 1;
				/** @brief When set, the page fault was caused by a write access. When not set, it was caused by a read access. */
				uint64_t W : 1;
				/** @brief When set, the page fault was caused while CPL = 3. This does not necessarily mean that the page fault was a privilege violation. */
				uint64_t U : 1;
				/** @brief When set, one or more page directory entries contain reserved bits which are set to 1. This only applies when the PSE or PAE flags in CR4 are set to 1. */
				uint64_t R : 1;
				/** @brief When set, the page fault was caused by an instruction fetch. This only applies when the No-Execute bit is supported and enabled. */
				uint64_t I : 1;
				/** @brief When set, the page fault was caused by a protection-key violation. The PKRU register (for user-mode accesses) or PKRS MSR (for supervisor-mode accesses) specifies the protection key rights. */
				uint64_t PK : 1;
				/** @brief When set, the page fault was caused by a shadow stack access. */
				uint64_t SS : 1;
				/** @brief Reserved */
				uint64_t Reserved0 : 8;
				/** @brief When set, the fault was due to an SGX violation. The fault is unrelated to ordinary paging. */
				uint64_t SGX : 1;
				/** @brief Reserved */
				uint64_t Reserved1 : 16;
			};
			uint64_t raw;
		} PageFaultErrorCode;

		// ! TODO: UNTESTED!
		typedef union SelectorErrorCode
		{
			struct
			{
				/** @brief When set, the exception originated externally to the processor. */
				uint64_t External : 1;
				/** @brief IDT/GDT/LDT Table
				 *  @details 0b00 - The Selector Index references a descriptor in the GDT.
				 *  @details 0b01 - The Selector Index references a descriptor in the IDT.
				 *  @details 0b10 - The Selector Index references a descriptor in the LDT.
				 *  @details 0b11 - The Selector Index references a descriptor in the IDT.
				 */
				uint64_t Table : 2;
				/** @brief The index in the GDT, IDT or LDT. */
				uint64_t Idx : 13;
				/** @brief Reserved */
				uint64_t Reserved : 16;
			};
			uint64_t raw;
		} SelectorErrorCode;

		struct FXState
		{
			/** @brief FPU control word */
			uint16_t fcw;
			/** @brief FPU status word */
			uint16_t fsw;
			/** @brief FPU tag words */
			uint8_t ftw;
			/** @brief Reserved (zero) */
			uint8_t Reserved;
			/** @brief FPU opcode */
			uint16_t fop;
			/** @brief PFU instruction pointer */
			uint64_t rip;
			/** @brief FPU data pointer */
			uint64_t rdp;
			/** @brief SSE control register */
			uint32_t mxcsr;
			/** @brief SSE control register mask */
			uint32_t mxcsrmask;
			/** @brief FPU registers (last 6 bytes reserved) */
			uint8_t st[8][16];
			/** @brief XMM registers */
			uint8_t xmm[16][16];
		} __packed __aligned(16);

		nsa static inline void lgdt(void *gdt)
		{
#ifdef a64
			asmv("lgdt (%0)"
				 :
				 : "r"(gdt));
#endif
		}

		nsa static inline void lidt(void *idt)
		{
#ifdef a64
			asmv("lidt (%0)"
				 :
				 : "r"(idt));
#endif
		}

		nsa static inline void ltr(uint16_t Segment)
		{
#ifdef a64
			asmv("ltr %0"
				 :
				 : "r"(Segment));
#endif
		}

		nsa static inline void invlpg(void *Address)
		{
#ifdef a64
			asmv("invlpg (%0)"
				 :
				 : "r"(Address)
				 : "memory");
#endif
		}

		/**
		 * @brief CPUID
		 *
		 * @param Function Leaf
		 * @param eax EAX
		 * @param ebx EBX
		 * @param ecx ECX
		 * @param edx EDX
		 */
		nsa static inline void cpuid(uint32_t Function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
		{
#ifdef a64
			asmv("cpuid"
				 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
				 : "a"(Function));
#endif
		}

		/**
		 * @brief Get the highest leaf function supported by CPUID
		 *
		 * @example if (GetHighestLeaf() < 0x15) { error("CPU doesn't support leaf 0x15!"); }
		 *
		 * @return uint32_t
		 */
		nsa static inline uint32_t GetHighestLeaf()
		{
			uint32_t eax, ebx, ecx, edx;
			cpuid(0x0, &eax, &ebx, &ecx, &edx);
			return eax;
		}

		nsa static inline void fxsave(void *FXSaveArea)
		{
#ifdef a64
			if (!FXSaveArea || FXSaveArea >= (char *)0xfffffffffffff000)
				return;

			asmv("fxsaveq (%0)"
				 :
				 : "r"(FXSaveArea)
				 : "memory");
#endif
		}

		nsa static inline void fxrstor(void *FXRstorArea)
		{
#ifdef a64
			if (!FXRstorArea || FXRstorArea >= (char *)0xfffffffffffff000)
				return;

			asmv("fxrstorq (%0)"
				 :
				 : "r"(FXRstorArea)
				 : "memory");
#endif
		}
	}

	namespace aarch64
	{
		struct TrapFrame
		{
			uint64_t x0;  // General purpose
			uint64_t x1;  // General purpose
			uint64_t x2;  // General purpose
			uint64_t x3;  // General purpose
			uint64_t x4;  // General purpose
			uint64_t x5;  // General purpose
			uint64_t x6;  // General purpose
			uint64_t x7;  // General purpose
			uint64_t x8;  // General purpose
			uint64_t x9;  // General purpose
			uint64_t x10; // General purpose
			uint64_t x11; // General purpose
			uint64_t x12; // General purpose
			uint64_t x13; // General purpose
			uint64_t x14; // General purpose
			uint64_t x15; // General purpose
			uint64_t x16; // General purpose
			uint64_t x17; // General purpose
			uint64_t x18; // General purpose
			uint64_t x19; // General purpose
			uint64_t x20; // General purpose
			uint64_t x21; // General purpose
			uint64_t x22; // General purpose
			uint64_t x23; // General purpose
			uint64_t x24; // General purpose
			uint64_t x25; // General purpose
			uint64_t x26; // General purpose
			uint64_t x27; // General purpose
			uint64_t x28; // General purpose

			uint64_t fp; // Frame Pointer (meant for stack frames)
			uint64_t lr; // Link Register

			uint64_t sp;	 // Stack Pointer
			uint64_t pc;	 // Program Counter
			uint64_t pstate; // Processor State (flags)
			uint64_t esr;	 // Exception Syndrome Register
			uint64_t far;	 // Fault Address Register
		};
	}

#if defined(a64)
	/**
	 * CPU trap frame for the current architecture
	 *
	 * @note This is for x86_64
	 */
	typedef x64::TrapFrame TrapFrame;
	typedef x64::ExceptionFrame ExceptionFrame;
#elif defined(a32)
	/**
	 * CPU trap frame for the current architecture
	 *
	 * @note This is for x86_32
	 */
	typedef x32::TrapFrame TrapFrame;
	typedef x32::ExceptionFrame ExceptionFrame;
#elif defined(aa64)
	/**
	 * CPU trap frame for the current architecture
	 *
	 * @note This is for aarch64
	 */
	typedef aarch64::TrapFrame TrapFrame;
	typedef aarch64::TrapFrame ExceptionFrame;
#endif
}

#endif // !__FENNIX_KERNEL_CPU_H__
