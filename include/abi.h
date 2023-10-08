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

#ifndef __FENNIX_KERNEL_ABI_H__
#define __FENNIX_KERNEL_ABI_H__

#include <types.h>

#define AT_NULL 0
#define AT_IGNORE 1
#define AT_EXECFD 2
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_NOTELF 10
#define AT_UID 11
#define AT_EUID 12
#define AT_GID 13
#define AT_EGID 14
#define AT_PLATFORM 15
#define AT_HWCAP 16
#define AT_CLKTCK 17
#define AT_SECURE 23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM 25
#define AT_HWCAP2 26
#define AT_EXECFN 31
#define AT_SYSINFO 32
#define AT_SYSINFO_EHDR 33
#define AT_L1I_CACHESHAPE 34
#define AT_L1D_CACHESHAPE 35
#define AT_L2_CACHESHAPE 36
#define AT_L3_CACHESHAPE 37
#define AT_L1I_CACHESIZE 40
#define AT_L1I_CACHEGEOMETRY 41
#define AT_L1D_CACHESIZE 42
#define AT_L1D_CACHEGEOMETRY 43
#define AT_L2_CACHESIZE 44
#define AT_L2_CACHEGEOMETRY 45
#define AT_L3_CACHESIZE 46
#define AT_L3_CACHEGEOMETRY 47
#define AT_MINSIGSTKSZ 51

typedef struct
{
    uint32_t a_type;
    union
    {
        uint32_t a_val;
    } a_un;
} Elf32_auxv_t;

typedef struct
{
    uint64_t a_type;
    union
    {
        uint64_t a_val;
    } a_un;
} Elf64_auxv_t;

#if defined(a64)
typedef Elf64_auxv_t Elf_auxv_t;
#elif defined(a32)
typedef Elf64_auxv_t Elf_auxv_t;
#elif defined(aa64)
typedef Elf64_auxv_t Elf_auxv_t;
#endif

typedef struct
{
    Elf_auxv_t archaux;
} AuxiliaryVector;

#endif // !__FENNIX_KERNEL_ABI_H__
