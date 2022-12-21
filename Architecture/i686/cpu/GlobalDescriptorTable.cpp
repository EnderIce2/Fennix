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

        // tss
        {}};

    GlobalDescriptorTableEntries GDTEntries[MAX_CPU];
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

    SafeFunction void Init(int Core)
    {
    }

    SafeFunction void SetKernelStack(void *Stack)
    {
    }
}
