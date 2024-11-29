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

#ifndef __FENNIX_KERNEL_CPU_x64_CR_H__
#define __FENNIX_KERNEL_CPU_x64_CR_H__

#include <types.h>

namespace CPU
{
	namespace x64
	{
		typedef union CR0
		{
			struct
			{
				/** Protection Enable */
				uint64_t PE : 1;
				/** Monitor Coprocessor */
				uint64_t MP : 1;
				/** Emulation */
				uint64_t EM : 1;
				/** Task Switched */
				uint64_t TS : 1;
				/** Extension Type */
				uint64_t ET : 1;
				/** Numeric Error */
				uint64_t NE : 1;
				/** Reserved */
				uint64_t Reserved0 : 10;
				/** Write Protect */
				uint64_t WP : 1;
				/** Reserved */
				uint64_t Reserved1 : 1;
				/** Alignment Mask */
				uint64_t AM : 1;
				/** Reserved */
				uint64_t Reserved2 : 10;
				/** Not Write-through */
				uint64_t NW : 1;
				/** Cache Disable */
				uint64_t CD : 1;
				/** Paging */
				uint64_t PG : 1;
			};
			uint64_t raw;
		} CR0;

		typedef union CR2
		{
			struct
			{
				/** Page Fault Linear Address */
				uint64_t PFLA;
			};
			uint64_t raw;
		} CR2;

		typedef union CR3
		{
			struct
			{
				/** Not used if bit 17 of CR4 is 1 */
				uint64_t PWT : 1;
				/** Not used if bit 17 of CR4 is 1 */
				uint64_t PCD : 1;
				/** Base of PML4T/PML5T */
				uint64_t PDBR;
			};
			uint64_t raw;
		} CR3;

		typedef union CR4
		{
			struct
			{
				/** Virtual-8086 Mode Extensions */
				uint64_t VME : 1;
				/** Protected-Mode Virtual Interrupts */
				uint64_t PVI : 1;
				/** Time Stamp Disable */
				uint64_t TSD : 1;
				/** Debugging Extensions */
				uint64_t DE : 1;
				/** Page Size Extensions */
				uint64_t PSE : 1;
				/** Physical Address Extension */
				uint64_t PAE : 1;
				/** Machine Check Enable */
				uint64_t MCE : 1;
				/** Page Global Enable */
				uint64_t PGE : 1;
				/** Performance Monitoring Counter */
				uint64_t PCE : 1;
				/** FXSAVE/FXRSTOR Support */
				uint64_t OSFXSR : 1;
				/** Unmasked Exception Support */
				uint64_t OSXMMEXCPT : 1;
				/** User-Mode Instruction Prevention */
				uint64_t UMIP : 1;
				/** Linear Address 57bit */
				uint64_t LA57 : 1;
				/** VMX Enable */
				uint64_t VMXE : 1;
				/** SMX Enable */
				uint64_t SMXE : 1;
				/** Reserved */
				uint64_t Reserved0 : 1;
				/** FSGSBASE Enable */
				uint64_t FSGSBASE : 1;
				/** PCID Enable */
				uint64_t PCIDE : 1;
				/** XSAVE and Processor Extended States Enable */
				uint64_t OSXSAVE : 1;
				/** Reserved */
				uint64_t Reserved1 : 1;
				/** SMEP Enable */
				uint64_t SMEP : 1;
				/** SMAP Enable */
				uint64_t SMAP : 1;
				/** Protection-Key Enable */
				uint64_t PKE : 1;
				/** Control-flow Enforcement Technology*/
				uint32_t CET : 1;
				/* Enable Protection Keys for Supervisor Mode Pages */
				uint32_t PKS : 1;
				/** Reserved */
				uint64_t Reserved2 : 7; // TODO: This could be wrong
			};
			uint64_t raw;
		} CR4;

		typedef union CR8
		{
			struct
			{
				/** Task Priority Register */
				uint64_t TPR : 4;
				/** Reserved */
				uint64_t Reserved : 60;
			};
			uint64_t raw;
		} CR8;

