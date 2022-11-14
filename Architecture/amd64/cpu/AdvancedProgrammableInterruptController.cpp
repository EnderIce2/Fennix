#include "apic.hpp"

#include <memory.hpp>
#include <uart.hpp>
#include <lock.hpp>
#include <cpu.hpp>
#include <smp.hpp>
#include <io.h>

#include "../../../kernel.h"
#include "../acpi.hpp"

NewLock(APICLock);

using namespace CPU::x64;

/*
In constructor ‘APIC::APIC::APIC(int)’:
warning: left shift count >= width of type
|         APICBaseAddress = BaseStruct.ApicBaseLo << 12u | BaseStruct.ApicBaseHi << 32u;
|                                                          ~~~~~~~~~~~~~~~~~~~~~~^~~~~~
*/
#pragma GCC diagnostic ignored "-Wshift-count-overflow"

namespace APIC
{
    // headache
    // https://www.amd.com/system/files/TechDocs/24593.pdf
    // https://www.naic.edu/~phil/software/intel/318148.pdf

    uint32_t APIC::Read(uint32_t Register)
    {
        if (Register != APIC_ICRLO &&
            Register != APIC_ICRHI &&
            Register != APIC_ID)
            debug("APIC::Read(%#lx) [x2=%d]", Register, x2APICSupported ? 1 : 0);
        if (x2APICSupported)
        {
            if (Register != APIC_ICRHI)
                return rdmsr((Register >> 4) + 0x800);
            else
                return rdmsr(0x30 + 0x800);
        }
        else
        {
            CPU::MemBar::Barrier();
            uint32_t ret = *((volatile uint32_t *)((uintptr_t)APICBaseAddress + Register));
            CPU::MemBar::Barrier();
            return ret;
        }
    }

    void APIC::Write(uint32_t Register, uint32_t Value)
    {
        if (Register != APIC_EOI &&
            Register != APIC_TDCR &&
            Register != APIC_TIMER &&
            Register != APIC_TICR &&
            Register != APIC_ICRLO &&
            Register != APIC_ICRHI)
            debug("APIC::Write(%#lx, %#lx) [x2=%d]", Register, Value, x2APICSupported ? 1 : 0);
        if (x2APICSupported)
        {
            if (Register != APIC_ICRHI)
                wrmsr((Register >> 4) + 0x800, Value);
            else
                wrmsr(MSR_X2APIC_ICR, Value);
        }
        else
        {
            CPU::MemBar::Barrier();
            *((volatile uint32_t *)(((uintptr_t)APICBaseAddress) + Register)) = Value;
            CPU::MemBar::Barrier();
        }
    }

    void APIC::IOWrite(uint64_t Base, uint32_t Register, uint32_t Value)
    {
        debug("APIC::IOWrite(%#lx, %#lx, %#lx)", Base, Register, Value);
        CPU::MemBar::Barrier();
        *((volatile uint32_t *)(((uintptr_t)Base))) = Register;
        CPU::MemBar::Barrier();
        *((volatile uint32_t *)(((uintptr_t)Base + 16))) = Value;
        CPU::MemBar::Barrier();
    }

    uint32_t APIC::IORead(uint64_t Base, uint32_t Register)
    {
        debug("APIC::IORead(%#lx, %#lx)", Base, Register);
        CPU::MemBar::Barrier();
        *((volatile uint32_t *)(((uintptr_t)Base))) = Register;
        CPU::MemBar::Barrier();
        uint32_t ret = *((volatile uint32_t *)(((uintptr_t)Base + 16)));
        CPU::MemBar::Barrier();
        return ret;
    }

    void APIC::EOI() { this->Write(APIC_EOI, 0); }

    void APIC::WaitForIPI()
    {
        InterruptCommandRegisterLow icr = {.raw = 0};
        do
        {
            icr.raw = this->Read(APIC_ICRLO);
        } while (icr.DeliveryStatus != Idle);
    }

    void APIC::IPI(uint8_t CPU, InterruptCommandRegisterLow icr)
    {
        SmartCriticalSection(APICLock);
        if (x2APICSupported)
        {
            fixme("Not implemented for x2APIC");
            // wrmsr(MSR_X2APIC_ICR, ((uint64_t)CPU) << 32);
        }
        else
        {
            this->Write(APIC_ICRHI, (CPU << 24));
            this->Write(APIC_ICRLO, icr.raw);
            this->WaitForIPI();
        }
    }

    void APIC::SendInitIPI(uint8_t CPU)
    {
        SmartCriticalSection(APICLock);
        if (x2APICSupported)
        {
            fixme("Not implemented for x2APIC");
            // wrmsr(MSR_X2APIC_ICR, ((uint64_t)CPU) << 32);
        }
        else
        {
            InterruptCommandRegisterLow icr = {.raw = 0};
            icr.DeliveryMode = INIT;
            icr.Level = Assert;
            this->Write(APIC_ICRHI, (CPU << 24));
            this->Write(APIC_ICRLO, icr.raw);
            this->WaitForIPI();
        }
    }

