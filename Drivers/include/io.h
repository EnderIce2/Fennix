/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_IO_H__
#define __FENNIX_API_IO_H__

#include <types.h>

#if defined(__amd64__) || defined(__i386__)

#ifdef __cplusplus
extern "C"
{
#endif

	static inline uint8_t inportb(uint16_t Port)
	{
		uint8_t Result;
		__asm__("in %%dx, %%al"
				: "=a"(Result)
				: "d"(Port));
		return Result;
	}

	static inline uint16_t inportw(uint16_t Port)
	{
		uint16_t Result;
		__asm__("in %%dx, %%ax"
				: "=a"(Result)
				: "d"(Port));
		return Result;
	}

	static inline uint32_t inportl(uint16_t Port)
	{
		uint32_t Result;
		__asm__ volatile("inl %1, %0"
						 : "=a"(Result)
						 : "dN"(Port));
		return Result;
	}

	static inline void outportb(uint16_t Port, uint8_t Data)
	{
		__asm__ volatile("out %%al, %%dx"
						 :
						 : "a"(Data), "d"(Port));
	}

	static inline void outportw(uint16_t Port, uint16_t Data)
	{
		__asm__ volatile("out %%ax, %%dx"
						 :
						 : "a"(Data), "d"(Port));
	}

	static inline void outportl(uint16_t Port, uint32_t Data)
	{
		__asm__ volatile("outl %1, %0"
						 :
						 : "dN"(Port), "a"(Data));
	}

	static inline uint8_t mmioin8(uintptr_t Address)
	{
		__asm__ volatile("" ::
							 : "memory");
		uint8_t Result = *(volatile uint8_t *)Address;
		__asm__ volatile("" ::
							 : "memory");
		return Result;
	}

	static inline uint16_t mmioin16(uintptr_t Address)
	{
		__asm__ volatile("" ::
							 : "memory");
		uint16_t Result = *(volatile uint16_t *)Address;
		__asm__ volatile("" ::
							 : "memory");
		return Result;
	}

	static inline uint32_t mmioin32(uintptr_t Address)
	{
		__asm__ volatile("" ::
							 : "memory");
		uint32_t Result = *(volatile uint32_t *)Address;
		__asm__ volatile("" ::
							 : "memory");
		return Result;
	}

	static inline uintptr_t mmioin64(uintptr_t Address)
	{
		__asm__ volatile("" ::
							 : "memory");
		uintptr_t Result = *(volatile uintptr_t *)Address;
		__asm__ volatile("" ::
							 : "memory");
		return Result;
	}

	static inline void mmioout8(uintptr_t Address, uint8_t Data)
	{
		__asm__ volatile("" ::
							 : "memory");
		*(volatile uint8_t *)Address = Data;
		__asm__ volatile("" ::
							 : "memory");
	}

	static inline void mmioout16(uintptr_t Address, uint16_t Data)
	{
		__asm__ volatile("" ::
							 : "memory");
		*(volatile uint16_t *)Address = Data;
		__asm__ volatile("" ::
							 : "memory");
	}

	static inline void mmioout32(uintptr_t Address, uint32_t Data)
	{
		__asm__ volatile("" ::
							 : "memory");
		*(volatile uint32_t *)Address = Data;
		__asm__ volatile("" ::
							 : "memory");
	}

	static inline void mmioout64(uintptr_t Address, uintptr_t Data)
	{
		__asm__ volatile("" ::
							 : "memory");
		*(volatile uintptr_t *)Address = Data;
		__asm__ volatile("" ::
							 : "memory");
	}

	static inline void mmoutb(void *Address, uint8_t Value)
	{
		__asm__ volatile("mov %1, %0"
						 : "=m"((*(uint8_t *)(Address)))
						 : "r"(Value)
						 : "memory");
	}

	static inline void mmoutw(void *Address, uint16_t Value)
	{
		__asm__ volatile("mov %1, %0"
						 : "=m"((*(uint16_t *)(Address)))
						 : "r"(Value)
						 : "memory");
	}

	static inline void mmoutl(void *Address, uint32_t Value)
	{
		__asm__ volatile("mov %1, %0"
						 : "=m"((*(uint32_t *)(Address)))
						 : "r"(Value)
						 : "memory");
	}

	static inline void mmoutq(void *Address, uintptr_t Value)
	{
		__asm__ volatile("mov %1, %0"
						 : "=m"((*(uintptr_t *)(Address)))
						 : "r"(Value)
						 : "memory");
	}

	static inline uint8_t mminb(void *Address)
	{
		uint8_t Result;
		__asm__ volatile("mov %1, %0"
						 : "=r"(Result)
						 : "m"((*(uint8_t *)(Address)))
						 : "memory");
		return Result;
	}

	static inline uint16_t mminw(void *Address)
	{
		uint16_t Result;
		__asm__ volatile("mov %1, %0"
						 : "=r"(Result)
						 : "m"((*(uint16_t *)(Address)))
						 : "memory");
		return Result;
	}

	static inline uint32_t mminl(void *Address)
	{
		uint32_t Result;
		__asm__ volatile("mov %1, %0"
						 : "=r"(Result)
						 : "m"((*(uint32_t *)(Address)))
						 : "memory");
		return Result;
	}

	static inline uintptr_t mminq(void *Address)
	{
		uintptr_t Result;
		__asm__ volatile("mov %1, %0"
						 : "=r"(Result)
						 : "m"((*(uintptr_t *)(Address)))
						 : "memory");
		return Result;
	}

#ifdef __cplusplus
}
#endif

#define inb(Port) inportb(Port)
#define inw(Port) inportw(Port)
#define inl(Port) inportl(Port)
#define outb(Port, Data) outportb(Port, Data)
#define outw(Port, Data) outportw(Port, Data)
#define outl(Port, Data) outportl(Port, Data)

#endif // defined(__amd64__) || defined(__i386__)

#endif // !__FENNIX_API_IO_H__
