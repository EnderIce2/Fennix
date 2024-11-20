#include <time.hpp>

#include <memory.hpp>
#include <debug.h>
#include <io.h>

#if defined(a64)
#include "../Architecture/amd64/acpi.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../kernel.h"

namespace Time
{
    void time::Sleep(uint64_t Milliseconds)
    {
#if defined(a64) || defined(a32)
        uint64_t Target = mminq(&((HPET *)hpet)->MainCounterValue) + (Milliseconds * 1000000000000) / clk;
#ifdef DEBUG
        uint64_t Counter = mminq(&((HPET *)hpet)->MainCounterValue);
        while (Counter < Target)
        {
            Counter = mminq(&((HPET *)hpet)->MainCounterValue);
            CPU::Pause();
        }
#else
        while (mminq(&((HPET *)hpet)->MainCounterValue) < Target)
            CPU::Pause();
#endif
#elif defined(aa64)
#endif
    }

    uint64_t time::GetCounter()
    {
#if defined(a64) || defined(a32)
        return mminq(&((HPET *)hpet)->MainCounterValue);
#elif defined(aa64)
#endif
    }

    uint64_t time::CalculateTarget(uint64_t Milliseconds)
    {
#if defined(a64) || defined(a32)
        return mminq(&((HPET *)hpet)->MainCounterValue) + (Milliseconds * 1000000000000) / clk;
#elif defined(aa64)
#endif
    }

    time::time(void *_acpi)
    {
        if (_acpi)
        {
#if defined(a64)
            this->acpi = _acpi;
            ACPI::ACPI *acpi = (ACPI::ACPI *)this->acpi;
            if (acpi->HPET)
            {
                Memory::Virtual().Map((void *)acpi->HPET->Address.Address,
                                      (void *)acpi->HPET->Address.Address,
                                      Memory::PTFlag::RW | Memory::PTFlag::PCD);
                this->hpet = (void *)acpi->HPET->Address.Address;
                HPET *hpet = (HPET *)this->hpet;
                trace("%s timer is at address %016p", acpi->HPET->Header.OEMID, (void *)acpi->HPET->Address.Address);
                clk = hpet->GeneralCapabilities >> 32;
                mmoutq(&hpet->GeneralConfiguration, 0);
                mmoutq(&hpet->MainCounterValue, 0);
                mmoutq(&hpet->GeneralConfiguration, 1);
            }
            else
            {
                // For now, we need HPET.
                error("HPET not found");
                KPrint("\eFF2200HPET not found");
                CPU::Stop();
            }
#elif defined(a32)
#elif defined(aa64)
#endif
        }
    }

    time::~time()
    {
        debug("Destructor called");
    }
}
