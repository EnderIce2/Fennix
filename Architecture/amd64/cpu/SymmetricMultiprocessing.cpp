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
};

volatile bool CPUEnabled = false;

static __attribute__((aligned(PAGE_SIZE))) CPUData CPUs[MAX_CPU] = {0};

CPUData *GetCPU(uint64_t id) { return &CPUs[id]; }

CPUData *GetCurrentCPU()
{
    uint64_t ret = 0;
#if defined(__amd64__)
    ret = ((APIC::APIC *)Interrupts::apic)->Read(APIC::APIC::APIC_ID) >> 24;
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
    uint64_t CPU_ID;
#if defined(__amd64__)
    // Enable CPU features
    {
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        uint32_t rax, rbx, rcx, rdx;
        CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
        if (rdx & CPU::x64::CPUID_FEAT_RDX_SSE)
        {
            cr0.EM = 0;
            cr0.MP = 1;
            cr4.OSFXSR = 1;
            cr4.OSXMMEXCPT = 1;
        }

        // Enable cpu cache but... how to use it?
        cr0.NW = 0;
        cr0.CD = 0;

        CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
        if (rdx & CPU::x64::CPUID_FEAT_RDX_UMIP)
        {
            fixme("Not going to enable UMIP.");
            // cr4.UMIP = 1;
        }
        if (rdx & CPU::x64::CPUID_FEAT_RDX_SMEP)
            cr4.SMEP = 1;
        if (rdx & CPU::x64::CPUID_FEAT_RDX_SMAP)
            cr4.SMAP = 1;
        CPU::x64::writecr0(cr0);
        CPU::x64::writecr4(cr4);
        CPU::x64::wrmsr(CPU::x64::MSR_CR_PAT, 0x6 | (0x0 << 8) | (0x1 << 16));
    }

    // Enable APIC
    {
        CPU::x64::wrmsr(CPU::x64::MSR_APIC_BASE, (CPU::x64::rdmsr(CPU::x64::MSR_APIC_BASE) | 0x800) & ~(1 << 10));
        ((APIC::APIC *)Interrupts::apic)->Write(APIC::APIC::APIC_SVR, ((APIC::APIC *)Interrupts::apic)->Read(APIC::APIC::APIC_SVR) | 0x1FF);
    }

    // Set CPU_ID variable using APIC
    CPU_ID = ((APIC::APIC *)Interrupts::apic)->Read(APIC::APIC::APIC_ID) >> 24;

    // Initialize GDT and IDT
    Interrupts::Initialize(CPU_ID);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    CPU::Interrupts(CPU::Enable);
    KPrint("CPU %d is online", CPU_ID);
    CPUEnabled = true;
    CPU::Stop();
}

namespace SMP
{
    void Initialize(void *madt)
    {
#if defined(__amd64__)
        for (uint8_t i = 0; i < ((ACPI::MADT *)madt)->CPUCores; i++)
            if ((((APIC::APIC *)Interrupts::apic)->Read(APIC::APIC::APIC_ID) >> 24) != ((ACPI::MADT *)madt)->lapic[i]->ACPIProcessorId)
            {
                ((APIC::APIC *)Interrupts::apic)->Write(APIC::APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic)->Write(APIC::APIC::APIC_ICRLO, 0x500);

                Memory::Virtual().Map(0x0, 0x0, Memory::PTFlag::RW | Memory::PTFlag::US);

                uint64_t TrampolineLength = (uintptr_t)&_trampoline_end - (uintptr_t)&_trampoline_start;
                for (uint64_t i = 0; i < (TrampolineLength / PAGE_SIZE) + 2; i++)
                    Memory::Virtual().Map((void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), (void *)(TRAMPOLINE_START + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);

                memcpy((void *)TRAMPOLINE_START, &_trampoline_start, TrampolineLength);

                POKE(volatile uint64_t, PAGE_TABLE) = CPU::x64::readcr3().raw;
                POKE(volatile uint64_t, STACK) = (uint64_t)KernelAllocator.RequestPage();

                asm volatile("sgdt [0x580]\n"
                             "sidt [0x590]\n");

                POKE(volatile uint64_t, START_ADDR) = (uintptr_t)&StartCPU;

                ((APIC::APIC *)Interrupts::apic)->Write(APIC::APIC::APIC_ICRHI, (((ACPI::MADT *)madt)->lapic[i]->APICId << 24));
                ((APIC::APIC *)Interrupts::apic)->Write(APIC::APIC::APIC_ICRLO, 0x600 | ((uint32_t)TRAMPOLINE_START / PAGE_SIZE));

                while (!CPUEnabled)
                    ;

                trace("CPU %d loaded.", ((ACPI::MADT *)madt)->lapic[i]->APICId);
                CPUEnabled = false;
            }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }
}
