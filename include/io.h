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
    static inline uint8_t inportb(uint16_t Port)
    {
        uint8_t Result;
        asm("in %%dx, %%al"
            : "=a"(Result)
            : "d"(Port));
        return Result;
    }

    static inline uint16_t inportw(uint16_t Port)
    {
        uint16_t Result;
        asm("in %%dx, %%ax"
            : "=a"(Result)
            : "d"(Port));
        return Result;
    }

    static inline uint32_t inportl(uint16_t Port)
    {
        uint32_t Result;
        asmv("inl %1, %0"
             : "=a"(Result)
             : "dN"(Port));
        return Result;
    }

    static inline void outportb(uint16_t Port, uint8_t Data)
    {
        asmv("out %%al, %%dx"
             :
             : "a"(Data), "d"(Port));
    }

    static inline void outportw(uint16_t Port, uint16_t Data)
    {
        asmv("out %%ax, %%dx"
             :
             : "a"(Data), "d"(Port));
    }

    static inline void outportl(uint16_t Port, uint32_t Data)
    {
        asmv("outl %1, %0"
             :
             : "dN"(Port), "a"(Data));
    }

    static inline uint8_t mmioin8(uint64_t Address)
    {
        asmv("" ::
                 : "memory");
        uint8_t Result = *(volatile uint8_t *)(uintptr_t)Address;
        asmv("" ::
                 : "memory");
        return Result;
    }

    static inline uint16_t mmioin16(uint64_t Address)
    {
        asmv("" ::
                 : "memory");
        uint16_t Result = *(volatile uint16_t *)(uintptr_t)Address;
        asmv("" ::
                 : "memory");
        return Result;
    }

    static inline uint32_t mmioin32(uint64_t Address)
    {
        asmv("" ::
                 : "memory");
        uint32_t Result = *(volatile uint32_t *)(uintptr_t)Address;
        asmv("" ::
                 : "memory");
        return Result;
    }

    static inline uint64_t mmioin64(uint64_t Address)
    {
        asmv("" ::
                 : "memory");
        uint64_t Result = *(volatile uint64_t *)(uintptr_t)Address;
        asmv("" ::
                 : "memory");
        return Result;
    }

    static inline void mmioout8(uint64_t Address, uint8_t Data)
    {
        asmv("" ::
                 : "memory");
        *(volatile uint8_t *)Address = Data;
        asmv("" ::
                 : "memory");
    }

    static inline void mmioout16(uint64_t Address, uint16_t Data)
    {
        asmv("" ::
                 : "memory");
        *(volatile uint16_t *)Address = Data;
        asmv("" ::
                 : "memory");
    }

    static inline void mmioout32(uint64_t Address, uint32_t Data)
    {
        asmv("" ::
                 : "memory");
        *(volatile uint32_t *)Address = Data;
        asmv("" ::
                 : "memory");
    }

    static inline void mmioout64(uint64_t Address, uint64_t Data)
    {
        asmv("" ::
                 : "memory");
        *(volatile uint64_t *)Address = Data;
        asmv("" ::
                 : "memory");
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

#define inb(Port) inportb(Port)
#define inw(Port) inportw(Port)
#define inl(Port) inportl(Port)
#define outb(Port, Data) outportb(Port, Data)
#define outw(Port, Data) outportw(Port, Data)
#define outl(Port, Data) outportl(Port, Data)

#endif // defined(a86)

#endif // !__FENNIX_KERNEL_IO_H__
