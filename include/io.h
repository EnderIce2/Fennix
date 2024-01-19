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

#ifndef __FENNIX_KERNEL_IO_H__
#define __FENNIX_KERNEL_IO_H__

#include <types.h>

#if defined(a86)

#ifdef __cplusplus
extern "C"
{
#endif
	static inline uint8_t inb(uint16_t Port)
	{
		uint8_t Result;
		asm("in %%dx, %%al"
			: "=a"(Result)
			: "d"(Port));
		return Result;
	}

	static inline uint16_t inw(uint16_t Port)
	{
		uint16_t Result;
		asm("in %%dx, %%ax"
			: "=a"(Result)
			: "d"(Port));
		return Result;
	}

	static inline uint32_t inl(uint16_t Port)
	{
		uint32_t Result;
		asmv("inl %1, %0"
			 : "=a"(Result)
			 : "dN"(Port));
		return Result;
	}

	static inline void outb(uint16_t Port, uint8_t Data)
	{
		asmv("out %%al, %%dx"
			 :
			 : "a"(Data), "d"(Port));
	}

	static inline void outw(uint16_t Port, uint16_t Data)
	{
		asmv("out %%ax, %%dx"
			 :
			 : "a"(Data), "d"(Port));
	}

	static inline void outl(uint16_t Port, uint32_t Data)
	{
		asmv("outl %1, %0"
			 :
			 : "dN"(Port), "a"(Data));
	}

	static inline void mmoutb(void *Address, uint8_t Value)
	{
		asmv("mov %1, %0"
			 : "=m"((*(uint8_t *)(Address)))
			 : "r"(Value)
			 : "memory");
	}

	static inline void mmoutw(void *Address, uint16_t Value)
	{
		asmv("mov %1, %0"
			 : "=m"((*(uint16_t *)(Address)))
			 : "r"(Value)
			 : "memory");
	}

	static inline void mmoutl(void *Address, uint32_t Value)
	{
		asmv("mov %1, %0"
			 : "=m"((*(uint32_t *)(Address)))
			 : "r"(Value)
			 : "memory");
	}

#if defined(a64)
	static inline void mmoutq(void *Address, uint64_t Value)
	{
		asmv("mov %1, %0"
			 : "=m"((*(uint64_t *)(Address)))
			 : "r"(Value)
			 : "memory");
	}
#endif

	static inline uint8_t mminb(void *Address)
	{
		uint8_t Result;
		asmv("mov %1, %0"
			 : "=r"(Result)
			 : "m"((*(uint8_t *)(Address)))
			 : "memory");
		return Result;
	}

	static inline uint16_t mminw(void *Address)
	{
		uint16_t Result;
		asmv("mov %1, %0"
			 : "=r"(Result)
			 : "m"((*(uint16_t *)(Address)))
			 : "memory");
		return Result;
	}

	static inline uint32_t mminl(void *Address)
	{
		uint32_t Result;
		asmv("mov %1, %0"
			 : "=r"(Result)
			 : "m"((*(uint32_t *)(Address)))
			 : "memory");
		return Result;
	}

#if defined(a64)
	static inline uint64_t mminq(void *Address)
	{
		uint64_t Result;
		asmv("mov %1, %0"
			 : "=r"(Result)
			 : "m"((*(uint64_t *)(Address)))
			 : "memory");
		return Result;
	}
#endif

#ifdef __cplusplus
}
#endif

#endif // defined(a86)
#endif // !__FENNIX_KERNEL_IO_H__
