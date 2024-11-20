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
    /** @brief The GDT Access Table
     * @details For more information, see https://wiki.osdev.org/Global_Descriptor_Table
     */
    union GlobalDescriptorTableAccess
    {
        struct
        {
            /** @brief Access bit.
             * @note The CPU sets this bit to 1 when the segment is accessed.
             */
            uint64_t A : 1;

            /** @brief Readable bit for code segments, writable bit for data segments.
             * @details For code segments, this bit must be 1 for the segment to be readable.
             * @details For data segments, this bit must be 1 for the segment to be writable.
             */
            uint64_t RW : 1;

            /** @brief Direction bit for data segments, conforming bit for code segments.
             * @details For data segments, this bit must be 1 for the segment to grow up (higher addresses).
             * @details For code segments, this bit must be 1 for code in the segment to be able to be executed from an equal or lower privilege level.
             */
            uint64_t DC : 1;

            /** @brief Executable bit.
             * @details This bit must be 1 for code-segment descriptors.
             * @details This bit must be 0 for data-segment and system descriptors.
             */
            uint64_t E : 1;

            /** @brief Descriptor type.
             * @details This bit must be 0 for system descriptors.
             * @details This bit must be 1 for code or data segment descriptor.
             */
            uint64_t S : 1;

            /** @brief Descriptor privilege level.
             * @details This field determines the privilege level of the segment.
             * @details 0 = kernel mode, 3 = user mode.
             */
            uint64_t DPL : 2;

            /** @brief Present bit.
             * @details This bit must be 1 for all valid descriptors.
             */
            uint64_t P : 1;
        } __packed;
        uint8_t Raw;
    };

    union GlobalDescriptorTableFlags
    {
        // TODO: Add more flags.
        struct
        {
            /** @brief Unknown. */
            uint64_t Unknown : 5;

            /** @brief Long mode.
             * @details If the long mode bit is clear, the segment is in 32-bit protected mode.
             * @details If the long mode bit is set, the segment is in 64-bit long mode.
             */
            uint64_t L : 1;
        } __packed;
        uint8_t Raw;
    };

    typedef struct _TaskStateSegmentEntry
    {
        /* LOW */
        uint16_t Length;
        uint16_t BaseLow;
        uint8_t BaseMiddle;
        GlobalDescriptorTableAccess Flags;
        uint8_t Granularity;
        uint8_t BaseHigh;
        /* HIGH */
        uint32_t BaseUpper;
        uint32_t Reserved;
    } __packed TaskStateSegmentEntry;

    typedef struct _TaskStateSegment
    {
        uint32_t Reserved0 __aligned(16);
        uint64_t StackPointer[3];
        uint64_t Reserved1;
        uint64_t InterruptStackTable[7];
        uint64_t Reserved2;
        uint16_t Reserved3;
        uint16_t IOMapBaseAddressOffset;
    } __packed TaskStateSegment;

    typedef struct _GlobalDescriptorTableEntry
    {
        /** @brief Length */
        uint16_t Length;
        /** @brief Low Base */
        uint16_t BaseLow;
        /** @brief Middle Base */
        uint8_t BaseMiddle;
        /** @brief Access */
        GlobalDescriptorTableAccess Access;
        /** @brief Flags */
        GlobalDescriptorTableFlags Flags;
        /** @brief High Base */
        uint8_t BaseHigh;
    } __packed GlobalDescriptorTableEntry;

    typedef struct _GlobalDescriptorTableEntries
    {
        GlobalDescriptorTableEntry Null;
        GlobalDescriptorTableEntry Code;
        GlobalDescriptorTableEntry Data;
        GlobalDescriptorTableEntry UserData;
        GlobalDescriptorTableEntry UserCode;
        TaskStateSegmentEntry TaskStateSegment;
    } __packed GlobalDescriptorTableEntries;

    typedef struct _GlobalDescriptorTableDescriptor
    {
        /** @brief GDT entries length */
        uint16_t Length;
        /** @brief GDT entries address */
        GlobalDescriptorTableEntries *Entries;
    } __packed GlobalDescriptorTableDescriptor;

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
