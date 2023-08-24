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

#ifndef __FENNIX_KERNEL_SEGMENT_DESCRIPTORS_H__
#define __FENNIX_KERNEL_SEGMENT_DESCRIPTORS_H__

#include <types.h>

/**
 * Manual: AMD Architecture Programmer's Manual Volume 2: System Programming
 * Subsection: 4.8.3 System Descriptors
 * Table: 4-6
 *
 * @note Reserved values are not listed in the table.
 */
enum GateType
{
	LDT_64BIT = 0b0010,
	AVAILABLE_64BIT_TSS = 0b1001,
	BUSY_64BIT_TSS = 0b1011,
	CALL_GATE_64BIT = 0b1100,
	INTERRUPT_GATE_64BIT = 0b1110,
	TRAP_GATE_64BIT = 0b1111,
};

enum PrivilegeLevelType
{
	RING0 = 0b0,
	RING1 = 0b1,
	RING2 = 0b10,
	RING3 = 0b11,
};

enum InterruptStackTableType
{
	IST0 = 0b0,
	IST1 = 0b1,
	IST2 = 0b10,
	IST3 = 0b11,
	IST4 = 0b100,
	IST5 = 0b101,
	IST6 = 0b110,
};

struct InterruptGate
{
	/* +0 */
	uint64_t TargetOffsetLow : 16;
	uint64_t TargetSelector : 16;
	/* +4 */
	uint64_t InterruptStackTable : 3;
	uint64_t Reserved0 : 5;
	uint64_t Type : 4;
	uint64_t Zero : 1;
	uint64_t DescriptorPrivilegeLevel : 2;
	uint64_t Present : 1;
	uint64_t TargetOffsetMiddle : 16;
	/* +8 */
	uint64_t TargetOffsetHigh : 32;
	/* +12 */
	uint64_t Reserved1 : 32;
} __packed;

typedef InterruptGate TrapGate;

struct CallGate
{
	/* +0 */
	uint64_t TargetOffsetLow : 16;
	uint64_t TargetSelector : 16;
	/* +4 */
	uint64_t Reserved0 : 8;
	uint64_t Type : 4;
	uint64_t Zero0 : 1;
	uint64_t DescriptorPrivilegeLevel : 2;
	uint64_t Present : 1;
	uint64_t TargetOffsetMiddle : 16;
	/* +8 */
	uint64_t TargetOffsetHigh : 32;
	/* +12 */
	uint64_t Reserved1 : 8;
	uint64_t Zero1 : 5;
	uint64_t Reserved2 : 19;
} __packed;

struct SystemSegmentDescriptor
{
	/* +0 */
	uint64_t SegmentLimitLow : 16;
	uint64_t BaseAddressLow : 16;
	/* +4 */
	uint64_t BaseAddressMiddle : 8;
	uint64_t Type : 4;
	uint64_t Zero0 : 1;
	uint64_t DescriptorPrivilegeLevel : 2;
	uint64_t Present : 1;
	uint64_t SegmentLimitMiddle : 4;
	uint64_t Available : 1;
	uint64_t Reserved0 : 2;
	uint64_t Granularity : 1;
	uint64_t BaseAddressHigh : 8;
	/* +8 */
	uint64_t BaseAddressHigher : 32;
	/* +12 */
	uint64_t Reserved1 : 8;
	uint64_t Zero1 : 5;
	uint64_t Reserved2 : 19;
} __packed;

struct CodeSegmentDescriptor
{
	/* +0 */
	uint64_t SegmentLimitLow : 16;
	uint64_t BaseAddressLow : 16;
	/* +4 */
	uint64_t BaseAddressHigh : 8;
	uint64_t Accessed : 1;
	uint64_t Readable : 1;
	uint64_t Conforming : 1;
	uint64_t Executable : 1;
	uint64_t Type : 1;
	uint64_t DescriptorPrivilegeLevel : 2;
	uint64_t Present : 1;
	uint64_t SegmentLimitHigh : 4;
	uint64_t Available : 1;
	uint64_t Long : 1;
	uint64_t Default : 1;
	uint64_t Granularity : 1;
	uint64_t BaseAddressHigher : 8;
} __packed;

struct DataSegmentDescriptor
{
	/* +0 */
	uint64_t SegmentLimitLow : 16;
	uint64_t BaseAddressLow : 16;
	/* +4 */
	uint64_t BaseAddressHigh : 8;
	uint64_t Accessed : 1;
	uint64_t Writable : 1;
	uint64_t ExpandDown : 1;
	uint64_t Executable : 1;
	uint64_t Type : 1;
	uint64_t DescriptorPrivilegeLevel : 2;
	uint64_t Present : 1;
	uint64_t SegmentLimitHigh : 4;
	uint64_t Available : 1;
	uint64_t Reserved : 1;
	uint64_t Default : 1;
	uint64_t Granularity : 1;
	uint64_t BaseAddressHigher : 8;
} __packed;

#endif // !__FENNIX_KERNEL_SEGMENT_DESCRIPTORS_H__
