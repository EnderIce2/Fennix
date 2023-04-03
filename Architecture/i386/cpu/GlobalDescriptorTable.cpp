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
