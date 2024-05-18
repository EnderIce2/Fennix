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

#ifndef __FENNIX_KERNEL_MACH_O_H__
#define __FENNIX_KERNEL_MACH_O_H__

#include <types.h>

#define MH_MAGIC 0xfeedface
#define MH_CIGAM 0xcefaedfe

#define CPU_TYPE_ANY ((cpu_type_t) - 1)

#define CPU_TYPE_VAX ((cpu_type_t)1)
#define CPU_TYPE_ROMP ((cpu_type_t)2)
#define CPU_TYPE_NS32032 ((cpu_type_t)4)
#define CPU_TYPE_NS32332 ((cpu_type_t)5)
#define CPU_TYPE_MC680x0 ((cpu_type_t)6)
#define CPU_TYPE_I386 ((cpu_type_t)7)
#define CPU_TYPE_X86_64 ((cpu_type_t)(CPU_TYPE_I386 | CPU_ARCH_ABI64))
#define CPU_TYPE_MIPS ((cpu_type_t)8)
#define CPU_TYPE_NS32532 ((cpu_type_t)9)
#define CPU_TYPE_HPPA ((cpu_type_t)11)
#define CPU_TYPE_ARM ((cpu_type_t)12)
#define CPU_TYPE_MC88000 ((cpu_type_t)13)
#define CPU_TYPE_SPARC ((cpu_type_t)14)
#define CPU_TYPE_I860 ((cpu_type_t)15)
#define CPU_TYPE_I860_LITTLE ((cpu_type_t)16)
#define CPU_TYPE_RS6000 ((cpu_type_t)17)
#define CPU_TYPE_MC98000 ((cpu_type_t)18)
#define CPU_TYPE_POWERPC ((cpu_type_t)18)
#define CPU_ARCH_ABI64 0x1000000
#define CPU_TYPE_POWERPC64 ((cpu_type_t)(CPU_TYPE_POWERPC | CPU_ARCH_ABI64))
#define CPU_TYPE_VEO ((cpu_type_t)255)

typedef int cpu_type_t;
typedef int cpu_subtype_t;

struct mach_header
{
	uint32_t magic;
	cpu_type_t cputype;
	cpu_subtype_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
};

struct mach_header_64
{
	uint32_t magic;
	cpu_type_t cputype;
	cpu_subtype_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
	uint32_t reserved;
};

#endif // !__FENNIX_KERNEL_MACH_O_H__
