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
static __attribute__((aligned(PAGE_SIZE))) CPUData CPUs[MAX_CPU] = {0};

CPUData *GetCPU(long id) { return &CPUs[id]; }
CPUData *GetCurrentCPU() { return (CPUData *)CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE); }

extern "C" void StartCPU()
{
    CPU::Interrupts(CPU::Disable);
    CPU::InitializeFeatures();
    uint64_t CoreID = (int)*reinterpret_cast<int *>(CORE);
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
        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_VIRTUALBOX) == 0)
        {
            KPrint("VirtualBox detected, disabling SMP");
            return;
        }

        int Cores = ((ACPI::MADT *)madt)->CPUCores + 1;

        if (Config.Cores > ((ACPI::MADT *)madt)->CPUCores + 1)
            KPrint("More cores requested than available. Using %d cores", ((ACPI::MADT *)madt)->CPUCores + 1);
        else if (Config.Cores != 0)
            Cores = Config.Cores;

        CPUCores = Cores;

        for (uint16_t i = 0; i < Cores; i++)
        {
            debug("Initializing CPU %d", i);
            if ((((APIC::APIC *)Interrupts::apic[0])->Read(APIC::APIC_ID) >> 24) != ((ACPI::MADT *)madt)->lapic[i]->ACPIProcessorId)
            {
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic[0])->Write(APIC::APIC_ICRLO, 0x500);

                Memory::Virtual().Map(0x0, 0x0, Memory::PTFlag::RW | Memory::PTFlag::US);

                uint64_t TrampolineLength = (uintptr_t)&_trampoline_end - (uintptr_t)&_trampoline_start;
                for (uint64_t i = 0; i < (TrampolineLength / PAGE_SIZE) + 2; i++)
                    Memory::Virtual().Map((void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), (void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                memcpy((void *)TRAMPOLINE_START, &_trampoline_start, TrampolineLength);

                POKE(volatile uint64_t, PAGE_TABLE) = CPU::x64::readcr3().raw;
                POKE(volatile uint64_t, STACK) = (uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE;
                POKE(volatile uint64_t, CORE) = i;

                asmv("sgdt [0x580]\n"
                     "sidt [0x590]\n");

                POKE(volatile uint64_t, START_ADDR) = (uintptr_t)&StartCPU;

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
