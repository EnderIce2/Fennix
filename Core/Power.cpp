#include <power.hpp>

#include <memory.hpp>
#include <debug.h>

#include "../kernel.h"

#if defined(__amd64__)
#include <io.h>

#include "../Architecture/amd64/acpi.hpp"

namespace Power
{
    void Power::Reboot()
    {
        if (((ACPI::ACPI *)this->acpi)->FADT)
            if (((ACPI::DSDT *)this->dsdt)->ACPIShutdownSupported)
                ((ACPI::DSDT *)this->dsdt)->Reboot();

        uint8_t val = 0x02;
        while (val & 0x02)
            val = inb(0x64);
        outb(0x64, 0xFE);

        warn("Executing the second attempt to reboot...");

        // second attempt to reboot
        // https://wiki.osdev.org/Reboot
        uint8_t temp;
        asmv("cli");
        do
        {
            temp = inb(0x64);
            if (((temp) & (1 << (0))) != 0)
                inb(0x60);
        } while (((temp) & (1 << (1))) != 0);
        outb(0x64, 0xFE);

        CPU::Stop();
    }

    void Power::Shutdown()
    {
        if (((ACPI::ACPI *)this->acpi)->FADT)
            if (((ACPI::DSDT *)this->dsdt)->ACPIShutdownSupported)
                ((ACPI::DSDT *)this->dsdt)->Shutdown();

        outl(0xB004, 0x2000); // for qemu
        outl(0x604, 0x2000);  // if qemu not working, bochs and older versions of qemu
        outl(0x4004, 0x3400); // virtual box
        CPU::Stop();
    }

    Power::Power()
    {
        this->acpi = new ACPI::ACPI(bInfo);
        if (((ACPI::ACPI *)this->acpi)->FADT)
            this->dsdt = new ACPI::DSDT((ACPI::ACPI *)acpi);
        this->madt = new ACPI::MADT(((ACPI::ACPI *)acpi)->MADT);
        trace("Power manager initialized");
    }

    Power::~Power()
    {
    }
}

#elif defined(__i386__)

namespace Power
{
    void Power::Reboot()
    {
        warn("Reboot not implemented for i386");
    }

    void Power::Shutdown()
    {
        warn("Shutdown not implemented for i386");
    }

    Power::Power()
    {
        error("Power not implemented for i386");
    }

    Power::~Power()
    {
    }
}

#elif defined(__aarch64__)

namespace Power
{
    void Power::Reboot()
    {
        warn("Reboot not implemented for aarch64");
    }

    void Power::Shutdown()
    {
        warn("Shutdown not implemented for aarch64");
    }

    Power::Power()
    {
        error("Power not implemented for aarch64");
    }

    Power::~Power()
    {
    }
}

#endif
