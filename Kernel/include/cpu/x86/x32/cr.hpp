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

#ifndef __FENNIX_KERNEL_CPU_x32_CR_H__
#define __FENNIX_KERNEL_CPU_x32_CR_H__

#include <types.h>

namespace CPU
{
	namespace x32
	{
		typedef union CR0
		{
			struct
			{
				/** Protection Enable */
				uint32_t PE : 1;
				/** Monitor Coprocessor */
				uint32_t MP : 1;
				/** Emulation */
				uint32_t EM : 1;
				/** Task Switched */
				uint32_t TS : 1;
				/** Extension Type */
				uint32_t ET : 1;
				/** Numeric Error */
				uint32_t NE : 1;
				/** Reserved */
				uint32_t Reserved0 : 10;
				/** Write Protect */
				uint32_t WP : 1;
				/** Reserved */
				uint32_t Reserved1 : 1;
				/** Alignment Mask */
				uint32_t AM : 1;
				/** Reserved */
				uint32_t Reserved2 : 10;
				/** Not Write-through */
				uint32_t NW : 1;
				/** Cache Disable */
				uint32_t CD : 1;
				/** Paging */
				uint32_t PG : 1;
			};
			uint32_t raw;
		} CR0;

		typedef union CR2
		{
			struct
			{
				/** Page Fault Linear Address */
				uint32_t PFLA;
			};
			uint32_t raw;
		} CR2;

		typedef union CR3
		{
			struct
			{
				/** Not used if bit 17 of CR4 is 1 */
				uint32_t PWT : 1;
				/** Not used if bit 17 of CR4 is 1 */
				uint32_t PCD : 1;
				/** Base of PML4T/PML5T */
				uint32_t PDBR;
			};
			uint32_t raw;
		} CR3;

		typedef union CR4
		{
			struct
			{
				/** Virtual-8086 Mode Extensions */
				uint32_t VME : 1;
				/** Protected-Mode Virtual Interrupts */
				uint32_t PVI : 1;
				/** Time Stamp Disable */
				uint32_t TSD : 1;
				/** Debugging Extensions */
				uint32_t DE : 1;
				/** Page Size Extensions */
				uint32_t PSE : 1;
				/** Physical Address Extension */
				uint32_t PAE : 1;
				/** Machine Check Enable */
				uint32_t MCE : 1;
				/** Page Global Enable */
				uint32_t PGE : 1;
				/** Performance Monitoring Counter */
				uint32_t PCE : 1;
				/** Operating System Support */
				uint32_t OSFXSR : 1;
				/** Operating System Support */
				uint32_t OSXMMEXCPT : 1;
				/** User-Mode Instruction Prevention */
				uint32_t UMIP : 1;
				/** Linear Address 57bit */
				uint32_t LA57 : 1;
				/** VMX Enable */
				uint32_t VMXE : 1;
				/** SMX Enable */
				uint32_t SMXE : 1;
				/** Reserved */
				uint32_t Reserved0 : 1;
				/** FSGSBASE Enable */
				uint32_t FSGSBASE : 1;
				/** PCID Enable */
				uint32_t PCIDE : 1;
				/** XSAVE and Processor Extended States Enable */
				uint32_t OSXSAVE : 1;
				/** Reserved */
				uint32_t Reserved1 : 1;
				/** SMEP Enable */
				uint32_t SMEP : 1;
				/** SMAP Enable */
				uint32_t SMAP : 1;
				/** Protection-Key Enable */
				uint32_t PKE : 1;
				/** Control-flow Enforcement Technology*/
				uint32_t CET : 1;
				/* Enable Protection Keys for Supervisor Mode Pages */
				uint32_t PKS : 1;
			};
			uint32_t raw;
		} CR4;
#if defined(__i386__)
		nsa static inline CR0 readcr0()
		{
			uint32_t Result = 0;
			asmv("mov %%cr0, %[Result]"
				 : [Result] "=q"(Result));
			return (CR0){.raw = Result};
		}

		nsa static inline CR2 readcr2()
		{
			uint32_t Result = 0;
			asmv("mov %%cr2, %[Result]"
				 : [Result] "=q"(Result));
			return (CR2){.raw = Result};
		}

		nsa static inline CR3 readcr3()
		{
			uint32_t Result = 0;
			asmv("mov %%cr3, %[Result]"
				 : [Result] "=q"(Result));
			return (CR3){.raw = Result};
		}

		nsa static inline CR4 readcr4()
		{
			uint32_t Result = 0;
			asmv("mov %%cr4, %[Result]"
				 : [Result] "=q"(Result));
			return (CR4){.raw = Result};
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
#endif
	}
}

#endif // !__FENNIX_KERNEL_CPU_x32_CR_H__
