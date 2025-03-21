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

#ifndef __FENNIX_KERNEL_INTRIN_H__
#define __FENNIX_KERNEL_INTRIN_H__

#include <types.h>
#include <debug.h>

namespace FXSR
{
    void _fxsave(void *mem_addr)
    {
#ifdef __amd64__
        __builtin_ia32_fxsave(mem_addr);
#endif
    }

    void _fxrstor(void *mem_addr)
    {
#ifdef __amd64__
        __builtin_ia32_fxrstor(mem_addr);
#endif
    }

    void _fxsave64(void *mem_addr)
    {
#ifdef __amd64__
        asmv("fxsaveq (%0)"
             :
             : "r"(mem_addr)
             : "memory");
#endif
    }

    void _fxrstor64(void *mem_addr)
    {
#ifdef __amd64__
        asmv("fxrstorq (%0)"
             :
             : "r"(mem_addr)
             : "memory");
#endif
    }
}

namespace SMAP
{
    void _clac(void)
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("clac" ::
                 : "cc");
#endif // __amd64__ || __i386__
    }

    void _stac(void)
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("stac" ::
                 : "cc");
#endif // __amd64__ || __i386__
    }
}

#endif // !__FENNIX_KERNEL_INTRIN_H__
