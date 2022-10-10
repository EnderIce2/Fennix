#include <power.hpp>

#include <memory.hpp>
#include "../kernel.h"
#include <debug.h>

#if defined(__amd64__)
#include "../arch/amd64/acpi.hpp"

namespace Power
{
    void Power::Reboot()
    {
    }

    void Power::Shutdown()
    {
    }

    Power::Power()
    {
        this->acpi = new ACPI::ACPI(bInfo);
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