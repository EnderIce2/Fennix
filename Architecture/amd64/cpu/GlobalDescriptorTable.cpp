#include "gdt.hpp"

#include <memory.hpp>
#include <smp.hpp>
#include <cpu.hpp>
#include <debug.h>

namespace GlobalDescriptorTable
{
    static GlobalDescriptorTableEntries GDTEntriesTemplate = {
        // null
        {.Length = 0x0,
         .BaseLow = 0x0,
         .BaseMiddle = 0x0,
         .Access = {.Raw = 0x0},
         .Flags = {.Raw = 0x0},
         .BaseHigh = 0x0},

        // kernel code
        {.Length = 0x0,
         .BaseLow = 0x0,
         .BaseMiddle = 0x0,
         .Access = {.A = 0,
                    .RW = 1,
                    .DC = 0,
                    .E = 1,
                    .S = 1,
                    .DPL = 0,
                    .P = 1},
         .Flags = {.Unknown = 0x0, .L = 1},
         .BaseHigh = 0x0},

        // kernel data
        {.Length = 0x0,
         .BaseLow = 0x0,
         .BaseMiddle = 0x0,
         .Access = {.A = 0,
                    .RW = 1,
                    .DC = 0,
                    .E = 0,
                    .S = 1,
                    .DPL = 0,
                    .P = 1},
         .Flags = {.Raw = 0x0},
         .BaseHigh = 0x0},

        // user code
        {.Length = 0x0,
         .BaseLow = 0x0,
         .BaseMiddle = 0x0,
         .Access = {.A = 0,
                    .RW = 1,
                    .DC = 0,
                    .E = 1,
                    .S = 1,
                    .DPL = 3,
                    .P = 1},
         .Flags = {.Unknown = 0x0, .L = 1},
         .BaseHigh = 0x0},

        // user data
        {.Length = 0x0,
         .BaseLow = 0x0,
         .BaseMiddle = 0x0,
         .Access = {.A = 0,
                    .RW = 1,
                    .DC = 0,
                    .E = 0,
                    .S = 1,
                    .DPL = 3,
                    .P = 1},
         .Flags = {.Raw = 0x0},
         .BaseHigh = 0x0},

        // tss
        {}};

    static GlobalDescriptorTableEntries GDTEntries[MAX_CPU];
    GlobalDescriptorTableDescriptor gdt[MAX_CPU];

    TaskStateSegment tss[MAX_CPU] = {
        0,
        {0, 0, 0},
        0,
        {0, 0, 0, 0, 0, 0, 0},
        0,
        0,
    };

    void *CPUStackPointer[MAX_CPU];

    __attribute__((no_stack_protector)) void Init(int Core)
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

        CPUStackPointer[Core] = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE));

        uint64_t Base = (uint64_t)&tss[Core];
        uint64_t Limit = Base + sizeof(TaskStateSegment);
        gdt[Core].Entries->TaskStateSegment.Length = Limit & 0xFFFF;
        gdt[Core].Entries->TaskStateSegment.BaseLow = Base & 0xFFFF;
        gdt[Core].Entries->TaskStateSegment.BaseMiddle = (Base >> 16) & 0xFF;
        gdt[Core].Entries->TaskStateSegment.BaseHigh = (Base >> 24) & 0xFF;
        gdt[Core].Entries->TaskStateSegment.BaseUpper = (Base >> 32) & 0xFFFFFFFF;
        gdt[Core].Entries->TaskStateSegment.Flags = {.A = 1, .RW = 0, .DC = 0, .E = 1, .S = 0, .DPL = 0, .P = 1};
        gdt[Core].Entries->TaskStateSegment.Granularity = (0 << 4) | ((Limit >> 16) & 0xF);

        tss[Core].IOMapBaseAddressOffset = sizeof(TaskStateSegment);
        tss[Core].StackPointer[0] = (uint64_t)CPUStackPointer[Core] + STACK_SIZE;
        tss[Core].InterruptStackTable[0] = (uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;
        tss[Core].InterruptStackTable[1] = (uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;
        tss[Core].InterruptStackTable[2] = (uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;

        CPU::x64::ltr(GDT_TSS);
        asmv("mov %%rsp, %0"
             : "=r"(tss[Core].StackPointer[0]));

        trace("GDT_KERNEL_CODE: %#lx", GDT_KERNEL_CODE);
        trace("GDT_KERNEL_DATA: %#lx", GDT_KERNEL_DATA);
        trace("GDT_USER_CODE: %#lx", GDT_USER_CODE);
        trace("GDT_USER_DATA: %#lx", GDT_USER_DATA);
        trace("GDT_TSS: %#lx", GDT_TSS);
        trace("Global Descriptor Table initialized");
    }

    __attribute__((no_stack_protector)) void SetKernelStack(void *Stack)
    {
        if (Stack)
            tss[GetCurrentCPU()->ID].StackPointer[0] = (uint64_t)Stack;
        else
            tss[GetCurrentCPU()->ID].StackPointer[0] = (uint64_t)CPUStackPointer[GetCurrentCPU()->ID] + STACK_SIZE;
    }
}