		typedef union XCR0
		{
			/*
			On https://wiki.osdev.org/CPU_Registers_x86#XCR0 says that the PKRU bit is 9?
			*/
			struct
			{
				/** X87 FPU/MMX/SSE Support (must be 1) */
				uint64_t X87 : 1;
				/** XSAVE support for MXCSR and XMM registers */
				uint64_t SSE : 1;
				/** AVX support for YMM registers */
				uint64_t AVX : 1;
				/** MPX support for BND registers */
				uint64_t BNDREG : 1;
				/** MPX support for BNDCFGU and BNDSTATUS registers */
				uint64_t BNDCSR : 1;
				/** AVX-512 support for opmask registers */
				uint64_t OpMask : 1;
				/** AVX-512 enabled and XSAVE support for upper halves of lower ZMM registers */
				uint64_t ZMM_HI256 : 1;
				/** AVX-512 enabled and XSAVE support for upper ZMM registers */
				uint64_t HI16_ZMM : 1;
				/** XSAVE support for PKRU register */
				uint64_t PKRU : 1;
				/** Reserved */
				uint64_t Reserved0 : 53;
				/** AMD lightweight profiling */
				uint64_t LWP : 1;
				/** Reserved */
				uint64_t Reserved1 : 1;
			};
			uint64_t raw;
		} XCR0;

#if defined(__amd64__)
		nsa static inline CR0 readcr0()
		{
			uint64_t Result = 0;
			asmv("mov %%cr0, %[Result]"
				 : [Result] "=q"(Result));
			return (CR0){.raw = Result};
		}

		nsa static inline CR2 readcr2()
		{
			uint64_t Result = 0;
			asmv("mov %%cr2, %[Result]"
				 : [Result] "=q"(Result));
			return (CR2){.raw = Result};
		}

		nsa static inline CR3 readcr3()
		{
			uint64_t Result = 0;
			asmv("mov %%cr3, %[Result]"
				 : [Result] "=q"(Result));
			return (CR3){.raw = Result};
		}

		nsa static inline CR4 readcr4()
		{
			uint64_t Result = 0;
			asmv("mov %%cr4, %[Result]"
				 : [Result] "=q"(Result));
			return (CR4){.raw = Result};
		}

		nsa static inline CR8 readcr8()
		{
			uint64_t Result = 0;
			asmv("mov %%cr8, %[Result]"
				 : [Result] "=q"(Result));
			return (CR8){.raw = Result};
		}

		nsa static inline XCR0 readxcr0()
		{
			uint64_t Result = 0;
			asmv("xgetbv"
				 : "=a"(Result)
				 : "c"(0)
				 : "edx");
			return (XCR0){.raw = Result};
		}

		nsa static inline void writecr0(CR0 ControlRegister)
		{
			asmv("mov %[ControlRegister], %%cr0"
				 :
				 : [ControlRegister] "q"(ControlRegister.raw)
				 : "memory");
		}

		nsa static inline void writecr2(CR2 ControlRegister)
		{
			asmv("mov %[ControlRegister], %%cr2"
				 :
				 : [ControlRegister] "q"(ControlRegister.raw)
				 : "memory");
		}

		nsa static inline void writecr3(CR3 ControlRegister)
		{
			asmv("mov %[ControlRegister], %%cr3"
				 :
				 : [ControlRegister] "q"(ControlRegister.raw)
				 : "memory");
		}

		nsa static inline void writecr4(CR4 ControlRegister)
		{
			asmv("mov %[ControlRegister], %%cr4"
				 :
				 : [ControlRegister] "q"(ControlRegister.raw)
				 : "memory");
		}

		nsa static inline void writecr8(CR8 ControlRegister)
		{
			asmv("mov %[ControlRegister], %%cr8"
				 :
				 : [ControlRegister] "q"(ControlRegister.raw)
				 : "memory");
		}

		nsa static inline void writexcr0(XCR0 ControlRegister)
		{
			asmv("xsetbv"
				 :
				 : "a"(ControlRegister.raw), "c"(0)
				 : "edx");
		}
#endif
	}
}

#endif // !__FENNIX_KERNEL_CPU_x64_CR_H__
