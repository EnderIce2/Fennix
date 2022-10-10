#include <interrupts.hpp>

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#include "../Architecture/amd64/cpu/idt.hpp"
#include "../Architecture/amd64/acpi.hpp"
#include "../Architecture/amd64/cpu/apic.hpp"
#elif defined(__i386__)
#include "../Architecture/i686/cpu/gdt.hpp"
#include "../Architecture/i686/cpu/idt.hpp"
#elif defined(__aarch64__)
#endif

#include "../kernel.h"

namespace Interrupts
{
#if defined(__amd64__)
    APIC::APIC *apic = nullptr;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

    void Initialize()
    {
#if defined(__amd64__)
        GlobalDescriptorTable::Init(0);
        InterruptDescriptorTable::Init(0);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    void Enable()
    {
#if defined(__amd64__)
        if (((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress != nullptr)
            apic = new APIC::APIC;
        else
        {
            error("LAPIC not found");
            // PIC
        }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }
}
