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

#ifndef __FENNIX_KERNEL_CPU_x86_INTERRUPTS_H__
#define __FENNIX_KERNEL_CPU_x86_INTERRUPTS_H__

#include <types.h>

namespace CPU
{
    namespace x86
    {
        enum CPUInterrupts
        {
            /* ISR */

            ISR0 = 0x0,   /* Divide-by-zero Error */
            ISR1 = 0x1,   /* Debug */
            ISR2 = 0x2,   /* Non-maskable Interrupt */
            ISR3 = 0x3,   /* Breakpoint */
            ISR4 = 0x4,   /* Overflow */
            ISR5 = 0x5,   /* Bound Range Exceeded */
            ISR6 = 0x6,   /* Invalid Opcode */
            ISR7 = 0x7,   /* Device Not Available */
            ISR8 = 0x8,   /* Double Fault */
            ISR9 = 0x9,   /* Coprocessor Segment Overrun */
            ISR10 = 0xa,  /* Invalid TSS */
            ISR11 = 0xb,  /* Segment Not P */
            ISR12 = 0xc,  /* Stack-Segment Fault */
            ISR13 = 0xd,  /* General Protection Fault */
            ISR14 = 0xe,  /* Page Fault */
            ISR15 = 0xf,  /* Reserved */
            ISR16 = 0x10, /* x87 Floating-Point Exception */
            ISR17 = 0x11, /* Alignment Check */
            ISR18 = 0x12, /* Machine Check */
            ISR19 = 0x13, /* SIMD Floating-Point Exception */
            ISR20 = 0x14, /* Virtualization Exception */
            ISR21 = 0x15, /* Reserved */
            ISR22 = 0x16, /* Reserved */
            ISR23 = 0x17, /* Reserved */
            ISR24 = 0x18, /* Reserved */
            ISR25 = 0x19, /* Reserved */
            ISR26 = 0x1a, /* Reserved */
            ISR27 = 0x1b, /* Reserved */
            ISR28 = 0x1c, /* Reserved */
            ISR29 = 0x1d, /* Reserved */
            ISR30 = 0x1e, /* Security Exception */
            ISR31 = 0x1f, /* Reserved */

            /* IRQ */

            IRQ0 = 0x20,  /* Programmable Interrupt Timer Interrupt */
            IRQ1 = 0x21,  /* Keyboard Interrupt */
            IRQ2 = 0x22,  /* Cascade (used internally by the two PICs. never raised) */
            IRQ3 = 0x23,  /* COM2/COM4 (if enabled) */
            IRQ4 = 0x24,  /* COM1/COM3 (if enabled) */
            IRQ5 = 0x25,  /* LPT2 (if enabled) */
            IRQ6 = 0x26,  /* Floppy Disk */
            IRQ7 = 0x27,  /* LPT1 / Unreliable "spurious" interrupt (usually) */
            IRQ8 = 0x28,  /* CMOS real-time clock (if enabled) */
            IRQ9 = 0x29,  /* Free for peripherals / legacy SCSI / NIC */
            IRQ10 = 0x2a, /* Free for peripherals / SCSI / NIC */
            IRQ11 = 0x2b, /* Free for peripherals / SCSI / NIC */
            IRQ12 = 0x2c, /* PS2 Mouse */
            IRQ13 = 0x2d, /* FPU / Coprocessor / Inter-processor */
            IRQ14 = 0x2e, /* Primary ATA Hard Disk */
            IRQ15 = 0x2f, /* Secondary ATA Hard Disk */

            /* Reserved by OS */

            IRQ16 = 0x30, /* Reserved for multitasking */
            IRQ17 = 0x31,
            IRQ18 = 0x32,
            IRQ19 = 0x33,
            IRQ20 = 0x34,
            IRQ21 = 0x35,
            IRQ22 = 0x36,
            IRQ23 = 0x37,
            IRQ24 = 0x38,
            IRQ25 = 0x39,
            IRQ26 = 0x3a,
            IRQ27 = 0x3b,
            IRQ28 = 0x3c,
            IRQ29 = 0x3d,
            IRQ30 = 0x3e,
            IRQ31 = 0x3f, /* Halt core interrupt */

            /* Free */