    void APIC::SendStartupIPI(uint8_t CPU, uint64_t StartupAddress)
    {
        SmartCriticalSection(APICLock);
        if (x2APICSupported)
        {
            warn("Not tested for x2APIC");
            wrmsr(MSR_X2APIC_ICR, ((uint64_t)CPU) << 32 | StartupAddress);
        }
        else
        {
            InterruptCommandRegisterLow icr = {.raw = 0};
            icr.Vector = StartupAddress >> 12;
            icr.DeliveryMode = Startup;
            icr.Level = Assert;
            this->Write(APIC_ICRHI, (CPU << 24));
            this->Write(APIC_ICRLO, icr.raw);
            this->WaitForIPI();
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

        // TODO: IOAPICRedirectEntry Entry = {.raw = 0};

        if (Flags & ActiveHighLow)
            Value |= (1 << 13);

        if (Flags & EdgeLevel)
            Value |= (1 << 15);

        if (!Status)
            Value |= (1 << 16);

        Value |= (((uintptr_t)CPU) << 56);
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

    void APIC::RedirectIRQs(int CPU)
    {
        SmartCriticalSection(APICLock);
        debug("Redirecting IRQs...");
        for (int i = 0; i < 16; i++)
            this->RedirectIRQ(CPU, i, 1);
        debug("Redirecting IRQs completed.");
    }

    APIC::APIC(int Core)
    {
        SmartCriticalSection(APICLock);
        APIC_BASE BaseStruct = {.raw = rdmsr(MSR_APIC_BASE)};
        APICBaseAddress = BaseStruct.ApicBaseLo << 12u | BaseStruct.ApicBaseHi << 32u;
        trace("APIC Address: %#lx", APICBaseAddress);

        uint32_t rcx;
        cpuid(1, 0, 0, &rcx, 0);
        if (rcx & CPUID_FEAT_RCX_x2APIC)
        {
            // this->x2APICSupported = true;
            warn("x2APIC not supported yet.");
            // wrmsr(MSR_APIC_BASE, (rdmsr(MSR_APIC_BASE) | (1 << 11)) & ~(1 << 10));
            BaseStruct.EN = 1;
            wrmsr(MSR_APIC_BASE, BaseStruct.raw);
        }
        else
        {
            BaseStruct.EN = 1;
            wrmsr(MSR_APIC_BASE, BaseStruct.raw);
        }

        this->Write(APIC_TPR, 0x0);
        // this->Write(APIC_SVR, this->Read(APIC_SVR) | 0x100); // 0x1FF or 0x100 ? on https://wiki.osdev.org/APIC is 0x100

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
                this->Write(APIC_LINT0, nmi);
            else if (madt->nmi[i]->lint == 1)
                this->Write(APIC_LINT1, nmi);
        }

        // Setup the spurrious interrupt vector
        Spurious Spurious = {.raw = this->Read(APIC_SVR)};
        Spurious.Vector = IRQ223; // TODO: Should I map the IRQ to something?
        Spurious.Software = 1;
        this->Write(APIC_SVR, Spurious.raw);

        static int once = 0;
        if (!once++)
        {
            // Disable PIT
            outb(0x43, 0x28);
            outb(0x40, 0x0);

            // Disable PIC
            outb(0x21, 0xFF);
            outb(0xA1, 0xFF);
        }
    }

    APIC::~APIC()
    {
    }

    void Timer::OnInterruptReceived(TrapFrame *Frame) { debug("APIC IRQ0 INTERRUPT RECEIVED ON CPU %d", GetCurrentCPU()->ID); }

    void Timer::OneShot(uint32_t Vector, uint64_t Miliseconds)
    {
        SmartCriticalSection(APICLock);
        LVTTimer timer = {.raw = 0};
        timer.Vector = Vector;
        timer.TimerMode = 0;
        if (strcmp(CPU::Hypervisor(), x86_CPUID_VENDOR_TCG) != 0)
            this->lapic->Write(APIC_TDCR, DivideBy128);
        else
            this->lapic->Write(APIC_TDCR, DivideBy16);
        this->lapic->Write(APIC_TICR, Ticks * Miliseconds);
        this->lapic->Write(APIC_TIMER, timer.raw);
    }

    Timer::Timer(APIC *apic) : Interrupts::Handler(IRQ0)
    {
        SmartCriticalSection(APICLock);
        this->lapic = apic;
        LVTTimerDivide Divider = DivideBy16;

        trace("Initializing APIC timer on CPU %d", GetCurrentCPU()->ID);

        this->lapic->Write(APIC_TDCR, Divider);
        this->lapic->Write(APIC_TICR, 0xFFFFFFFF);

        TimeManager->Sleep(10);

        // Mask the timer
        this->lapic->Write(APIC_TIMER, 0x10000 /* LVTTimer.Mask flag */);
        Ticks = 0xFFFFFFFF - this->lapic->Read(APIC_TCCR);

        // Config for IRQ0 timer
        LVTTimer timer = {.raw = 0};
        timer.Vector = IRQ0;
        timer.Mask = Unmasked;
        timer.TimerMode = LVTTimerMode::OneShot;

        // Initialize APIC timer
        this->lapic->Write(APIC_TDCR, Divider);
        this->lapic->Write(APIC_TICR, Ticks);
        this->lapic->Write(APIC_TIMER, timer.raw);
        trace("%d APIC Timer %d ticks in.", GetCurrentCPU()->ID, Ticks);
        KPrint("APIC Timer: \e8888FF%ld\eCCCCCC ticks.", Ticks);
    }

    Timer::~Timer()
    {
    }
}
