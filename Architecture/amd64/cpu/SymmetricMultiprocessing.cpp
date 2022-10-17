#include <smp.hpp>

#include <interrupts.hpp>
#include <memory.hpp>
#include <cpu.hpp>

#include "../../../kernel.h"
#if defined(__amd64__)
#include "../Architecture/amd64/acpi.hpp"
#include "../Architecture/amd64/cpu/apic.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

extern "C" uint64_t _trampoline_start, _trampoline_end;

#define TRAMPOLINE_START 0x2000

enum SMPTrampolineAddress
{
    PAGE_TABLE = 0x500,
    START_ADDR = 0x520,
    STACK = 0x570,
    GDT = 0x580,
    IDT = 0x590,
    CORE = 0x600
};

volatile bool CPUEnabled = false;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static __attribute__((aligned(PAGE_SIZE))) CPUData CPUs[MAX_CPU] = {0};

CPUData *GetCPU(uint64_t id) { return &CPUs[id]; }

CPUData *GetCurrentCPU()
{
    uint64_t ret = 0;
#if defined(__amd64__)
    ret = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

    if (!CPUs[ret].IsActive)
    {
        error("CPU %d is not active!", ret);
        return &CPUs[0];
    }

    if (CPUs[ret].Checksum != CPU_DATA_CHECKSUM)
    {
        error("CPU %d data is corrupted!", ret);
        return &CPUs[0];
    }
    return &CPUs[ret];
}

extern "C" void StartCPU()
{
    CPU::Interrupts(CPU::Disable);
    CPU::InitializeFeatures();
    uintptr_t CoreID = CORE;
    CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, (int)*reinterpret_cast<int *>(CoreID));
    uint64_t CPU_ID = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);

    // Initialize GDT and IDT
    Interrupts::Initialize(CPU_ID);
    Interrupts::Enable(CPU_ID);
    Interrupts::InitializeTimer(CPU_ID);

    CPU::Interrupts(CPU::Enable);
    KPrint("CPU %d is online", CPU_ID);
    CPUEnabled = true;
    CPU::Stop(); // Stop and surpress interrupts.
}

namespace SMP
{
    void Initialize(void *madt)
    {
        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) == 0)
        {
            KPrint("VirtualBox detected, disabling SMP");
            return;
        }
        for (uint16_t i = 0; i < ((ACPI::MADT *)madt)->CPUCores + 1; i++)
        {
            debug("Initializing CPU %d", i);
            if ((((APIC::APIC *)Interrupts::apic[0])->Read(APIC::APIC::APIC_ID) >> 24) != ((ACPI::MADT *)madt)->lapic[i]->ACPIProcessorId)
            {
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC::APIC_ICRLO, 0x500);

                Memory::Virtual().Map(0x0, 0x0, Memory::PTFlag::RW | Memory::PTFlag::US);

                uint64_t TrampolineLength = (uintptr_t)&_trampoline_end - (uintptr_t)&_trampoline_start;
                for (uint64_t i = 0; i < (TrampolineLength / PAGE_SIZE) + 2; i++)
                    Memory::Virtual().Map((void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), (void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                memcpy((void *)TRAMPOLINE_START, &_trampoline_start, TrampolineLength);

                POKE(volatile uint64_t, PAGE_TABLE) = CPU::x64::readcr3().raw;
                POKE(volatile uint64_t, STACK) = (uint64_t)KernelAllocator.RequestPage();
                POKE(volatile uint64_t, CORE) = i;

                asm volatile("sgdt [0x580]\n"
                             "sidt [0x590]\n");

                POKE(volatile uint64_t, START_ADDR) = (uintptr_t)&StartCPU;

                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC::APIC_ICRLO, 0x600 | ((uint32_t)TRAMPOLINE_START / PAGE_SIZE));

                while (!CPUEnabled)
                    ;

                trace("CPU %d loaded.", ((ACPI::MADT *)madt)->lapic[i]->APICId);
                CPUEnabled = false;
            }
            else
                KPrint("CPU %d is the BSP", ((ACPI::MADT *)madt)->lapic[i]->APICId);
        }
    }
}