            IRQ32 = 0x40,
            IRQ33 = 0x41,
            IRQ34 = 0x42,
            IRQ35 = 0x43,
            IRQ36 = 0x44,
            IRQ37 = 0x45,
            IRQ38 = 0x46,
            IRQ39 = 0x47,
            IRQ40 = 0x48,
            IRQ41 = 0x49,
            IRQ42 = 0x4a,
            IRQ43 = 0x4b,
            IRQ44 = 0x4c,
            IRQ45 = 0x4d,
            IRQ46 = 0x4e,
            IRQ47 = 0x4f,
            IRQ48 = 0x50,
            IRQ49 = 0x51,
            IRQ50 = 0x52,
            IRQ51 = 0x53,
            IRQ52 = 0x54,
            IRQ53 = 0x55,
            IRQ54 = 0x56,
            IRQ55 = 0x57,
            IRQ56 = 0x58,
            IRQ57 = 0x59,
            IRQ58 = 0x5a,
            IRQ59 = 0x5b,
            IRQ60 = 0x5c,
            IRQ61 = 0x5d,
            IRQ62 = 0x5e,
            IRQ63 = 0x5f,
            IRQ64 = 0x60,
            IRQ65 = 0x61,
            IRQ66 = 0x62,
            IRQ67 = 0x63,
            IRQ68 = 0x64,
            IRQ69 = 0x65,
            IRQ70 = 0x66,
            IRQ71 = 0x67,
            IRQ72 = 0x68,
            IRQ73 = 0x69,
            IRQ74 = 0x6a,
            IRQ75 = 0x6b,
            IRQ76 = 0x6c,
            IRQ77 = 0x6d,
            IRQ78 = 0x6e,
            IRQ79 = 0x6f,
            IRQ80 = 0x70,
            IRQ81 = 0x71,
            IRQ82 = 0x72,
            IRQ83 = 0x73,
            IRQ84 = 0x74,
            IRQ85 = 0x75,
            IRQ86 = 0x76,
            IRQ87 = 0x77,
            IRQ88 = 0x78,
            IRQ89 = 0x79,
            IRQ90 = 0x7a,
            IRQ91 = 0x7b,
            IRQ92 = 0x7c,
            IRQ93 = 0x7d,
            IRQ94 = 0x7e,
            IRQ95 = 0x7f,
            IRQ96 = 0x80,
            IRQ97 = 0x81,
            IRQ98 = 0x82,
            IRQ99 = 0x83,
            IRQ100 = 0x84,
            IRQ101 = 0x85,
            IRQ102 = 0x86,
            IRQ103 = 0x87,
            IRQ104 = 0x88,
            IRQ105 = 0x89,
            IRQ106 = 0x8a,
            IRQ107 = 0x8b,
            IRQ108 = 0x8c,
            IRQ109 = 0x8d,
            IRQ110 = 0x8e,
            IRQ111 = 0x8f,
            IRQ112 = 0x90,
            IRQ113 = 0x91,
            IRQ114 = 0x92,
            IRQ115 = 0x93,
            IRQ116 = 0x94,
            IRQ117 = 0x95,
            IRQ118 = 0x96,
            IRQ119 = 0x97,
            IRQ120 = 0x98,
            IRQ121 = 0x99,
            IRQ122 = 0x9a,
            IRQ123 = 0x9b,
            IRQ124 = 0x9c,
            IRQ125 = 0x9d,
            IRQ126 = 0x9e,
            IRQ127 = 0x9f,
            IRQ128 = 0xa0,
            IRQ129 = 0xa1,
            IRQ130 = 0xa2,
            IRQ131 = 0xa3,
            IRQ132 = 0xa4,
            IRQ133 = 0xa5,
            IRQ134 = 0xa6,
            IRQ135 = 0xa7,
            IRQ136 = 0xa8,
            IRQ137 = 0xa9,
            IRQ138 = 0xaa,
            IRQ139 = 0xab,
            IRQ140 = 0xac,
            IRQ141 = 0xad,
            IRQ142 = 0xae,
            IRQ143 = 0xaf,
            IRQ144 = 0xb0,
            IRQ145 = 0xb1,
            IRQ146 = 0xb2,
            IRQ147 = 0xb3,
            IRQ148 = 0xb4,
            IRQ149 = 0xb5,
            IRQ150 = 0xb6,
            IRQ151 = 0xb7,
            IRQ152 = 0xb8,
            IRQ153 = 0xb9,
            IRQ154 = 0xba,
            IRQ155 = 0xbb,
            IRQ156 = 0xbc,
            IRQ157 = 0xbd,
            IRQ158 = 0xbe,
            IRQ159 = 0xbf,
            IRQ160 = 0xc0,
            IRQ161 = 0xc1,
            IRQ162 = 0xc2,
            IRQ163 = 0xc3,
            IRQ164 = 0xc4,
            IRQ165 = 0xc5,
            IRQ166 = 0xc6,
            IRQ167 = 0xc7,
            IRQ168 = 0xc8,
            IRQ169 = 0xc9,
            IRQ170 = 0xca,
            IRQ171 = 0xcb,
            IRQ172 = 0xcc,
            IRQ173 = 0xcd,
            IRQ174 = 0xce,
            IRQ175 = 0xcf,
            IRQ176 = 0xd0,
            IRQ177 = 0xd1,
            IRQ178 = 0xd2,
            IRQ179 = 0xd3,
            IRQ180 = 0xd4,
            IRQ181 = 0xd5,
            IRQ182 = 0xd6,
            IRQ183 = 0xd7,
            IRQ184 = 0xd8,
            IRQ185 = 0xd9,
            IRQ186 = 0xda,
            IRQ187 = 0xdb,
            IRQ188 = 0xdc,
            IRQ189 = 0xdd,
            IRQ190 = 0xde,
            IRQ191 = 0xdf,
            IRQ192 = 0xe0,
            IRQ193 = 0xe1,
            IRQ194 = 0xe2,
            IRQ195 = 0xe3,
            IRQ196 = 0xe4,
            IRQ197 = 0xe5,
            IRQ198 = 0xe6,
            IRQ199 = 0xe7,
            IRQ200 = 0xe8,
            IRQ201 = 0xe9,
            IRQ202 = 0xea,
            IRQ203 = 0xeb,
            IRQ204 = 0xec,
            IRQ205 = 0xed,
            IRQ206 = 0xee,
            IRQ207 = 0xef,
            IRQ208 = 0xf0,
            IRQ209 = 0xf1,
            IRQ210 = 0xf2,
            IRQ211 = 0xf3,
            IRQ212 = 0xf4,
            IRQ213 = 0xf5,
            IRQ214 = 0xf6,
            IRQ215 = 0xf7,
            IRQ216 = 0xf8,
            IRQ217 = 0xf9,
            IRQ218 = 0xfa,
            IRQ219 = 0xfb,
            IRQ220 = 0xfc,
            IRQ221 = 0xfd,
            IRQ222 = 0xfe,
            IRQ223 = 0xff,
        };
    }
}

#endif // !__FENNIX_KERNEL_CPU_x86_INTERRUPTS_H__
