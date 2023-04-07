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

#ifndef __FENNIX_KERNEL_IDT_H__
#define __FENNIX_KERNEL_IDT_H__

#include <types.h>

namespace InterruptDescriptorTable
{
    typedef enum _InterruptDescriptorTableFlags
    {
        FlagGate_TASK = 0b101,
        FlagGate_16BIT_INT = 0b110,
        FlagGate_16BIT_TRAP = 0b111,
        FlagGate_32BIT_INT = 0b1110,
        FlagGate_32BIT_TRAP = 0b1111,
        FlagGate_RING0 = 0b0,
        FlagGate_RING1 = 0b1,
        FlagGate_RING2 = 0b10,
        FlagGate_RING3 = 0b11,
        FlagGate_PRESENT = 0b1, // Not sure if this is correct.
    } InterruptDescriptorTableFlags;

    typedef struct _InterruptDescriptorTableEntry
    {
        uint64_t BaseLow : 16;
        uint64_t SegmentSelector : 16;
        uint64_t InterruptStackTable : 3;
        uint64_t Reserved1 : 5;
        InterruptDescriptorTableFlags Flags : 4;
        uint64_t Reserved2 : 1;
        uint64_t Ring : 2;
        uint64_t Present : 1;
        uint64_t BaseHigh : 48;
        uint64_t Reserved3 : 32;
    } __packed InterruptDescriptorTableEntry;

    typedef struct _InterruptDescriptorTableDescriptor
    {
        uint16_t Length;
        InterruptDescriptorTableEntry *Entries;
    } __packed InterruptDescriptorTableDescriptor;

    void SetEntry(uint8_t Index, void (*Base)(), InterruptDescriptorTableFlags Attribute, uint8_t InterruptStackTable, InterruptDescriptorTableFlags Ring, uint16_t SegmentSelector);
    void Init(int Core);
}

#endif // !__FENNIX_KERNEL_IDT_H__
