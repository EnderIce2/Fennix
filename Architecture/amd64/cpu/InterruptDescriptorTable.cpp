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

#include "idt.hpp"

#include <memory.hpp>
#include <cpu.hpp>
#include <debug.h>
#include <io.h>

#include "gdt.hpp"
#include "../../../kernel.h"

/* conversion from ‘uint64_t’ {aka ‘long unsigned int’} to ‘unsigned char:2’ may change value */
#pragma GCC diagnostic ignored "-Wconversion"

extern "C" void MainInterruptHandler(void *Data);
extern "C" void ExceptionHandler(void *Data);

namespace InterruptDescriptorTable
{
    static InterruptDescriptorTableEntry Entries[0x100];

    InterruptDescriptorTableDescriptor idtd = {
        .Length = sizeof(Entries) - 1,
        .Entries = Entries,
    };

    void SetEntry(uint8_t Index,
                  void (*Base)(),
                  uint8_t InterruptStackTable,
                  InterruptGateType Gate,
                  InterruptRingType Ring,
                  bool Present,
                  uint16_t SegmentSelector)
    {
        Entries[Index].BaseLow = s_cst(uint16_t, ((uint64_t)Base & 0xFFFF));
        Entries[Index].BaseHigh = s_cst(uint64_t, ((uint64_t)Base >> 16 /* & 0xFFFF */));
        Entries[Index].SegmentSelector = SegmentSelector;
        Entries[Index].Flags = Gate;
        Entries[Index].Reserved1 = 0;
        Entries[Index].Reserved2 = 0;
        Entries[Index].Reserved3 = 0;
        Entries[Index].InterruptStackTable = InterruptStackTable;
        Entries[Index].Ring = Ring;
        Entries[Index].Present = Present;
    }

    extern "C" __naked __used __no_stack_protector __aligned(16) void ExceptionHandlerStub()
    {
        asm("cld\n"

            "pushq %rax\n"
            "pushq %rbx\n"
            "pushq %rcx\n"
            "pushq %rdx\n"
            "pushq %rsi\n"
            "pushq %rdi\n"
            "pushq %rbp\n"
            "pushq %r8\n"
            "pushq %r9\n"
            "pushq %r10\n"
            "pushq %r11\n"
            "pushq %r12\n"
            "pushq %r13\n"
            "pushq %r14\n"
            "pushq %r15\n"

            "movq %rsp, %rdi\n"
            "call ExceptionHandler\n"

            "popq %r15\n"
            "popq %r14\n"
            "popq %r13\n"
            "popq %r12\n"
            "popq %r11\n"
            "popq %r10\n"
            "popq %r9\n"
            "popq %r8\n"
            "popq %rbp\n"
            "popq %rdi\n"
            "popq %rsi\n"
            "popq %rdx\n"
            "popq %rcx\n"
            "popq %rbx\n"
            "popq %rax\n"

            "addq $16, %rsp\n"

            "iretq"); // pop CS RIP RFLAGS SS RSP
    }

    extern "C" __naked __used __no_stack_protector __aligned(16) void InterruptHandlerStub()
    {
        asm("cld\n"

            "pushq %rax\n"
            "pushq %rbx\n"
            "pushq %rcx\n"
            "pushq %rdx\n"
            "pushq %rsi\n"
            "pushq %rdi\n"
            "pushq %rbp\n"
            "pushq %r8\n"
            "pushq %r9\n"
            "pushq %r10\n"
            "pushq %r11\n"
            "pushq %r12\n"
            "pushq %r13\n"
            "pushq %r14\n"
            "pushq %r15\n"

            "movq %rsp, %rdi\n"
            "call MainInterruptHandler\n"

            "popq %r15\n"
            "popq %r14\n"
            "popq %r13\n"
            "popq %r12\n"
            "popq %r11\n"
            "popq %r10\n"
            "popq %r9\n"
            "popq %r8\n"
            "popq %rbp\n"
            "popq %rdi\n"
            "popq %rsi\n"
            "popq %rdx\n"
            "popq %rcx\n"
            "popq %rbx\n"
            "popq %rax\n"

            "addq $16, %rsp\n"

            "iretq"); // pop CS RIP RFLAGS SS RSP
    }

#pragma region Exceptions

#define EXCEPTION_HANDLER(num)                                                             \
    __naked __used __no_stack_protector __aligned(16) static void InterruptHandler_##num() \
    {                                                                                      \
        asm("pushq $0\npushq $" #num "\n"                                                  \
            "jmp ExceptionHandlerStub");                                                   \
    }

#define EXCEPTION_ERROR_HANDLER(num)                                                       \
    __naked __used __no_stack_protector __aligned(16) static void InterruptHandler_##num() \
    {                                                                                      \
        asm("pushq $" #num "\n"                                                            \
            "jmp ExceptionHandlerStub");                                                   \
    }

#define INTERRUPT_HANDLER(num)                                                      \
    __naked __used __no_stack_protector __aligned(16) void InterruptHandler_##num() \
    {                                                                               \
        asm("pushq $0\npushq $" #num "\n"                                           \
            "jmp InterruptHandlerStub\n");                                          \
    }

