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

#include "apic.hpp"

#include <memory.hpp>
#include <acpi.hpp>
#include <uart.hpp>
#include <lock.hpp>
#include <cpu.hpp>
#include <smp.hpp>
#include <io.h>

#include "../../../kernel.h"

NewLock(APICLock);

using namespace CPU::x64;
using namespace CPU::x86;

/*
In constructor 'APIC::APIC::APIC(int)':
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
#ifdef DEBUG
		if (Register != APIC_ICRLO &&
			Register != APIC_ICRHI &&
			Register != APIC_ID)
			debug("APIC::Read(%#lx) [x2=%d]",
				  Register, x2APICSupported ? 1 : 0);
#endif
		if (unlikely(x2APICSupported))
			assert(!"x2APIC is not supported");

		CPU::MemBar::Barrier();
		uint32_t ret = *((volatile uint32_t *)((uintptr_t)APICBaseAddress + Register));
		CPU::MemBar::Barrier();
		return ret;
	}

	void APIC::Write(uint32_t Register, uint32_t Value)
	{
#ifdef DEBUG
		if (Register != APIC_EOI &&
			Register != APIC_TDCR &&
			Register != APIC_TIMER &&
			Register != APIC_TICR &&
			Register != APIC_ICRLO &&
			Register != APIC_ICRHI)
			debug("APIC::Write(%#lx, %#lx) [x2=%d]",
				  Register, Value, x2APICSupported ? 1 : 0);
#endif
		if (unlikely(x2APICSupported))
			assert(!"x2APIC is not supported");

		CPU::MemBar::Barrier();
		*((volatile uint32_t *)(((uintptr_t)APICBaseAddress) + Register)) = Value;
		CPU::MemBar::Barrier();
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

	void APIC::EOI()
	{
		Memory::SwapPT swap =
			Memory::SwapPT(KernelPageTable, thisPageTable);
		if (this->x2APICSupported)
			wrmsr(MSR_X2APIC_EOI, 0);
		else
			this->Write(APIC_EOI, 0);
	}

	void APIC::WaitForIPI()
	{
		if (this->x2APICSupported)
		{
			ErrorStatusRegister esr{};
			esr.raw = uint32_t(rdmsr(MSR_X2APIC_ESR));
			UNUSED(esr);
			/* FIXME: Not sure if this is required or
				how to implement it. */
		}
		else
		{
			InterruptCommandRegister icr{};
			do
			{
				icr.split.Low = this->Read(APIC_ICRLO);
				CPU::Pause();
			} while (icr.DS != Idle);
		}
	}

	void APIC::ICR(InterruptCommandRegister icr)
	{
		SmartCriticalSection(APICLock);
		if (x2APICSupported)
		{
			assert(icr.MT != LowestPriority);
			assert(icr.MT != DeliveryMode);
			assert(icr.MT != ExtINT);
			wrmsr(MSR_X2APIC_ICR, icr.raw);
			this->WaitForIPI();
		}
		else
		{
			this->Write(APIC_ICRHI, icr.split.High);
			this->Write(APIC_ICRLO, icr.split.Low);
			this->WaitForIPI();
		}
	}

	void APIC::SendInitIPI(int CPU)
	{
		SmartCriticalSection(APICLock);
		InterruptCommandRegister icr{};

		if (x2APICSupported)
		{
			icr.x2.MT = INIT;
			icr.x2.L = Assert;
			icr.x2.DES = uint8_t(CPU);

			wrmsr(MSR_X2APIC_ICR, icr.raw);
			this->WaitForIPI();
		}
		else
		{
			icr.MT = INIT;
			icr.L = Assert;
			icr.DES = uint8_t(CPU);

			this->Write(APIC_ICRHI, icr.split.High);
			this->Write(APIC_ICRLO, icr.split.Low);
			this->WaitForIPI();
		}
	}

	void APIC::SendStartupIPI(int CPU, uint64_t StartupAddress)
	{
		SmartCriticalSection(APICLock);
		InterruptCommandRegister icr{};

		if (x2APICSupported)
		{
			icr.x2.VEC = s_cst(uint8_t, StartupAddress >> 12);
			icr.x2.MT = Startup;
			icr.x2.L = Assert;
			icr.x2.DES = uint8_t(CPU);

			wrmsr(MSR_X2APIC_ICR, icr.raw);
			this->WaitForIPI();
		}
		else
		{
			icr.VEC = s_cst(uint8_t, StartupAddress >> 12);
			icr.MT = Startup;
			icr.L = Assert;
			icr.DES = uint8_t(CPU);

			this->Write(APIC_ICRHI, icr.split.High);
			this->Write(APIC_ICRLO, icr.split.Low);
			this->WaitForIPI();
		}
	}

	uint32_t APIC::IOGetMaxRedirect(uint32_t APICID)
	{
		ACPI::MADT::MADTIOApic *ioapic = ((ACPI::MADT *)PowerManager->GetMADT())->ioapic[APICID];
		uint32_t TableAddress = (this->IORead(ioapic->Address, GetIOAPICVersion));
		IOAPICVersion ver = {.raw = TableAddress};
		return ver.MLE + 1;
	}

	void APIC::RawRedirectIRQ(uint8_t Vector, uint32_t GSI, uint16_t Flags, uint8_t CPU, int Status)
	{
		int64_t IOAPICTarget = -1;
		ACPI::MADT *madt = (ACPI::MADT *)PowerManager->GetMADT();
		for (size_t i = 0; i < madt->ioapic.size(); i++)
		{
			if (madt->ioapic[i]->GSIBase <= GSI)
			{
				if (madt->ioapic[i]->GSIBase + IOGetMaxRedirect(uint32_t(i)) > GSI)
				{
					IOAPICTarget = i;
					break;
				}
			}
		}

		if (IOAPICTarget == -1)
		{
			error("No ISO table found for I/O APIC");
			return;
		}

		IOAPICRedirectEntry Entry{};
		Entry.VEC = Vector;
		Entry.DES = CPU;

		if (Flags & ActiveHighLow)
			Entry.IPP = 1;

		if (Flags & EdgeLevel)
			Entry.TGM = 1;

		if (!Status)
			Entry.M = 1;

		uint32_t IORegister = (GSI - madt->ioapic[IOAPICTarget]->GSIBase) * 2 + 16;
		this->IOWrite(madt->ioapic[IOAPICTarget]->Address,
					  IORegister, Entry.split.Low);
		this->IOWrite(madt->ioapic[IOAPICTarget]->Address,
					  IORegister + 1, Entry.split.High);
	}

	void APIC::RedirectIRQ(uint8_t CPU, uint8_t IRQ, int Status)
	{
		ACPI::MADT *madt = (ACPI::MADT *)PowerManager->GetMADT();
		for (uint64_t i = 0; i < madt->iso.size(); i++)
			if (madt->iso[i]->IRQSource == IRQ)
			{
				debug("[ISO %d] Mapping to source IRQ%#d GSI:%#lx on CPU %d",
					  i, madt->iso[i]->IRQSource, madt->iso[i]->GSI, CPU);

				this->RawRedirectIRQ(madt->iso[i]->IRQSource + 0x20,
									 madt->iso[i]->GSI,
									 madt->iso[i]->Flags,
									 CPU, Status);
				return;
			}
		debug("Mapping IRQ%d on CPU %d", IRQ, CPU);
		this->RawRedirectIRQ(IRQ + 0x20, IRQ, 0, CPU, Status);
	}

	void APIC::RedirectIRQs(uint8_t CPU)
	{
		SmartCriticalSection(APICLock);
		debug("Redirecting IRQs...");
		for (uint8_t i = 0; i < 16; i++)
			this->RedirectIRQ(CPU, i, 1);
		debug("Redirecting IRQs completed.");
	}

	APIC::APIC(int Core)
	{
		SmartCriticalSection(APICLock);
		APIC_BASE BaseStruct = {.raw = rdmsr(MSR_APIC_BASE)};
		uint64_t BaseLow = BaseStruct.ABALow;
		uint64_t BaseHigh = BaseStruct.ABAHigh;
		this->APICBaseAddress = BaseLow << 12u | BaseHigh << 32u;
		trace("APIC Address: %#lx", this->APICBaseAddress);
		Memory::Virtual().Map((void *)this->APICBaseAddress,
							  (void *)this->APICBaseAddress,
							  Memory::RW | Memory::PCD);

		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid;
			if (cpuid.ECX.x2APIC)
			{
				this->x2APICSupported = cpuid.ECX.x2APIC;
				debug("x2APIC is supported");
			}
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
			if (cpuid.ECX.x2APIC)
			{
				this->x2APICSupported = cpuid.ECX.x2APIC;
				debug("x2APIC is supported");
			}
		}

		BaseStruct.AE = 1;
		wrmsr(MSR_APIC_BASE, BaseStruct.raw);

		if (this->x2APICSupported)
		{
			BaseStruct.EXTD = 1;
			wrmsr(MSR_APIC_BASE, BaseStruct.raw);
		}

		if (!this->x2APICSupported)
		{
			this->Write(APIC_TPR, 0x0);
			this->Write(APIC_DFR, 0xF0000000);
			this->Write(APIC_LDR, this->Read(APIC_ID));
		}
		else
		{
			wrmsr(MSR_X2APIC_TPR, 0x0);
		}

		ACPI::MADT *madt = (ACPI::MADT *)PowerManager->GetMADT();

		for (size_t i = 0; i < madt->nmi.size(); i++)
		{
			if (madt->nmi[i]->processor != 0xFF &&
				Core != madt->nmi[i]->processor)
				break;

			uint32_t nmi = 0x402;
			if (madt->nmi[i]->flags & 2)
				nmi |= 1 << 13;
			if (madt->nmi[i]->flags & 8)
				nmi |= 1 << 15;
			if (madt->nmi[i]->lint == 0)
			{
				if (this->x2APICSupported)
					wrmsr(MSR_X2APIC_LVT_LINT0, nmi);
				else
					this->Write(APIC_LINT0, nmi);
			}
			else if (madt->nmi[i]->lint == 1)
			{
				if (this->x2APICSupported)
					wrmsr(MSR_X2APIC_LVT_LINT1, nmi);
				else
					this->Write(APIC_LINT1, nmi);
			}
		}

		/* Setup the spurious interrupt vector */
		Spurious svr{};
		if (this->x2APICSupported)
			svr.raw = uint32_t(rdmsr(MSR_X2APIC_SIVR));
		else
			svr.raw = this->Read(APIC_SVR);

		svr.VEC = IRQ223;
		svr.ASE = 1;
		if (this->x2APICSupported)
			wrmsr(MSR_X2APIC_SIVR, svr.raw);
		else
			this->Write(APIC_SVR, svr.raw);

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

	APIC::~APIC() {}

	void Timer::OnInterruptReceived(CPU::TrapFrame *) {}

	void Timer::OneShot(uint32_t Vector, uint64_t Miliseconds)
	{
		/* FIXME: Sometimes APIC stops firing when debugging, why? */
		LVTTimer timer{};
		timer.VEC = uint8_t(Vector);
		timer.TMM = LVTTimerMode::OneShot;

		LVTTimerDivide Divider = DivideBy8;

		SmartCriticalSection(APICLock);
		if (this->lapic->x2APIC)
		{
			// wrmsr(MSR_X2APIC_DIV_CONF, Divider); <- gpf on real hardware
			wrmsr(MSR_X2APIC_INIT_COUNT, uint32_t(Ticks * Miliseconds));
			wrmsr(MSR_X2APIC_LVT_TIMER, uint32_t(timer.raw));
		}
		else
		{
			this->lapic->Write(APIC_TDCR, Divider);
			this->lapic->Write(APIC_TICR, uint32_t(Ticks * Miliseconds));
			this->lapic->Write(APIC_TIMER, uint32_t(timer.raw));
		}
	}

	Timer::Timer(APIC *apic) : Interrupts::Handler(0) /* IRQ0 */
	{
		SmartCriticalSection(APICLock);
		this->lapic = apic;
		LVTTimerDivide Divider = DivideBy8;

		trace("Initializing APIC timer on CPU %d",
			  GetCurrentCPU()->ID);

		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_DIV_CONF, Divider);
			wrmsr(MSR_X2APIC_INIT_COUNT, 0xFFFFFFFF);
		}
		else
		{
			this->lapic->Write(APIC_TDCR, Divider);
			this->lapic->Write(APIC_TICR, 0xFFFFFFFF);
		}

		TimeManager->Sleep(1, Time::Units::Milliseconds);

		// Mask the timer
		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_LVT_TIMER, 0x10000 /* LVTTimer.Mask flag */);
			Ticks = 0xFFFFFFFF - rdmsr(MSR_X2APIC_CUR_COUNT);
		}
		else
		{
			this->lapic->Write(APIC_TIMER, 0x10000 /* LVTTimer.Mask flag */);
			Ticks = 0xFFFFFFFF - this->lapic->Read(APIC_TCCR);
		}

		// Config for IRQ0 timer
		LVTTimer timer{};
		timer.VEC = IRQ0;
		timer.M = Unmasked;
		timer.TMM = LVTTimerMode::OneShot;

		// Initialize APIC timer
		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_DIV_CONF, Divider);
			wrmsr(MSR_X2APIC_INIT_COUNT, Ticks);
			wrmsr(MSR_X2APIC_LVT_TIMER, timer.raw);
		}
		else
		{
			this->lapic->Write(APIC_TDCR, Divider);
			this->lapic->Write(APIC_TICR, uint32_t(Ticks));
			this->lapic->Write(APIC_TIMER, uint32_t(timer.raw));
		}

		trace("%d APIC Timer %d ticks in.",
			  GetCurrentCPU()->ID, Ticks);
		KPrint("APIC Timer: \e8888FF%ld\eCCCCCC ticks.", Ticks);
	}

	Timer::~Timer()
	{
	}
}
