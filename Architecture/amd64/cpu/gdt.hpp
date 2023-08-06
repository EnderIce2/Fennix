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

#ifndef __FENNIX_KERNEL_GDT_H__
#define __FENNIX_KERNEL_GDT_H__

#include <types.h>

namespace GlobalDescriptorTable
{
	struct TaskStateSegmentEntry
	{
		/* LOW */
		uint16_t Limit;
		uint16_t BaseLow;
		uint8_t BaseMiddle;
		union GlobalDescriptorTableAccess
		{
			struct
			{
				/**
				 * Access bit.
				 *
				 * @note The CPU sets this bit to 1 when the segment
				 * is accessed.
				 */
				uint8_t A : 1;

				/**
				 * Readable bit for code segments, writable bit for data
				 * segments.
				 *
				 * Code Segment:
				 * This bit must be 1 for the segment to be readable.
				 *
				 * Data Segment:
				 * This bit must be 1 for the segment to be writable.
				 */
				uint8_t RW : 1;

				/**
				 * Direction bit for data segments, conforming bit for
				 * code segments.
				 *
				 * Code Segment:
				 * This bit must be 1 for code in the segment to be
				 * able to be executed from an equal or lower privilege
				 * level.
				 *
				 * Data Segment:
				 * This bit must be 1 for the segment to grow up (higher
				 * addresses).
				 */
				uint8_t DC : 1;

				/**
				 * Executable bit.
				 *
				 * This bit must be 1 for code-segment descriptors.
				 *
				 * This bit must be 0 for data-segment and system
				 * descriptors.
				 */
				uint8_t E : 1;

				/**
				 * Descriptor type.
				 *
				 * This bit must be 0 for system descriptors.
				 *
				 * This bit must be 1 for code or data segment
				 * descriptor.
				 */
				uint8_t S : 1;

				/**
				 * Descriptor privilege level.
				 *
				 * This field determines the privilege level of the
				 * segment.
				 *
				 * 0 = kernel mode
				 * 3 = user mode
				 */
				uint8_t DPL : 2;

				/**
				 * Present bit.
				 *
				 * This bit must be 1 for all valid descriptors.
				 */
				uint8_t P : 1;
			} __packed;
			uint8_t Raw : 8;
		} Access;
		uint8_t Granularity;
		uint8_t BaseHigh;
		/* HIGH */
		uint32_t BaseUpper;
		uint32_t Reserved;
	} __packed;

	struct TaskStateSegment
	{
		uint32_t Reserved0 __aligned(16);
		uint64_t StackPointer[3];
		uint64_t Reserved1;
		uint64_t InterruptStackTable[7];
		uint64_t Reserved2;
		uint16_t Reserved3;
		uint16_t IOMapBaseAddressOffset;
	} __packed;

	struct GlobalDescriptorTableEntry
	{
		/**
		 * Limit 0:15
		 */
		uint16_t Limit0 : 16;

		/**
		 * Low Base 0:15
		 */
		uint16_t BaseLow : 16;

		/**
		 * Middle Base 16:23
		 */
		uint8_t BaseMiddle : 8;

		/**
		 * Access
		 */
		union GlobalDescriptorTableAccess
		{
			struct
			{
				/**
				 * Access bit.
				 *
				 * @note The CPU sets this bit to 1 when the segment
				 * is accessed.
				 */
				uint8_t A : 1;

				/**
				 * Readable bit for code segments, writable bit for
				 * data segments.
				 *
				 * Code Segment:
				 * This bit must be 1 for the segment to be readable.
				 *
				 * Data Segment:
				 * This bit must be 1 for the segment to be writable.
				 */
				uint8_t RW : 1;

				/**
				 * Direction bit for data segments, conforming bit for
				 * code segments.
				 *
				 * Code Segment:
				 * This bit must be 1 for code in the segment to be able
				 * to be executed from an equal or lower privilege level.
				 *
				 * Data Segment:
				 * This bit must be 1 for the segment to grow up (higher
				 * addresses).
				 */
				uint8_t DC : 1;

				/**
				 * Executable bit.
				 *
				 * This bit must be 1 for code-segment descriptors.
				 *
				 * This bit must be 0 for data-segment and
				 * system descriptors.
				 */
				uint8_t E : 1;

				/**
				 * Descriptor type.
				 *
				 * This bit must be 0 for system descriptors.
				 *
				 * This bit must be 1 for code or data segment
				 * descriptor.
				 */
				uint8_t S : 1;

				/**
				 * Descriptor privilege level.
				 *
				 * This field determines the privilege level of the
				 * segment.
				 *
				 * 0 = kernel mode
				 * 3 = user mode
				 */
				uint8_t DPL : 2;

				/**
				 * Present bit.
				 *
				 * This bit must be 1 for all valid descriptors.
				 */
				uint8_t P : 1;
			} __packed;
			uint8_t Raw : 8;
		} Access;

		// /** Limit 16:19 */

		// uint16_t Limit1 : 4;

		/**
		 * Flags
		 */
		union GlobalDescriptorTableFlags
		{
			struct
			{
				/* FIXME: Without this, the kernel crashes. */
				uint8_t Reserved : 4;

				/**
				 * Available bit.
				 *
				 * This bit is available for use by system software.
				 */
				uint8_t AVL : 1;

				/**
				 * Long mode.
				 *
				 * If the long mode bit is clear, the segment is in
				 * 32-bit protected mode.
				 *
				 * If the long mode bit is set, the segment is in
				 * 64-bit long mode.
				 */
				uint8_t L : 1;

				/**
				 * Size flag.
				 *
				 * If the size bit is clear, the segment is in
				 * 16-bit protected mode.
				 *
				 * If the size bit is set, the segment is in
				 * 32-bit protected mode.
				 */
				uint8_t DB : 1;

				/**
				 * Granularity bit.
				 *
				 * If the granularity bit is clear, the segment limit
				 * is in 1 B blocks.
				 *
				 * If the granularity bit is set, the segment limit is
				 * in 4 KiB blocks.
				 */
				uint8_t G : 1;
			} __packed;
			uint8_t Raw : 8;
		} Flags;

		/**
		 * High Base 24:31
		 */
		uint8_t BaseHigh : 8;
	} __packed;

	struct GlobalDescriptorTableEntries
	{
		GlobalDescriptorTableEntry Null;
		GlobalDescriptorTableEntry Code;
		GlobalDescriptorTableEntry Data;
		GlobalDescriptorTableEntry UserData;
		GlobalDescriptorTableEntry UserCode;
		TaskStateSegmentEntry TaskStateSegment;
	} __packed;

	struct GlobalDescriptorTableDescriptor
	{
		/**
		 * GDT entries length
		 */
		uint16_t Length;

		/**
		 * GDT entries address
		 */
		GlobalDescriptorTableEntries *Entries;
	} __packed;

	extern void *CPUStackPointer[];
	extern TaskStateSegment tss[];
	void Init(int Core);
	void SetKernelStack(void *Stack);
	void *GetKernelStack();
}

#define GDT_KERNEL_CODE offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, Code)
#define GDT_KERNEL_DATA offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, Data)
#define GDT_USER_CODE (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, UserCode) | 3)
#define GDT_USER_DATA (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, UserData) | 3)
#define GDT_TSS (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, TaskStateSegment) | 3)

#endif // !__FENNIX_KERNEL_GDT_H__