    /* ISR */

    EXCEPTION_HANDLER(0x0);
    EXCEPTION_HANDLER(0x1);
    EXCEPTION_HANDLER(0x2);
    EXCEPTION_HANDLER(0x3);
    EXCEPTION_HANDLER(0x4);
    EXCEPTION_HANDLER(0x5);
    EXCEPTION_HANDLER(0x6);
    EXCEPTION_HANDLER(0x7);
    EXCEPTION_ERROR_HANDLER(0x8);
    EXCEPTION_HANDLER(0x9);
    EXCEPTION_ERROR_HANDLER(0xa);
    EXCEPTION_ERROR_HANDLER(0xb);
    EXCEPTION_ERROR_HANDLER(0xc);
    EXCEPTION_ERROR_HANDLER(0xd);
    EXCEPTION_ERROR_HANDLER(0xe);
    EXCEPTION_HANDLER(0xf);
    EXCEPTION_ERROR_HANDLER(0x10);
    EXCEPTION_HANDLER(0x11);
    EXCEPTION_HANDLER(0x12);
    EXCEPTION_HANDLER(0x13);
    EXCEPTION_HANDLER(0x14);
    EXCEPTION_HANDLER(0x15);
    EXCEPTION_HANDLER(0x16);
    EXCEPTION_HANDLER(0x17);
    EXCEPTION_HANDLER(0x18);
    EXCEPTION_HANDLER(0x19);
    EXCEPTION_HANDLER(0x1a);
    EXCEPTION_HANDLER(0x1b);
    EXCEPTION_HANDLER(0x1c);
    EXCEPTION_HANDLER(0x1d);
    EXCEPTION_HANDLER(0x1e);
    EXCEPTION_HANDLER(0x1f);

    /* IRQ */

    INTERRUPT_HANDLER(0x20)
    INTERRUPT_HANDLER(0x21)
    INTERRUPT_HANDLER(0x22)
    INTERRUPT_HANDLER(0x23)
    INTERRUPT_HANDLER(0x24)
    INTERRUPT_HANDLER(0x25)
    INTERRUPT_HANDLER(0x26)
    INTERRUPT_HANDLER(0x27)
    INTERRUPT_HANDLER(0x28)
    INTERRUPT_HANDLER(0x29)
    INTERRUPT_HANDLER(0x2a)
    INTERRUPT_HANDLER(0x2b)
    INTERRUPT_HANDLER(0x2c)
    INTERRUPT_HANDLER(0x2d)
    INTERRUPT_HANDLER(0x2e)
    INTERRUPT_HANDLER(0x2f)

    /* Reserved by OS */

    INTERRUPT_HANDLER(0x30)
    INTERRUPT_HANDLER(0x31)
    INTERRUPT_HANDLER(0x32)
    INTERRUPT_HANDLER(0x33)
    INTERRUPT_HANDLER(0x34)
    INTERRUPT_HANDLER(0x35)
    INTERRUPT_HANDLER(0x36)
    INTERRUPT_HANDLER(0x37)
    INTERRUPT_HANDLER(0x38)
    INTERRUPT_HANDLER(0x39)
    INTERRUPT_HANDLER(0x3a)
    INTERRUPT_HANDLER(0x3b)
    INTERRUPT_HANDLER(0x3c)
    INTERRUPT_HANDLER(0x3d)

    /* Free */

