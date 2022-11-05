#include "gdt.hpp"

#include <memory.hpp>
#include <smp.hpp>
#include <cpu.hpp>
#include <debug.h>

namespace GlobalDescriptorTable
{
    static GlobalDescriptorTableEntries GDTEntries = {
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
        {.Length = 0x0,
         .Low = 0x0,
         .Middle = 0x0,
         .Flags1 = 0b10001001,
         .Flags2 = 0b00000000,
         .High = 0x0,
         .Upper32 = 0x0,
         .Reserved = 0x0}};

    GlobalDescriptorTableDescriptor gdt = {.Length = sizeof(GlobalDescriptorTableEntries) - 1, .Entries = &GDTEntries};

    TaskStateSegment tss[MAX_CPU] = {
        0,
        {0, 0, 0},
        0,
        {0, 0, 0, 0, 0, 0, 0},
        0,
        0,
    };

    __attribute__((no_stack_protector)) void Init(int Core)
    {
        debug("Kernel: Code Access: %ld; Data Access: %ld", GDTEntries.Code.Access.Raw, GDTEntries.Data.Access.Raw);
        debug("Kernel: Code Flags: %ld; Data Flags: %ld", GDTEntries.Code.Flags.Raw, GDTEntries.Data.Flags.Raw);
        debug("User: Code Access: %ld; Data Access: %ld", GDTEntries.UserCode.Access.Raw, GDTEntries.UserData.Access.Raw);
        debug("User: Code Flags: %ld; Data Flags: %ld", GDTEntries.UserCode.Flags.Raw, GDTEntries.UserData.Flags.Raw);
        CPU::x64::lgdt(&gdt);

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

        uint64_t Base = (uint64_t)&tss[Core];
        gdt.Entries->TaskStateSegment.Length = Base + sizeof(tss[0]);
        gdt.Entries->TaskStateSegment.Low = (uint16_t)(Base & 0xFFFF);
        gdt.Entries->TaskStateSegment.Middle = (uint8_t)((Base >> 16) & 0xFF);
        gdt.Entries->TaskStateSegment.High = (uint8_t)((Base >> 24) & 0xFF);
        gdt.Entries->TaskStateSegment.Upper32 = (uint32_t)((Base >> 32) & 0xFFFFFFFF);
        gdt.Entries->TaskStateSegment.Flags1 = 0b10001001;
        gdt.Entries->TaskStateSegment.Flags2 = 0b00000000;
        tss[Core].IOMapBaseAddressOffset = sizeof(TaskStateSegment);
        tss[Core].StackPointer[0] = (uint64_t)KernelAllocator.RequestPage();
        tss[Core].InterruptStackTable[0] = (uint64_t)KernelAllocator.RequestPage();
        tss[Core].InterruptStackTable[1] = (uint64_t)KernelAllocator.RequestPage();
        tss[Core].InterruptStackTable[2] = (uint64_t)KernelAllocator.RequestPage();

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
}
