#include <time.hpp>

#include <memory.hpp>
#include <debug.h>
#include <io.h>

#if defined(__amd64__)
#include "../Architecture/amd64/acpi.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

namespace Time
{
    void time::Sleep(uint64_t Milliseconds)
    {
#if defined(__amd64__) || defined(__i386__)
        uintptr_t Target = mminq(&((HPET *)hpet)->MainCounterValue) + (Milliseconds * 1000000000000) / clk;
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
#elif defined(__aarch64__)
#endif
    }

    time::time(void *_acpi)
    {
        if (_acpi)
        {
#if defined(__amd64__)
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
                trace("HPET not found");
            }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
        }
    }

    time::~time()
    {
    }
}