    INTERRUPT_HANDLER(0x3e)
    INTERRUPT_HANDLER(0x3f)
    INTERRUPT_HANDLER(0x40)
    INTERRUPT_HANDLER(0x41)
    INTERRUPT_HANDLER(0x42)
    INTERRUPT_HANDLER(0x43)
    INTERRUPT_HANDLER(0x44)
    INTERRUPT_HANDLER(0x45)
    INTERRUPT_HANDLER(0x46)
    INTERRUPT_HANDLER(0x47)
    INTERRUPT_HANDLER(0x48)
    INTERRUPT_HANDLER(0x49)
    INTERRUPT_HANDLER(0x4a)
    INTERRUPT_HANDLER(0x4b)
    INTERRUPT_HANDLER(0x4c)
    INTERRUPT_HANDLER(0x4d)
    INTERRUPT_HANDLER(0x4e)
    INTERRUPT_HANDLER(0x4f)
    INTERRUPT_HANDLER(0x50)
    INTERRUPT_HANDLER(0x51)
    INTERRUPT_HANDLER(0x52)
    INTERRUPT_HANDLER(0x53)
    INTERRUPT_HANDLER(0x54)
    INTERRUPT_HANDLER(0x55)
    INTERRUPT_HANDLER(0x56)
    INTERRUPT_HANDLER(0x57)
    INTERRUPT_HANDLER(0x58)
    INTERRUPT_HANDLER(0x59)
    INTERRUPT_HANDLER(0x5a)
    INTERRUPT_HANDLER(0x5b)
    INTERRUPT_HANDLER(0x5c)
    INTERRUPT_HANDLER(0x5d)
    INTERRUPT_HANDLER(0x5e)
    INTERRUPT_HANDLER(0x5f)
    INTERRUPT_HANDLER(0x60)
    INTERRUPT_HANDLER(0x61)
    INTERRUPT_HANDLER(0x62)
    INTERRUPT_HANDLER(0x63)
    INTERRUPT_HANDLER(0x64)
    INTERRUPT_HANDLER(0x65)
    INTERRUPT_HANDLER(0x66)
    INTERRUPT_HANDLER(0x67)
    INTERRUPT_HANDLER(0x68)
    INTERRUPT_HANDLER(0x69)
    INTERRUPT_HANDLER(0x6a)
    INTERRUPT_HANDLER(0x6b)
    INTERRUPT_HANDLER(0x6c)
    INTERRUPT_HANDLER(0x6d)
    INTERRUPT_HANDLER(0x6e)
    INTERRUPT_HANDLER(0x6f)
    INTERRUPT_HANDLER(0x70)
    INTERRUPT_HANDLER(0x71)
    INTERRUPT_HANDLER(0x72)
    INTERRUPT_HANDLER(0x73)
    INTERRUPT_HANDLER(0x74)
    INTERRUPT_HANDLER(0x75)
    INTERRUPT_HANDLER(0x76)
    INTERRUPT_HANDLER(0x77)
    INTERRUPT_HANDLER(0x78)
    INTERRUPT_HANDLER(0x79)
    INTERRUPT_HANDLER(0x7a)
    INTERRUPT_HANDLER(0x7b)
    INTERRUPT_HANDLER(0x7c)
    INTERRUPT_HANDLER(0x7d)
    INTERRUPT_HANDLER(0x7e)
    INTERRUPT_HANDLER(0x7f)
    INTERRUPT_HANDLER(0x80)
    INTERRUPT_HANDLER(0x81)
    INTERRUPT_HANDLER(0x82)
    INTERRUPT_HANDLER(0x83)
    INTERRUPT_HANDLER(0x84)
    INTERRUPT_HANDLER(0x85)
    INTERRUPT_HANDLER(0x86)
    INTERRUPT_HANDLER(0x87)
    INTERRUPT_HANDLER(0x88)
    INTERRUPT_HANDLER(0x89)
    INTERRUPT_HANDLER(0x8a)
    INTERRUPT_HANDLER(0x8b)
    INTERRUPT_HANDLER(0x8c)
    INTERRUPT_HANDLER(0x8d)
    INTERRUPT_HANDLER(0x8e)
    INTERRUPT_HANDLER(0x8f)
    INTERRUPT_HANDLER(0x90)
    INTERRUPT_HANDLER(0x91)
    INTERRUPT_HANDLER(0x92)
    INTERRUPT_HANDLER(0x93)
    INTERRUPT_HANDLER(0x94)
    INTERRUPT_HANDLER(0x95)
    INTERRUPT_HANDLER(0x96)
    INTERRUPT_HANDLER(0x97)
    INTERRUPT_HANDLER(0x98)
    INTERRUPT_HANDLER(0x99)
    INTERRUPT_HANDLER(0x9a)
    INTERRUPT_HANDLER(0x9b)
    INTERRUPT_HANDLER(0x9c)
    INTERRUPT_HANDLER(0x9d)
    INTERRUPT_HANDLER(0x9e)
    INTERRUPT_HANDLER(0x9f)
    INTERRUPT_HANDLER(0xa0)
    INTERRUPT_HANDLER(0xa1)
    INTERRUPT_HANDLER(0xa2)
    INTERRUPT_HANDLER(0xa3)
    INTERRUPT_HANDLER(0xa4)
    INTERRUPT_HANDLER(0xa5)
    INTERRUPT_HANDLER(0xa6)
    INTERRUPT_HANDLER(0xa7)
    INTERRUPT_HANDLER(0xa8)
    INTERRUPT_HANDLER(0xa9)
    INTERRUPT_HANDLER(0xaa)
    INTERRUPT_HANDLER(0xab)
    INTERRUPT_HANDLER(0xac)
    INTERRUPT_HANDLER(0xad)
    INTERRUPT_HANDLER(0xae)
    INTERRUPT_HANDLER(0xaf)
    INTERRUPT_HANDLER(0xb0)
    INTERRUPT_HANDLER(0xb1)
    INTERRUPT_HANDLER(0xb2)
    INTERRUPT_HANDLER(0xb3)
    INTERRUPT_HANDLER(0xb4)
    INTERRUPT_HANDLER(0xb5)
    INTERRUPT_HANDLER(0xb6)
    INTERRUPT_HANDLER(0xb7)
    INTERRUPT_HANDLER(0xb8)
    INTERRUPT_HANDLER(0xb9)
    INTERRUPT_HANDLER(0xba)
    INTERRUPT_HANDLER(0xbb)
    INTERRUPT_HANDLER(0xbc)
    INTERRUPT_HANDLER(0xbd)
    INTERRUPT_HANDLER(0xbe)
    INTERRUPT_HANDLER(0xbf)
    INTERRUPT_HANDLER(0xc0)
    INTERRUPT_HANDLER(0xc1)
    INTERRUPT_HANDLER(0xc2)
    INTERRUPT_HANDLER(0xc3)
    INTERRUPT_HANDLER(0xc4)
    INTERRUPT_HANDLER(0xc5)
    INTERRUPT_HANDLER(0xc6)
    INTERRUPT_HANDLER(0xc7)
    INTERRUPT_HANDLER(0xc8)
    INTERRUPT_HANDLER(0xc9)
    INTERRUPT_HANDLER(0xca)
    INTERRUPT_HANDLER(0xcb)
    INTERRUPT_HANDLER(0xcc)
    INTERRUPT_HANDLER(0xcd)
    INTERRUPT_HANDLER(0xce)
    INTERRUPT_HANDLER(0xcf)
    INTERRUPT_HANDLER(0xd0)
    INTERRUPT_HANDLER(0xd1)
    INTERRUPT_HANDLER(0xd2)
    INTERRUPT_HANDLER(0xd3)
    INTERRUPT_HANDLER(0xd4)
    INTERRUPT_HANDLER(0xd5)
    INTERRUPT_HANDLER(0xd6)
    INTERRUPT_HANDLER(0xd7)
    INTERRUPT_HANDLER(0xd8)
    INTERRUPT_HANDLER(0xd9)
    INTERRUPT_HANDLER(0xda)
    INTERRUPT_HANDLER(0xdb)
    INTERRUPT_HANDLER(0xdc)
    INTERRUPT_HANDLER(0xdd)
    INTERRUPT_HANDLER(0xde)
    INTERRUPT_HANDLER(0xdf)
    INTERRUPT_HANDLER(0xe0)
    INTERRUPT_HANDLER(0xe1)
    INTERRUPT_HANDLER(0xe2)
    INTERRUPT_HANDLER(0xe3)
    INTERRUPT_HANDLER(0xe4)
    INTERRUPT_HANDLER(0xe5)
    INTERRUPT_HANDLER(0xe6)
    INTERRUPT_HANDLER(0xe7)
    INTERRUPT_HANDLER(0xe8)
    INTERRUPT_HANDLER(0xe9)
    INTERRUPT_HANDLER(0xea)
    INTERRUPT_HANDLER(0xeb)
    INTERRUPT_HANDLER(0xec)
    INTERRUPT_HANDLER(0xed)
    INTERRUPT_HANDLER(0xee)
    INTERRUPT_HANDLER(0xef)
    INTERRUPT_HANDLER(0xf0)
    INTERRUPT_HANDLER(0xf1)
    INTERRUPT_HANDLER(0xf2)
    INTERRUPT_HANDLER(0xf3)
    INTERRUPT_HANDLER(0xf4)
    INTERRUPT_HANDLER(0xf5)
    INTERRUPT_HANDLER(0xf6)
    INTERRUPT_HANDLER(0xf7)
    INTERRUPT_HANDLER(0xf8)
    INTERRUPT_HANDLER(0xf9)
    INTERRUPT_HANDLER(0xfa)
    INTERRUPT_HANDLER(0xfb)
    INTERRUPT_HANDLER(0xfc)
    INTERRUPT_HANDLER(0xfd)
    INTERRUPT_HANDLER(0xfe)
    INTERRUPT_HANDLER(0xff)

#pragma endregion Exceptions

