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
                /** @brief Protection Enable */
                uint64_t PE : 1;
                /** @brief Monitor Coprocessor */
                uint64_t MP : 1;
                /** @brief Emulation */
                uint64_t EM : 1;
                /** @brief Task Switched */
                uint64_t TS : 1;
                /** @brief Extension Type */
                uint64_t ET : 1;
                /** @brief Numeric Error */
                uint64_t NE : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 10;
                /** @brief Write Protect */
                uint64_t WP : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief Alignment Mask */
                uint64_t AM : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 10;
                /** @brief Not Write-through */
                uint64_t NW : 1;
                /** @brief Cache Disable */
                uint64_t CD : 1;
                /** @brief Paging */
                uint64_t PG : 1;
            };
            uint64_t raw;
        } CR0;

        typedef union CR2
        {
            struct
            {
                /** @brief Page Fault Linear Address */
                uint64_t PFLA;
            };
            uint64_t raw;
        } CR2;

        typedef union CR3
        {
            struct
            {
                /** @brief Not used if bit 17 of CR4 is 1 */
                uint64_t PWT : 1;
                /** @brief Not used if bit 17 of CR4 is 1 */
                uint64_t PCD : 1;
                /** @brief Base of PML4T/PML5T */
                uint64_t PDBR;
            };
            uint64_t raw;
        } CR3;

        typedef union CR4
        {
            struct
            {
                /** @brief Virtual-8086 Mode Extensions */
                uint64_t VME : 1;
                /** @brief Protected-Mode Virtual Interrupts */
                uint64_t PVI : 1;
                /** @brief Time Stamp Disable */
                uint64_t TSD : 1;
                /** @brief Debugging Extensions */
                uint64_t DE : 1;
                /** @brief Page Size Extensions */
                uint64_t PSE : 1;
                /** @brief Physical Address Extension */
                uint64_t PAE : 1;
                /** @brief Machine Check Enable */
                uint64_t MCE : 1;
                /** @brief Page Global Enable */
                uint64_t PGE : 1;
                /** @brief Performance Monitoring Counter */
                uint64_t PCE : 1;
                /** @brief Operating System Support */
                uint64_t OSFXSR : 1;
                /** @brief Operating System Support */
                uint64_t OSXMMEXCPT : 1;
                /** @brief User-Mode Instruction Prevention */
                uint64_t UMIP : 1;
                /** @brief Linear Address 57bit */
                uint64_t LA57 : 1;
                /** @brief VMX Enable */
                uint64_t VMXE : 1;
                /** @brief SMX Enable */
                uint64_t SMXE : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 1;
                /** @brief FSGSBASE Enable */
                uint64_t FSGSBASE : 1;
                /** @brief PCID Enable */
                uint64_t PCIDE : 1;
                /** @brief XSAVE and Processor Extended States Enable */
                uint64_t OSXSAVE : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
                /** @brief SMEP Enable */
                uint64_t SMEP : 1;
                /** @brief SMAP Enable */
                uint64_t SMAP : 1;
                /** @brief Protection-Key Enable */
                uint64_t PKE : 1;
                /** @brief Control-flow Enforcement Technology*/
                uint32_t CET : 1;
                /* @brief Enable Protection Keys for Supervisor Mode Pages */
                uint32_t PKS : 1;
                /** @brief Reserved */
                uint64_t Reserved2 : 7; // TODO: This could be wrong
            };
            uint64_t raw;
        } CR4;

        typedef union CR8
        {
            struct
            {
                /** @brief Task Priority Level */
                uint64_t TPL : 1;
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
                /** @brief X87 FPU/MMX/SSE Support (must be 1) */
                uint64_t X87 : 1;
                /** @brief XSAVE support for MXCSR and XMM registers */
                uint64_t SSE : 1;
                /** @brief AVX support for YMM registers */
                uint64_t AVX : 1;
                /** @brief MPX support for BND registers */
                uint64_t BNDREG : 1;
                /** @brief MPX support for BNDCFGU and BNDSTATUS registers */
                uint64_t BNDCSR : 1;
                /** @brief AVX-512 support for opmask registers */
                uint64_t OpMask : 1;
                /** @brief AVX-512 enabled and XSAVE support for upper halves of lower ZMM registers */
                uint64_t ZMM_HI256 : 1;
                /** @brief AVX-512 enabled and XSAVE support for upper ZMM registers */
                uint64_t HI16_ZMM : 1;
                /** @brief XSAVE support for PKRU register */
                uint64_t PKRU : 1;
                /** @brief Reserved */
                uint64_t Reserved0 : 53;
                /** @brief AMD lightweight profiling */
                uint64_t LWP : 1;
                /** @brief Reserved */
                uint64_t Reserved1 : 1;
            };
            uint64_t raw;
        } XCR0;

#if defined(a64)
        SafeFunction static inline CR0 readcr0()
        {
            uint64_t Result = 0;
            asmv("mov %%cr0, %[Result]"
                 : [Result] "=q"(Result));
            return (CR0){.raw = Result};
        }

        SafeFunction static inline CR2 readcr2()
        {
            uint64_t Result = 0;
            asmv("mov %%cr2, %[Result]"
                 : [Result] "=q"(Result));
            return (CR2){.raw = Result};
        }

        SafeFunction static inline CR3 readcr3()
        {
            uint64_t Result = 0;
            asmv("mov %%cr3, %[Result]"
                 : [Result] "=q"(Result));
            return (CR3){.raw = Result};
        }

        SafeFunction static inline CR4 readcr4()
        {
            uint64_t Result = 0;
            asmv("mov %%cr4, %[Result]"
                 : [Result] "=q"(Result));
            return (CR4){.raw = Result};
        }

        SafeFunction static inline CR8 readcr8()
        {
            uint64_t Result = 0;
            asmv("mov %%cr8, %[Result]"
                 : [Result] "=q"(Result));
            return (CR8){.raw = Result};
        }

        SafeFunction static inline XCR0 readxcr0()
        {
            uint64_t Result = 0;
            asmv("xgetbv"
                 : "=a"(Result)
                 : "c"(0)
                 : "edx");
            return (XCR0){.raw = Result};
        }

        SafeFunction static inline void writecr0(CR0 ControlRegister)
        {
            asmv("mov %[ControlRegister], %%cr0"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
        }

        SafeFunction static inline void writecr2(CR2 ControlRegister)
        {
            asmv("mov %[ControlRegister], %%cr2"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
        }

        SafeFunction static inline void writecr3(CR3 ControlRegister)
        {
            asmv("mov %[ControlRegister], %%cr3"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
        }

        SafeFunction static inline void writecr4(CR4 ControlRegister)
        {
            asmv("mov %[ControlRegister], %%cr4"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
        }

        SafeFunction static inline void writecr8(CR8 ControlRegister)
        {
            asmv("mov %[ControlRegister], %%cr8"
                 :
                 : [ControlRegister] "q"(ControlRegister.raw)
                 : "memory");
        }

        SafeFunction static inline void writexcr0(XCR0 ControlRegister)
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
