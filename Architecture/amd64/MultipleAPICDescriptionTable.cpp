#include "acpi.hpp"

#include <memory.hpp>
#include <debug.h>

namespace ACPI
{
    MADT::MADT(ACPI::MADTHeader *madt)
    {
        LAPICAddress = (LAPIC *)(uintptr_t)madt->LocalControllerAddress;
        for (uint8_t *ptr = (uint8_t *)(madt->Entries);
             (uintptr_t)(ptr) < (uintptr_t)(madt) + madt->Header.Length;
             ptr += *(ptr + 1))
        {
            switch (*(ptr))
            {
            case 0:
            {
                if (ptr[4] & 1)
                {
                    lapic.push_back((LocalAPIC *)ptr);
                    trace("Local APIC %#llx (APIC %#llx) found.", lapic.back()->ACPIProcessorId, lapic.back()->APICId);
                    CPUCores++;
                }
                break;
            }
            case 1:
            {
                ioapic.push_back((MADTIOApic *)ptr);
                trace("I/O APIC %#llx (Address %#llx) found.", ioapic.back()->APICID, ioapic.back()->Address);
                Memory::Virtual().Map((void *)(uintptr_t)ioapic.back()->Address, (void *)(uintptr_t)ioapic.back()->Address, Memory::PTFlag::RW | Memory::PTFlag::PCD); // Make sure that the address is mapped.
                break;
            }
            case 2:
            {
                iso.push_back((MADTIso *)ptr);
                trace("ISO (IRQ:%#llx, BUS:%#llx, GSI:%#llx, %s/%s) found.",
                      iso.back()->IRQSource, iso.back()->BuSSource, iso.back()->GSI,
                      iso.back()->Flags & 0x00000004 ? "Active High" : "Active Low",
                      iso.back()->Flags & 0x00000100 ? "Edge Triggered" : "Level Triggered");
                break;
            }
            case 4:
            {
                nmi.push_back((MADTNmi *)ptr);
                trace("NMI %#llx (lint:%#llx) found.", nmi.back()->processor, nmi.back()->lint);
                break;
            }
            case 5:
            {
                LAPICAddress = (LAPIC *)ptr;
                trace("APIC found at %#llx", LAPICAddress);
                break;
            }
            }
            Memory::Virtual().Map((void *)LAPICAddress, (void *)LAPICAddress, Memory::PTFlag::RW | Memory::PTFlag::PCD); // I should map more than one page?
        }
        trace("Total CPU cores: %d", CPUCores);
    }

    MADT::~MADT()
    {
    }
}
