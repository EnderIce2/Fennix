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

#include <cpu/apic.hpp>
#include <cpu/x86/msr.hpp>

#include <smp.hpp>

#include <entry.hpp>

using namespace CPU::x86;
using namespace std::chrono_literals;

namespace APIC
{
	void Timer::SetOneShot(std::chrono::nanoseconds ns, Interrupt::IRQLines Line)
	{
		uint64_t ticks = (ns.count() * TicksPerSecond) / 1'000'000'000ULL;

		LVTTimer lvt{};
		lvt.VEC = uint8_t(Line + (int)CPU::x86::IRQ0);
		lvt.M = Unmasked;
		lvt.TMM = OneShot;

		if (lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_LVT_TIMER, lvt.raw);
			wrmsr(MSR_X2APIC_INIT_COUNT, (uint32_t)ticks);
		}
		else
		{
			lapic->Write(APIC_TIMER, lvt.raw);
			lapic->Write(APIC_TICR, (uint32_t)ticks);
		}

		// /* FIXME: Sometimes APIC stops firing when debugging, why? */
		// LVTTimer timer{};
		// timer.VEC = uint8_t(Line + CPU::x86::IRQ0);
		// timer.TMM = LVTTimerMode::OneShot;

		// LVTTimerDivide Divider = DivideBy8;

		// if (this->lapic->x2APIC)
		// {
		// 	// wrmsr(MSR_X2APIC_DIV_CONF, Divider); <- gpf on real hardware
		// 	wrmsr(MSR_X2APIC_INIT_COUNT, uint32_t(TicksPerSecond * std::chrono::duration_cast<std::chrono::milliseconds>(ns).count()));
		// 	wrmsr(MSR_X2APIC_LVT_TIMER, uint32_t(timer.raw));
		// }
		// else
		// {
		// 	this->lapic->Write(APIC_TDCR, Divider);
		// 	this->lapic->Write(APIC_TICR, uint32_t(TicksPerSecond * std::chrono::duration_cast<std::chrono::milliseconds>(ns).count()));
		// 	this->lapic->Write(APIC_TIMER, uint32_t(timer.raw));
		// }
	}

	void Timer::SetPeriodic(std::chrono::nanoseconds ns, Interrupt::IRQLines Line)
	{
		uint64_t ticks = (ns.count() * TicksPerSecond) / 1'000'000'000ULL;

		if (ticks == 0)
			ticks = 1;

		LVTTimer lvt{};
		lvt.VEC = uint8_t(Line + (int)CPU::x86::IRQ0);
		lvt.M = Unmasked;
		lvt.TMM = Periodic;

		if (lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_LVT_TIMER, lvt.raw);
			wrmsr(MSR_X2APIC_INIT_COUNT, (uint32_t)ticks);
		}
		else
		{
			lapic->Write(APIC_TIMER, lvt.raw);
			lapic->Write(APIC_TICR, (uint32_t)ticks);
		}
	}

	Timer::Timer(Local *local) : lapic(local)
	{
		LVTTimerDivide Divider = DivideBy8;

		debug("Initializing APIC timer on CPU %d", GetCurrentCPU()->ID);

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

		thisClock->Sleep(1ms);

		// Mask the timer
		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_LVT_TIMER, 0x10000 /* LVTTimer.Mask flag */);
			TicksPerSecond = 0xFFFFFFFF - rdmsr(MSR_X2APIC_CUR_COUNT);
		}
		else
		{
			this->lapic->Write(APIC_TIMER, 0x10000 /* LVTTimer.Mask flag */);
			TicksPerSecond = 0xFFFFFFFF - this->lapic->Read(APIC_TCCR);
		}

		// Config for IRQ0 timer
		LVTTimer timer{};
		timer.VEC = IRQ0 + CPU::x86::IRQ0;
		timer.M = Unmasked;
		timer.TMM = OneShot;

		// Initialize APIC timer
		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_DIV_CONF, Divider);
			wrmsr(MSR_X2APIC_INIT_COUNT, TicksPerSecond);
			wrmsr(MSR_X2APIC_LVT_TIMER, timer.raw);
		}
		else
		{
			this->lapic->Write(APIC_TDCR, Divider);
			this->lapic->Write(APIC_TICR, uint32_t(TicksPerSecond));
			this->lapic->Write(APIC_TIMER, uint32_t(timer.raw));
		}

		debug("%d APIC Timer %d ticks in", GetCurrentCPU()->ID, TicksPerSecond);
	}

	Timer::~Timer()
	{
		// Mask the timer
		if (this->lapic->x2APIC)
		{
			wrmsr(MSR_X2APIC_LVT_TIMER, 0x10000 /* LVTTimer.Mask flag */);
		}
		else
		{
			this->lapic->Write(APIC_TIMER, 0x10000 /* LVTTimer.Mask flag */);
		}
	}
}
