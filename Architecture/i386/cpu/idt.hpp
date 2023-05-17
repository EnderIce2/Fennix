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
    typedef enum _InterruptGateType
    {
        TASK = 0b101,
        INT_16BIT = 0b110,
        TRAP_16BIT = 0b111,
        INT_32BIT = 0b1110,
        TRAP_32BIT = 0b1111,
    } InterruptGateType;

    typedef enum _InterruptRingType
    {
        RING0 = 0b0,
        RING1 = 0b1,
        RING2 = 0b10,
        RING3 = 0b11,
    } InterruptRingType;

    typedef struct _InterruptDescriptorTableEntry
    {
        uint32_t LowOffset : 16;
        uint32_t SegmentSelector : 16;
        uint32_t Reserved0 : 5;
        uint32_t Flags : 4;
        uint32_t Reserved1 : 1;
        uint32_t Ring : 2;
        uint32_t Present : 1;
        uint32_t HighOffset : 16;
    } __packed InterruptDescriptorTableEntry;

    typedef struct _InterruptDescriptorTableDescriptor
    {
        uint16_t Length;
        InterruptDescriptorTableEntry *Entries;
    } __packed InterruptDescriptorTableDescriptor;

    void SetEntry(uint8_t Index,
                  void (*Base)(),
                  InterruptGateType Gate,
                  InterruptRingType Ring,
                  bool Present,
                  uint16_t SegmentSelector);

    void Init(int Core);
}

#endif // !__FENNIX_KERNEL_IDT_H__