    void Init(int Core)
    {
        if (Core == 0) /* Disable PIC using BSP */
        {
            // PIC
            outb(0x20, 0x10 | 0x1);
            outb(0x80, 0);
            outb(0xA0, 0x10 | 0x10);
            outb(0x80, 0);

            outb(0x21, 0x20);
            outb(0x80, 0);
            outb(0xA1, 0x28);
            outb(0x80, 0);

            outb(0x21, 0x04);
            outb(0x80, 0);
            outb(0xA1, 0x02);
            outb(0x80, 0);

            outb(0x21, 1);
            outb(0x80, 0);
            outb(0xA1, 1);
            outb(0x80, 0);

            // Masking and disabling PIC
            outb(0x21, 0xff);
            outb(0x80, 0);
            outb(0xA1, 0xff);
        }

        /* ISR */

#ifdef DEBUG
        if (!DebuggerIsAttached)
        {
#endif
            SetEntry(0x0, InterruptHandler_0x0, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1, InterruptHandler_0x1, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x2, InterruptHandler_0x2, 2, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x3, InterruptHandler_0x3, 1, TRAP_32BIT, RING3, (!DebuggerIsAttached), GDT_KERNEL_CODE); /* Do not handle breakpoints if we are debugging the kernel. */
            SetEntry(0x4, InterruptHandler_0x4, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x5, InterruptHandler_0x5, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x6, InterruptHandler_0x6, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x7, InterruptHandler_0x7, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x8, InterruptHandler_0x8, 3, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x9, InterruptHandler_0x9, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xa, InterruptHandler_0xa, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xb, InterruptHandler_0xb, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xc, InterruptHandler_0xc, 3, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xd, InterruptHandler_0xd, 3, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xe, InterruptHandler_0xe, 3, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0xf, InterruptHandler_0xf, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x10, InterruptHandler_0x10, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x11, InterruptHandler_0x11, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x12, InterruptHandler_0x12, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x13, InterruptHandler_0x13, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x14, InterruptHandler_0x14, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x15, InterruptHandler_0x15, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x16, InterruptHandler_0x16, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x17, InterruptHandler_0x17, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x18, InterruptHandler_0x18, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x19, InterruptHandler_0x19, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1a, InterruptHandler_0x1a, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1b, InterruptHandler_0x1b, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1c, InterruptHandler_0x1c, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1d, InterruptHandler_0x1d, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1e, InterruptHandler_0x1e, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
            SetEntry(0x1f, InterruptHandler_0x1f, 1, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
#ifdef DEBUG
        }
        else
            KPrint("\eFFA500The debugger is attached, not setting up the ISR.");
#endif

        /* IRQ */

        SetEntry(0x20, InterruptHandler_0x20, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x21, InterruptHandler_0x21, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x22, InterruptHandler_0x22, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x23, InterruptHandler_0x23, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x24, InterruptHandler_0x24, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x25, InterruptHandler_0x25, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x26, InterruptHandler_0x26, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x27, InterruptHandler_0x27, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x28, InterruptHandler_0x28, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x29, InterruptHandler_0x29, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2a, InterruptHandler_0x2a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2b, InterruptHandler_0x2b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2c, InterruptHandler_0x2c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2d, InterruptHandler_0x2d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2e, InterruptHandler_0x2e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x2f, InterruptHandler_0x2f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);

        /* Reserved by OS */

        SetEntry(0x30, InterruptHandler_0x30, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x31, InterruptHandler_0x31, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x32, InterruptHandler_0x32, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x33, InterruptHandler_0x33, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x34, InterruptHandler_0x34, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x35, InterruptHandler_0x35, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x36, InterruptHandler_0x36, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x37, InterruptHandler_0x37, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x38, InterruptHandler_0x38, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x39, InterruptHandler_0x39, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x3a, InterruptHandler_0x3a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x3b, InterruptHandler_0x3b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x3c, InterruptHandler_0x3c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x3d, InterruptHandler_0x3d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);

        /* Free */

        SetEntry(0x3e, InterruptHandler_0x3e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x3f, InterruptHandler_0x3f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x40, InterruptHandler_0x40, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x41, InterruptHandler_0x41, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x42, InterruptHandler_0x42, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x43, InterruptHandler_0x43, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x44, InterruptHandler_0x44, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x45, InterruptHandler_0x45, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x46, InterruptHandler_0x46, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x47, InterruptHandler_0x47, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x48, InterruptHandler_0x48, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x49, InterruptHandler_0x49, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4a, InterruptHandler_0x4a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4b, InterruptHandler_0x4b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4c, InterruptHandler_0x4c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4d, InterruptHandler_0x4d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4e, InterruptHandler_0x4e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x4f, InterruptHandler_0x4f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x50, InterruptHandler_0x50, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x51, InterruptHandler_0x51, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x52, InterruptHandler_0x52, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x53, InterruptHandler_0x53, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x54, InterruptHandler_0x54, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x55, InterruptHandler_0x55, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x56, InterruptHandler_0x56, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x57, InterruptHandler_0x57, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x58, InterruptHandler_0x58, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x59, InterruptHandler_0x59, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5a, InterruptHandler_0x5a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5b, InterruptHandler_0x5b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5c, InterruptHandler_0x5c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5d, InterruptHandler_0x5d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5e, InterruptHandler_0x5e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x5f, InterruptHandler_0x5f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x60, InterruptHandler_0x60, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x61, InterruptHandler_0x61, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x62, InterruptHandler_0x62, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x63, InterruptHandler_0x63, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x64, InterruptHandler_0x64, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x65, InterruptHandler_0x65, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x66, InterruptHandler_0x66, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x67, InterruptHandler_0x67, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x68, InterruptHandler_0x68, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x69, InterruptHandler_0x69, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6a, InterruptHandler_0x6a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6b, InterruptHandler_0x6b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6c, InterruptHandler_0x6c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6d, InterruptHandler_0x6d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6e, InterruptHandler_0x6e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x6f, InterruptHandler_0x6f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x70, InterruptHandler_0x70, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x71, InterruptHandler_0x71, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x72, InterruptHandler_0x72, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x73, InterruptHandler_0x73, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x74, InterruptHandler_0x74, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x75, InterruptHandler_0x75, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x76, InterruptHandler_0x76, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x77, InterruptHandler_0x77, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x78, InterruptHandler_0x78, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x79, InterruptHandler_0x79, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7a, InterruptHandler_0x7a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7b, InterruptHandler_0x7b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7c, InterruptHandler_0x7c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7d, InterruptHandler_0x7d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7e, InterruptHandler_0x7e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x7f, InterruptHandler_0x7f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x80, InterruptHandler_0x80, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x81, InterruptHandler_0x81, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x82, InterruptHandler_0x82, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x83, InterruptHandler_0x83, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x84, InterruptHandler_0x84, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x85, InterruptHandler_0x85, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x86, InterruptHandler_0x86, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x87, InterruptHandler_0x87, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x88, InterruptHandler_0x88, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x89, InterruptHandler_0x89, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8a, InterruptHandler_0x8a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8b, InterruptHandler_0x8b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8c, InterruptHandler_0x8c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8d, InterruptHandler_0x8d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8e, InterruptHandler_0x8e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x8f, InterruptHandler_0x8f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x90, InterruptHandler_0x90, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x91, InterruptHandler_0x91, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x92, InterruptHandler_0x92, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x93, InterruptHandler_0x93, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x94, InterruptHandler_0x94, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x95, InterruptHandler_0x95, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x96, InterruptHandler_0x96, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x97, InterruptHandler_0x97, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x98, InterruptHandler_0x98, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x99, InterruptHandler_0x99, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9a, InterruptHandler_0x9a, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9b, InterruptHandler_0x9b, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9c, InterruptHandler_0x9c, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9d, InterruptHandler_0x9d, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9e, InterruptHandler_0x9e, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0x9f, InterruptHandler_0x9f, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa0, InterruptHandler_0xa0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa1, InterruptHandler_0xa1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa2, InterruptHandler_0xa2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa3, InterruptHandler_0xa3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa4, InterruptHandler_0xa4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa5, InterruptHandler_0xa5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa6, InterruptHandler_0xa6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa7, InterruptHandler_0xa7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa8, InterruptHandler_0xa8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xa9, InterruptHandler_0xa9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xaa, InterruptHandler_0xaa, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xab, InterruptHandler_0xab, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xac, InterruptHandler_0xac, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xad, InterruptHandler_0xad, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xae, InterruptHandler_0xae, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xaf, InterruptHandler_0xaf, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb0, InterruptHandler_0xb0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb1, InterruptHandler_0xb1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb2, InterruptHandler_0xb2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb3, InterruptHandler_0xb3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb4, InterruptHandler_0xb4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb5, InterruptHandler_0xb5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb6, InterruptHandler_0xb6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb7, InterruptHandler_0xb7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb8, InterruptHandler_0xb8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xb9, InterruptHandler_0xb9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xba, InterruptHandler_0xba, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xbb, InterruptHandler_0xbb, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xbc, InterruptHandler_0xbc, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xbd, InterruptHandler_0xbd, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xbe, InterruptHandler_0xbe, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xbf, InterruptHandler_0xbf, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc0, InterruptHandler_0xc0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc1, InterruptHandler_0xc1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc2, InterruptHandler_0xc2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc3, InterruptHandler_0xc3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc4, InterruptHandler_0xc4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc5, InterruptHandler_0xc5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc6, InterruptHandler_0xc6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc7, InterruptHandler_0xc7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc8, InterruptHandler_0xc8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xc9, InterruptHandler_0xc9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xca, InterruptHandler_0xca, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xcb, InterruptHandler_0xcb, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xcc, InterruptHandler_0xcc, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xcd, InterruptHandler_0xcd, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xce, InterruptHandler_0xce, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xcf, InterruptHandler_0xcf, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd0, InterruptHandler_0xd0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd1, InterruptHandler_0xd1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd2, InterruptHandler_0xd2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd3, InterruptHandler_0xd3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd4, InterruptHandler_0xd4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd5, InterruptHandler_0xd5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd6, InterruptHandler_0xd6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd7, InterruptHandler_0xd7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd8, InterruptHandler_0xd8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xd9, InterruptHandler_0xd9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xda, InterruptHandler_0xda, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xdb, InterruptHandler_0xdb, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xdc, InterruptHandler_0xdc, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xdd, InterruptHandler_0xdd, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xde, InterruptHandler_0xde, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xdf, InterruptHandler_0xdf, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe0, InterruptHandler_0xe0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe1, InterruptHandler_0xe1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe2, InterruptHandler_0xe2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe3, InterruptHandler_0xe3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe4, InterruptHandler_0xe4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe5, InterruptHandler_0xe5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe6, InterruptHandler_0xe6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe7, InterruptHandler_0xe7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe8, InterruptHandler_0xe8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xe9, InterruptHandler_0xe9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xea, InterruptHandler_0xea, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xeb, InterruptHandler_0xeb, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xec, InterruptHandler_0xec, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xed, InterruptHandler_0xed, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xee, InterruptHandler_0xee, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xef, InterruptHandler_0xef, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf0, InterruptHandler_0xf0, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf1, InterruptHandler_0xf1, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf2, InterruptHandler_0xf2, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf3, InterruptHandler_0xf3, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf4, InterruptHandler_0xf4, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf5, InterruptHandler_0xf5, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf6, InterruptHandler_0xf6, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf7, InterruptHandler_0xf7, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf8, InterruptHandler_0xf8, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xf9, InterruptHandler_0xf9, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xfa, InterruptHandler_0xfa, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xfb, InterruptHandler_0xfb, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xfc, InterruptHandler_0xfc, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xfd, InterruptHandler_0xfd, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xfe, InterruptHandler_0xfe, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        SetEntry(0xff, InterruptHandler_0xff, 0, TRAP_32BIT, RING0, true, GDT_KERNEL_CODE);
        CPU::x64::lidt(&idtd);
    }
}
