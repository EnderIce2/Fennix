#ifndef __FENNIX_KERNEL_IDT_H__
#define __FENNIX_KERNEL_IDT_H__

#include <types.h>

namespace InterruptDescriptorTable
{
    typedef enum _InterruptDescriptorTableFlags
    {
        FlagGate_TASK = 0b0101,
        FlagGate_16BIT_INT = 0b0110,
        FlagGate_16BIT_TRAP = 0b0111,
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
    } __attribute__((packed)) InterruptDescriptorTableEntry;

    typedef struct _InterruptDescriptorTableDescriptor
    {
        uint16_t Length;
        InterruptDescriptorTableEntry *Entries;
    } __attribute__((packed)) InterruptDescriptorTableDescriptor;

    void SetEntry(uint8_t Index, void (*Base)(), InterruptDescriptorTableFlags Attribute, uint8_t InterruptStackTable, InterruptDescriptorTableFlags Ring, uint16_t SegmentSelector);
    void Init(int Core);
}

#endif // !__FENNIX_KERNEL_IDT_H__
