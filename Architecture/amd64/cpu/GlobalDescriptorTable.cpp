#include "gdt.hpp"

#include <memory.hpp>
#include <smp.hpp>
#include <cpu.hpp>
#include <debug.h>

namespace GlobalDescriptorTable
{
    static GlobalDescriptorTableEntries GDTEntries = {
        {.Length = 0x0000, .BaseLow = 0x0000, .BaseMiddle = 0x00, .Access = 0b00000000, .Flags = 0b00000000, .BaseHigh = 0x00},    // null
        {.Length = 0x0000, .BaseLow = 0x0000, .BaseMiddle = 0x00, .Access = 0b10011010, .Flags = 0b00100000, .BaseHigh = 0x00},    // kernel code
        {.Length = 0x0000, .BaseLow = 0x0000, .BaseMiddle = 0x00, .Access = 0b10010010, .Flags = 0b00000000, .BaseHigh = 0x00},    // kernel data
        {.Length = 0x0000, .BaseLow = 0x0000, .BaseMiddle = 0x00, .Access = 0b11111010, .Flags = 0b00100000, .BaseHigh = 0x00},    // user code
        {.Length = 0x0000, .BaseLow = 0x0000, .BaseMiddle = 0x00, .Access = 0b11110010, .Flags = 0b00000000, .BaseHigh = 0x00},    // user data
        {.Length = 0, .Low = 0, .Middle = 0, .Flags1 = 0b10001001, .Flags2 = 0b00000000, .High = 0, .Upper32 = 0, .Reserved = 0}}; // tss

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
