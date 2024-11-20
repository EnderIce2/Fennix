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

#include "gdt.hpp"

#include <memory.hpp>
#include <smp.hpp>
#include <cpu.hpp>
#include <debug.h>

namespace GlobalDescriptorTable
{
    static GlobalDescriptorTableEntries GDTEntriesTemplate = {
        .Null =
            {
                .Length = 0x0,
                .BaseLow = 0x0,
                .BaseMiddle = 0x0,
                .Access = {.Raw = 0x0},
                .Flags = {.Raw = 0x0},
                .BaseHigh = 0x0,
            },

        .Code =
            {
                .Length = 0x0,
                .BaseLow = 0x0,
                .BaseMiddle = 0x0,
                .Access = {
                    .A = 0,
                    .RW = 1,
                    .DC = 0,
                    .E = 1,
                    .S = 1,
                    .DPL = 0,
                    .P = 1,
                },
                .Flags = {
                    .Unknown = 0x0,
                    .L = 1,
                },
                .BaseHigh = 0x0,
            },

        .Data = {
            .Length = 0x0,
            .BaseLow = 0x0,
            .BaseMiddle = 0x0,
            .Access = {
                .A = 0,
                .RW = 1,
                .DC = 0,
                .E = 0,
                .S = 1,
                .DPL = 0,
                .P = 1,
            },
            .Flags = {.Raw = 0x0},
            .BaseHigh = 0x0,
        },

        .UserData = {
            .Length = 0x0,
            .BaseLow = 0x0,
            .BaseMiddle = 0x0,
            .Access = {
                .A = 0,
                .RW = 1,
                .DC = 0,
                .E = 0,
                .S = 1,
                .DPL = 3,
                .P = 1,
            },
            .Flags = {
                .Raw = 0x0,
            },
            .BaseHigh = 0x0,
        },

        .UserCode = {
            .Length = 0x0,
            .BaseLow = 0x0,
            .BaseMiddle = 0x0,
            .Access = {
                .A = 0,
                .RW = 1,
                .DC = 0,
                .E = 1,
                .S = 1,
                .DPL = 3,
                .P = 1,
            },
            .Flags = {
                .Unknown = 0x0,
                .L = 1,
            },
            .BaseHigh = 0x0,
        },

        .TaskStateSegment = {},
    };

    GlobalDescriptorTableEntries GDTEntries[MAX_CPU] __aligned(16);
    GlobalDescriptorTableDescriptor gdt[MAX_CPU] __aligned(16);

    TaskStateSegment tss[MAX_CPU] = {
        0,
        {0, 0, 0},
        0,
        {0, 0, 0, 0, 0, 0, 0},
        0,
        0,
        0,
    };

    void *CPUStackPointer[MAX_CPU];

    SafeFunction void Init(int Core)
    {
        memcpy(&GDTEntries[Core], &GDTEntriesTemplate, sizeof(GlobalDescriptorTableEntries));
        gdt[Core] = {.Length = sizeof(GlobalDescriptorTableEntries) - 1, .Entries = &GDTEntries[Core]};

        debug("Kernel: Code Access: %ld; Data Access: %ld", GDTEntries[Core].Code.Access.Raw, GDTEntries[Core].Data.Access.Raw);
        debug("Kernel: Code Flags: %ld; Data Flags: %ld", GDTEntries[Core].Code.Flags.Raw, GDTEntries[Core].Data.Flags.Raw);
        debug("User: Code Access: %ld; Data Access: %ld", GDTEntries[Core].UserCode.Access.Raw, GDTEntries[Core].UserData.Access.Raw);
        debug("User: Code Flags: %ld; Data Flags: %ld", GDTEntries[Core].UserCode.Flags.Raw, GDTEntries[Core].UserData.Flags.Raw);
        CPU::x64::lgdt(&gdt[Core]);

        asmv("movq %%rsp, %%rax\n"
             "pushq $16\n"
             "pushq %%rax\n"
             "pushfq\n"
             "pushq $8\n"
             "pushq $1f\n"
             "iretq\n"
             "1:\n"
             "movw $16, %%ax\n"
             "movw %%ax, %%ds\n"
             "movw %%ax, %%es\n" ::
                 : "memory", "rax");

        CPUStackPointer[Core] = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1));
        memset(CPUStackPointer[Core], 0, STACK_SIZE);
        debug("CPU %d Stack Pointer: %#lx-%#lx (%d pages)", Core,
              CPUStackPointer[Core], (uintptr_t)CPUStackPointer[Core] + STACK_SIZE,
              TO_PAGES(STACK_SIZE + 1));

        uintptr_t Base = (uintptr_t)&tss[Core];
        size_t Limit = Base + sizeof(TaskStateSegment);
        gdt[Core].Entries->TaskStateSegment.Length = Limit & 0xFFFF;
        gdt[Core].Entries->TaskStateSegment.BaseLow = Base & 0xFFFF;
        gdt[Core].Entries->TaskStateSegment.BaseMiddle = (Base >> 16) & 0xFF;
        gdt[Core].Entries->TaskStateSegment.BaseHigh = (Base >> 24) & 0xFF;
        gdt[Core].Entries->TaskStateSegment.BaseUpper = s_cst(uint32_t, (Base >> 32) & 0xFFFFFFFF);
        gdt[Core].Entries->TaskStateSegment.Flags = {.A = 1, .RW = 0, .DC = 0, .E = 1, .S = 0, .DPL = 0, .P = 1};
        gdt[Core].Entries->TaskStateSegment.Granularity = (0 << 4) | ((Limit >> 16) & 0xF);

        tss[Core].IOMapBaseAddressOffset = sizeof(TaskStateSegment);
        tss[Core].StackPointer[0] = (uint64_t)CPUStackPointer[Core] + STACK_SIZE;

        for (size_t i = 0; i < sizeof(tss[Core].InterruptStackTable) / sizeof(tss[Core].InterruptStackTable[7]); i++)
        {
            void *NewStack = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1));

            tss[Core].InterruptStackTable[i] = (uint64_t)NewStack + STACK_SIZE;
            memset((void *)(tss[Core].InterruptStackTable[i] - STACK_SIZE), 0, STACK_SIZE);
            debug("IST-%d: %#lx-%#lx", i, NewStack, (uintptr_t)NewStack + STACK_SIZE);
        }

        CPU::x64::ltr(GDT_TSS);

        debug("GDT_KERNEL_CODE: %#lx", GDT_KERNEL_CODE);
        debug("GDT_KERNEL_DATA: %#lx", GDT_KERNEL_DATA);
        debug("GDT_USER_CODE: %#lx", GDT_USER_CODE);
        debug("GDT_USER_DATA: %#lx", GDT_USER_DATA);
        debug("GDT_TSS: %#lx", GDT_TSS);
        debug("Global Descriptor Table initialized");
    }

    SafeFunction void SetKernelStack(void *Stack)
    {
        long CPUID = GetCurrentCPU()->ID;
        if (Stack != nullptr)
            tss[CPUID].StackPointer[0] = (uint64_t)Stack;
        else
            tss[CPUID].StackPointer[0] = (uint64_t)CPUStackPointer[CPUID] + STACK_SIZE;

        /*
        FIXME: There's a bug in kernel which if
        we won't update "tss[CPUID].StackPointer[0]"
        with the current stack pointer, the kernel
        will crash.
        */
        asmv("mov %%rsp, %0"
             : "=r"(tss[CPUID].StackPointer[0]));
    }

    void *GetKernelStack() { return (void *)tss[GetCurrentCPU()->ID].StackPointer[0]; }
}
