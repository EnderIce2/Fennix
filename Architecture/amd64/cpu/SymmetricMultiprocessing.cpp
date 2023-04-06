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

#include <smp.hpp>

#include <ints.hpp>
#include <memory.hpp>
#include <assert.h>
#include <cpu.hpp>

#include "../../../kernel.h"
#include "../acpi.hpp"
#include "apic.hpp"

extern "C" uint64_t _trampoline_start, _trampoline_end;

enum SMPTrampolineAddress
{
    PAGE_TABLE = 0x500,
    START_ADDR = 0x520,
    STACK = 0x570,
    GDT = 0x580,
    IDT = 0x590,
    CORE = 0x600,
    TRAMPOLINE_START = 0x2000
};

volatile bool CPUEnabled = false;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static __aligned(PAGE_SIZE) CPUData CPUs[MAX_CPU] = {0};

SafeFunction CPUData *GetCPU(long id) { return &CPUs[id]; }

SafeFunction CPUData *GetCurrentCPU()
{
    if (unlikely(!Interrupts::apic[0]))
        return &CPUs[0]; /* No APIC means we are on the BSP. */

    int CoreID = ((APIC::APIC *)Interrupts::apic[0])->Read(APIC::APIC_ID) >> 24;

    if (unlikely((&CPUs[CoreID])->IsActive != true))
    {
        error("CPU %d is not active!", CoreID);
        assert((&CPUs[0])->IsActive == true); /* We can't continue without the BSP. */
        return &CPUs[0];
    }

    assert((&CPUs[CoreID])->Checksum == CPU_DATA_CHECKSUM); /* This should never happen. */
    return &CPUs[CoreID];
}

extern "C" void StartCPU()
{
    CPU::Interrupts(CPU::Disable);
    int CoreID = (int)*reinterpret_cast<int *>(CORE);
    CPU::InitializeFeatures(CoreID);
    // Initialize GDT and IDT
    Interrupts::Initialize(CoreID);
    Interrupts::Enable(CoreID);
    Interrupts::InitializeTimer(CoreID);

    CPU::Interrupts(CPU::Enable);
    KPrint("\e058C19CPU \e8888FF%d \e058C19is online", CoreID);
    CPUEnabled = true;
    CPU::Halt(true);
}

namespace SMP
{
    int CPUCores = 0;

    void Initialize(void *madt)
    {
        int Cores = ((ACPI::MADT *)madt)->CPUCores + 1;

        if (Config.Cores > ((ACPI::MADT *)madt)->CPUCores + 1)
            KPrint("More cores requested than available. Using %d cores", ((ACPI::MADT *)madt)->CPUCores + 1);
        else if (Config.Cores != 0)
            Cores = Config.Cores;

        CPUCores = Cores;

        for (int i = 0; i < Cores; i++)
        {
            debug("Initializing CPU %d", i);
            if ((((APIC::APIC *)Interrupts::apic[0])->Read(APIC::APIC_ID) >> 24) != ((ACPI::MADT *)madt)->lapic[i]->ACPIProcessorId)
            {
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC_ICRLO, 0x500);

                Memory::Virtual(KernelPageTable).Map(0x0, 0x0, Memory::PTFlag::RW | Memory::PTFlag::US);

                uint64_t TrampolineLength = (uintptr_t)&_trampoline_end - (uintptr_t)&_trampoline_start;
                Memory::Virtual(KernelPageTable).Map((void *)TRAMPOLINE_START, (void *)TRAMPOLINE_START, TrampolineLength, Memory::PTFlag::RW | Memory::PTFlag::US);

                memcpy((void *)TRAMPOLINE_START, &_trampoline_start, TrampolineLength);

                VPOKE(uint64_t, PAGE_TABLE) = (uint64_t)KernelPageTable;
                VPOKE(uint64_t, STACK) = (uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;
                VPOKE(int, CORE) = i;

                asmv("sgdt [0x580]\n"
                     "sidt [0x590]\n");

                VPOKE(uint64_t, START_ADDR) = (uintptr_t)&StartCPU;

                ((APIC::APIC *)Interrupts::apic[0])->SendInitIPI(((ACPI::MADT *)madt)->lapic[i]->APICId);
                ((APIC::APIC *)Interrupts::apic[0])->SendStartupIPI(((ACPI::MADT *)madt)->lapic[i]->APICId, TRAMPOLINE_START);

                while (!CPUEnabled)
                    CPU::Pause();

                trace("CPU %d loaded.", ((ACPI::MADT *)madt)->lapic[i]->APICId);
                KernelAllocator.FreePages((void *)*reinterpret_cast<long *>(STACK), TO_PAGES(STACK_SIZE));
                CPUEnabled = false;
            }
            else
                KPrint("\e058C19CPU \e8888FF%d \e058C19is the BSP", ((ACPI::MADT *)madt)->lapic[i]->APICId);
        }
    }
}
