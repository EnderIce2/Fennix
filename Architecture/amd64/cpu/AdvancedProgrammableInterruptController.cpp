#include "apic.hpp"

#include <memory.hpp>
#include <uart.hpp>
#include <cpu.hpp>
#include <smp.hpp>
#include <io.h>

#include "../../../kernel.h"
#include "../acpi.hpp"

namespace APIC
{
    enum IOAPICRegisters
    {
        GetIOAPICVersion = 0x1
    };

    enum IOAPICFlags
    {
        ActiveHighLow = 2,
        EdgeLevel = 8
    };

    struct IOAPICVersion
    {
        uint8_t Version;
        uint8_t Reserved;
        uint8_t MaximumRedirectionEntry;
        uint8_t Reserved2;
    };

    // headache
    // https://www.amd.com/system/files/TechDocs/24593.pdf
    // https://www.naic.edu/~phil/software/intel/318148.pdf

    uint32_t APIC::Read(uint32_t Register)
    {
        debug("APIC::Read(%#lx) [x2=%d]", Register, x2APICSupported ? 1 : 0);
        if (x2APICSupported)
        {
            if (Register != APIC_ICRHI)
                return CPU::x64::rdmsr((Register >> 4) + 0x800);
            else
                return CPU::x64::rdmsr(0x30 + 0x800);
        }
        else
            return *((volatile uint32_t *)((uintptr_t)((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress + Register));
    }

    void APIC::Write(uint32_t Register, uint32_t Value)
    {
        if (Register != APIC_EOI &&
            Register != APIC_TDCR &&
            Register != APIC_TIMER &&
            Register != APIC_TICR)
            debug("APIC::Write(%#lx, %#lx) [x2=%d]", Register, Value, x2APICSupported ? 1 : 0);
        if (x2APICSupported)
        {
            if (Register != APIC_ICRHI)
                CPU::x64::wrmsr((Register >> 4) + 0x800, Value);
            else
                CPU::x64::wrmsr(CPU::x64::MSR_X2APIC_ICR, Value);
        }
        else
            *((volatile uint32_t *)(((uintptr_t)((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress) + Register)) = Value;
    }

    void APIC::IOWrite(uint64_t Base, uint32_t Register, uint32_t Value)
    {
        debug("APIC::IOWrite(%#lx, %#lx, %#lx)", Base, Register, Value);
        *((volatile uint32_t *)(((uintptr_t)Base))) = Register;
        *((volatile uint32_t *)(((uintptr_t)Base + 16))) = Value;
    }

    uint32_t APIC::IORead(uint64_t Base, uint32_t Register)
    {
        debug("APIC::IORead(%#lx, %#lx)", Base, Register);
        *((volatile uint32_t *)(((uintptr_t)Base))) = Register;
        return *((volatile uint32_t *)(((uintptr_t)Base + 16)));
    }

    void APIC::EOI() { this->Write(APIC_EOI, 0); }

    void APIC::RedirectIRQs(int CPU)
    {
        debug("Redirecting IRQs...");
        for (int i = 0; i < 16; i++)
            this->RedirectIRQ(CPU, i, 1);
        debug("Redirecting IRQs completed.");
    }

    void APIC::IPI(uint8_t CPU, uint32_t InterruptNumber)
    {
        if (x2APICSupported)
        {
            CPU::x64::wrmsr(CPU::x64::MSR_X2APIC_ICR, ((uint64_t)CPU) << 32 | InterruptNumber);
        }
        else
        {
            InterruptNumber = (1 << 14) | InterruptNumber;
            this->Write(APIC_ICRHI, (CPU << 24));
            this->Write(APIC_ICRLO, InterruptNumber);
        }
    }

    uint32_t APIC::IOGetMaxRedirect(uint32_t APICID)
    {
        uint32_t TableAddress = (this->IORead((((ACPI::MADT *)PowerManager->GetMADT())->ioapic[APICID]->Address), GetIOAPICVersion));
        return ((IOAPICVersion *)&TableAddress)->MaximumRedirectionEntry;
    }

    void APIC::RawRedirectIRQ(uint8_t Vector, uint32_t GSI, uint16_t Flags, int CPU, int Status)
    {
        uint64_t Value = Vector;

        int64_t IOAPICTarget = -1;
        for (uint64_t i = 0; ((ACPI::MADT *)PowerManager->GetMADT())->ioapic[i] != 0; i++)
            if (((ACPI::MADT *)PowerManager->GetMADT())->ioapic[i]->GSIBase <= GSI)
                if (((ACPI::MADT *)PowerManager->GetMADT())->ioapic[i]->GSIBase + IOGetMaxRedirect(i) > GSI)
                {
                    IOAPICTarget = i;
                    break;
                }

        if (IOAPICTarget == -1)
        {
            error("No ISO table found for I/O APIC");
            return;
        }

        if (Flags & ActiveHighLow)
            Value |= (1 << 13);

        if (Flags & EdgeLevel)
            Value |= (1 << 15);

        if (!Status)
            Value |= (1 << 16);

        // Value |= (((uintptr_t)GetCPU(CPU)->Data->LAPIC.APICId) << 56);
        Value |= (((uintptr_t)0) << 56);
        uint32_t IORegister = (GSI - ((ACPI::MADT *)PowerManager->GetMADT())->ioapic[IOAPICTarget]->GSIBase) * 2 + 16;

        this->IOWrite(((ACPI::MADT *)PowerManager->GetMADT())->ioapic[IOAPICTarget]->Address, IORegister, (uint32_t)Value);
        this->IOWrite(((ACPI::MADT *)PowerManager->GetMADT())->ioapic[IOAPICTarget]->Address, IORegister + 1, (uint32_t)(Value >> 32));
    }

    void APIC::RedirectIRQ(int CPU, uint8_t IRQ, int Status)
    {
        for (uint64_t i = 0; i < ((ACPI::MADT *)PowerManager->GetMADT())->iso.size(); i++)
            if (((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->IRQSource == IRQ)
            {
                debug("[ISO %d] Mapping to source IRQ%#d GSI:%#lx on CPU %d",
                      i, ((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->IRQSource, ((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->GSI, CPU);

                this->RawRedirectIRQ(((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->IRQSource + 0x20, ((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->GSI, ((ACPI::MADT *)PowerManager->GetMADT())->iso[i]->Flags, CPU, Status);
                return;
            }
        debug("Mapping IRQ%d on CPU %d", IRQ, CPU);
        this->RawRedirectIRQ(IRQ + 0x20, IRQ, 0, CPU, Status);
    }

    APIC::APIC(int Core)
    {
        uint32_t rcx;
        CPU::x64::cpuid(1, 0, 0, &rcx, 0);
        if (rcx & CPU::x64::CPUID_FEAT_RCX_x2APIC)
        {
            // this->x2APICSupported = true;
            warn("x2APIC not supported yet.");
            // CPU::x64::wrmsr(CPU::x64::MSR_APIC_BASE, (CPU::x64::rdmsr(CPU::x64::MSR_APIC_BASE) | (1 << 11)) & ~(1 << 10));
            CPU::x64::wrmsr(CPU::x64::MSR_APIC_BASE, CPU::x64::rdmsr(CPU::x64::MSR_APIC_BASE) | (1 << 11));
        }
        else
        {
            CPU::x64::wrmsr(CPU::x64::MSR_APIC_BASE, CPU::x64::rdmsr(CPU::x64::MSR_APIC_BASE) | (1 << 11));
        }
        trace("APIC Address: %#lx", CPU::x64::rdmsr(CPU::x64::MSR_APIC_BASE));

        this->Write(APIC_TPR, 0x0);
        this->Write(APIC_SVR, this->Read(APIC_SVR) | 0x100); // 0x1FF or 0x100 ? on https://wiki.osdev.org/APIC is 0x100

        if (!this->x2APICSupported)
        {
            this->Write(APIC_DFR, 0xF0000000);
            this->Write(APIC_LDR, this->Read(APIC_ID));
        }

        ACPI::MADT *madt = (ACPI::MADT *)PowerManager->GetMADT();

        for (size_t i = 0; i < madt->nmi.size(); i++)
        {
            if (madt->nmi[i]->processor != 0xFF && Core != madt->nmi[i]->processor)
                return;

            uint32_t nmi = 0x402;
            if (madt->nmi[i]->flags & 2)
                nmi |= 1 << 13;
            if (madt->nmi[i]->flags & 8)
                nmi |= 1 << 15;
            if (madt->nmi[i]->lint == 0)
                this->Write(0x350, nmi);
            else if (madt->nmi[i]->lint == 1)
                this->Write(0x360, nmi);
        }
    }

    APIC::~APIC()
    {
    }

    void Timer::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        // fixme("APIC IRQ0 INTERRUPT RECEIVED ON CPU %d", CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE));
        // UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write('\n');
        // UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write('H');
        // UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM1).Write('\n');
    }

    void Timer::OneShot(uint32_t Vector, uint64_t Miliseconds)
    {
        this->lapic->Write(APIC_TDCR, 0x03);
        this->lapic->Write(APIC_TIMER, (APIC_ONESHOT | Vector));
        this->lapic->Write(APIC_TICR, (TicksIn10ms / 10) * Miliseconds);
    }

    Timer::Timer(APIC *apic) : Interrupts::Handler(CPU::x64::IRQ0)
    {
        trace("Initializing APIC timer on CPU %d", GetCurrentCPU()->ID);
        this->lapic = apic;
        this->lapic->Write(APIC_TDCR, 0x3);

        int Count = 10000; /* µs */
        int Ticks = 1193180 / (Count / 100);

        int IOIn = inb(0x61);
        IOIn = (IOIn & 0xFD) | 1;
        outb(0x61, IOIn);
        outb(0x43, 178);
        outb(0x40, Ticks & 0xff);
        inb(0x60);
        outb(0x40, Ticks >> 8);

        this->lapic->Write(APIC_TICR, 0xFFFFFFFF);

        IOIn = inb(0x61);
        IOIn = (IOIn & 0xFC);
        outb(0x61, IOIn);
        IOIn |= 1;
        outb(0x61, IOIn);
        uint32_t Loop = 0;
        while ((inb(0x61) & 0x20) != 0)
            ++Loop;

        this->lapic->Write(APIC_TIMER, 0x10000);

        outb(0x43, 0x28);
        outb(0x40, 0x0);

        outb(0x21, 0xFF);
        outb(0xA1, 0xFF);

        TicksIn10ms = 0xFFFFFFFF - this->lapic->Read(APIC_TCCR);

        this->lapic->Write(APIC_TIMER, (long)CPU::x64::IRQ0 | (long)APIC_PERIODIC);
        this->lapic->Write(APIC_TDCR, 0x3);
        this->lapic->Write(APIC_TICR, TicksIn10ms / 10);
        trace("APIC Timer (CPU %d): %d ticks in 10ms", GetCurrentCPU()->ID, TicksIn10ms / 10);
        KPrint("APIC Timer: \e8888FF%ld\eCCCCCC ticks in 10ms", TicksIn10ms / 10);
    }

    Timer::~Timer()
    {
    }
}
