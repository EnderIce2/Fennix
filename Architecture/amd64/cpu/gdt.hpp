#ifndef __FENNIX_KERNEL_GDT_H__
#define __FENNIX_KERNEL_GDT_H__

#include <types.h>

namespace GlobalDescriptorTable
{
    /* https://github.com/nanobyte-dev/nanobyte_os/blob/master/src/kernel/arch/i686/gdt.c */
    /* https://wiki.osdev.org/Global_Descriptor_Table */
    typedef enum
    {
        CODE_READABLE = 0b00000010,
        DATA_WRITEABLE = 0b00000010,

        CODE_CONFORMING = 0b00000100,
        DATA_DIRECTION_NORMAL = 0b00000000,
        DATA_DIRECTION_DOWN = 0b00000100,

        DATA_SEGMENT = 0b00010000,
        CODE_SEGMENT = 0b00011000,

        DESCRIPTOR_TSS = 0b00000000,

        RING0 = 0b00000000,
        RING1 = 0b00100000,
        RING2 = 0b01000000,
        RING3 = 0b01100000,

        PRESENT = 0b10000000
    } AccessFlags;

    typedef enum
    {
        _64BITS = 0b00100000,
        _32BITS = 0b01000000,
        _16BITS = 0b00000000,

        GRANULARITY_1B = 0b00000000,
        GRANULARITY_4K = 0b10000000
    } GDTFlags;

    typedef struct _TaskStateSegmentEntry
    {
        uint16_t Length;
        uint16_t Low;
        uint8_t Middle;
        uint8_t Flags1;
        uint8_t Flags2;
        uint8_t High;
        uint32_t Upper32;
        uint32_t Reserved;
    } __attribute__((packed)) TaskStateSegmentEntry;

    typedef struct _TaskStateSegment
    {
        uint32_t Reserved0;
        uint64_t StackPointer[3];
        uint64_t Reserved1;
        uint64_t InterruptStackTable[7];
        uint16_t Reserved2;
        uint16_t IOMapBaseAddressOffset;
    } __attribute__((packed)) TaskStateSegment;

    typedef struct _GlobalDescriptorTableEntry
    {
        /** @brief Length [Bits 0-15] */
        uint16_t Length;
        /** @brief Low Base [Bits 0-15] */
        uint16_t BaseLow;
        /** @brief Middle Base [Bits 0-23] */
        uint8_t BaseMiddle;
        /** @brief Access */
        uint8_t Access;
        /** @brief Flags [Bits 16-19] */
        uint8_t Flags;
        /** @brief High Base [Bits 24-31] */
        uint8_t BaseHigh;
    } __attribute__((packed)) GlobalDescriptorTableEntry;

    typedef struct _GlobalDescriptorTableEntries
    {
        GlobalDescriptorTableEntry Null;
        GlobalDescriptorTableEntry Code;
        GlobalDescriptorTableEntry Data;
        GlobalDescriptorTableEntry UserCode;
        GlobalDescriptorTableEntry UserData;
        TaskStateSegmentEntry TaskStateSegment;
    } __attribute__((packed)) GlobalDescriptorTableEntries;

    typedef struct _GlobalDescriptorTableDescriptor
    {
        /** @brief GDT entries length */
        uint16_t Length;
        /** @brief GDT entries address */
        GlobalDescriptorTableEntries *Entries;
    } __attribute__((packed)) GlobalDescriptorTableDescriptor;

    void Init(int Core);
}

#define GDT_KERNEL_CODE offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, Code)
#define GDT_KERNEL_DATA offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, Data)
#define GDT_USER_CODE (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, UserCode) | 3)
#define GDT_USER_DATA (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, UserData) | 3)
#define GDT_TSS (offsetof(GlobalDescriptorTable::GlobalDescriptorTableEntries, TaskStateSegment) | 3)

#endif // !__FENNIX_KERNEL_GDT_H__
