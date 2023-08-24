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

#include <ints.hpp>

#include <syscalls.hpp>
#include <acpi.hpp>
#include <smp.hpp>
#include <vector>
#include <io.h>

#if defined(a64)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#include "../Architecture/amd64/cpu/idt.hpp"
#elif defined(a32)
#include "../Architecture/i386/cpu/apic.hpp"
#include "../Architecture/i386/cpu/gdt.hpp"
#include "../Architecture/i386/cpu/idt.hpp"
#elif defined(aa64)
#endif

#include "crashhandler.hpp"
#include "../kernel.h"

extern "C" SafeFunction void ExceptionHandler(void *Data)
{
	CrashHandler::Handle(Data);
}

namespace Interrupts
{
	struct Event
	{
		int ID;
		void *Data;
	};
	std::vector<Event> RegisteredEvents;

#if defined(a86)
	/* APIC::APIC */ void *apic[MAX_CPU];
	/* APIC::Timer */ void *apicTimer[MAX_CPU];
#elif defined(aa64)
#endif
	void *InterruptFrames[INT_FRAMES_MAX];

	void Initialize(int Core)
	{
#if defined(a64)
		GlobalDescriptorTable::Init(Core);
		InterruptDescriptorTable::Init(Core);
		CPUData *CoreData = GetCPU(Core);
		CoreData->Checksum = CPU_DATA_CHECKSUM;
		CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, (uint64_t)CoreData);
		CPU::x64::wrmsr(CPU::x64::MSR_SHADOW_GS_BASE, (uint64_t)CoreData);
		CoreData->ID = Core;
		CoreData->IsActive = true;
		CoreData->Stack = (uintptr_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1)) + STACK_SIZE;
		if (CoreData->Checksum != CPU_DATA_CHECKSUM)
		{
			KPrint("CPU %d checksum mismatch! %x != %x",
				   Core, CoreData->Checksum, CPU_DATA_CHECKSUM);
			CPU::Stop();
		}
		debug("Stack for core %d is %#lx (Address: %#lx)",
			  Core, CoreData->Stack, CoreData->Stack - STACK_SIZE);
		InitializeSystemCalls();
#elif defined(a32)
		GlobalDescriptorTable::Init(Core);
		InterruptDescriptorTable::Init(Core);
		CPUData *CoreData = GetCPU(Core);
		CoreData->Checksum = CPU_DATA_CHECKSUM;
		CPU::x32::wrmsr(CPU::x32::MSR_GS_BASE, (uint64_t)CoreData);
		CPU::x32::wrmsr(CPU::x32::MSR_SHADOW_GS_BASE, (uint64_t)CoreData);
		CoreData->ID = Core;
		CoreData->IsActive = true;
		CoreData->Stack = (uintptr_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1)) + STACK_SIZE;
		if (CoreData->Checksum != CPU_DATA_CHECKSUM)
		{
			KPrint("CPU %d checksum mismatch! %x != %x",
				   Core, CoreData->Checksum, CPU_DATA_CHECKSUM);
			CPU::Stop();
		}
		debug("Stack for core %d is %#lx (Address: %#lx)",
			  Core, CoreData->Stack, CoreData->Stack - STACK_SIZE);
#elif defined(aa64)
		warn("aarch64 is not supported yet");
#endif
	}

	void Enable(int Core)
	{
#if defined(a86)
		if (((ACPI::MADT *)PowerManager->GetMADT())->LAPICAddress != nullptr)
		{
			// TODO: This function is called by SMP too. Do not initialize timers that doesn't support multiple cores.
			apic[Core] = new APIC::APIC(Core);
			if (Core == Config.IOAPICInterruptCore) // Redirect IRQs to the specified core.
				((APIC::APIC *)apic[Core])->RedirectIRQs(Core);
		}
		else
		{
			error("LAPIC not found");
			// TODO: PIC
		}
#elif defined(aa64)
		warn("aarch64 is not supported yet");
#endif
	}

	void InitializeTimer(int Core)
	{
		// TODO: This function is called by SMP too. Do not initialize timers that doesn't support multiple cores.
#if defined(a86)
		if (apic[Core] != nullptr)
			apicTimer[Core] = new APIC::Timer((APIC::APIC *)apic[Core]);
		else
		{
			fixme("apic not found");
		}
#elif defined(aa64)
		warn("aarch64 is not supported yet");
#endif
	}

	SafeFunction void RemoveAll()
	{
		RegisteredEvents.clear();
	}

	extern "C" SafeFunction void MainInterruptHandler(void *Data)
	{
#if defined(a64)
		CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;
#elif defined(a32)
		CPU::x32::TrapFrame *Frame = (CPU::x32::TrapFrame *)Data;
#elif defined(aa64)
		CPU::aarch64::TrapFrame *Frame = (CPU::aarch64::TrapFrame *)Data;
#endif
		// debug("IRQ%ld", Frame->InterruptNumber - 32);

		memmove(InterruptFrames + 1, InterruptFrames, sizeof(InterruptFrames) - sizeof(InterruptFrames[0]));
#if defined(a64)
		InterruptFrames[0] = (void *)Frame->rip;
#elif defined(a32)
		InterruptFrames[0] = (void *)Frame->eip;
#elif defined(aa64)
		InterruptFrames[0] = (void *)Frame->elr_el1;
#endif

		CPUData *CoreData = GetCurrentCPU();
		int Core = 0;
		if (likely(CoreData != nullptr))
			Core = CoreData->ID;

		/* If this is false, we have a big problem. */
		if (likely(Frame->InterruptNumber < CPU::x86::IRQ223 && Frame->InterruptNumber > CPU::x86::ISR0))
		{
			/* Halt core interrupt */
			if (unlikely(Frame->InterruptNumber == CPU::x86::IRQ29))
				CPU::Stop();

			bool InterruptHandled = false;
			foreach (auto ev in RegisteredEvents)
			{
#if defined(a86)
				if ((ev.ID + CPU::x86::IRQ0) == s_cst(int, Frame->InterruptNumber))
#elif defined(aa64)
				if (ev.ID == s_cst(int, Frame->InterruptNumber))
#endif
				{
					Handler *hnd = (Handler *)ev.Data;
					hnd->OnInterruptReceived(Frame);
					InterruptHandled = true;
				}
			}

			if (!InterruptHandled)
			{
				error("IRQ%d is unhandled on CPU %d.", Frame->InterruptNumber - 32, Core);
				if (Frame->InterruptNumber == CPU::x86::IRQ1)
				{
					uint8_t scancode = inb(0x60);
					warn("IRQ1 is the keyboard interrupt. Scancode: %#x", scancode);
				}
			}

			if (likely(apic[Core]))
			{
				((APIC::APIC *)Interrupts::apic[Core])->EOI();
				// TODO: Handle PIC too
				return;
			}
			else
				fixme("APIC not found for core %d", Core);
			// TODO: PIC
		}
		else
		{
			error("Interrupt number %d is out of range.",
				  Frame->InterruptNumber);
		}

		error("HALT HALT HALT HALT HALT HALT HALT HALT HALT [IRQ%d]",
			  Frame->InterruptNumber - 32);
		CPU::Stop();
	}

	Handler::Handler(int InterruptNumber)
	{
		foreach (auto ev in RegisteredEvents)
		{
			if (ev.ID == InterruptNumber)
			{
				warn("IRQ%d is already registered.", InterruptNumber);
			}
		}

		debug("Registering interrupt handler for IRQ%d.", InterruptNumber);
		this->InterruptNumber = InterruptNumber;
		RegisteredEvents.push_back({InterruptNumber, this});
	}

	Handler::~Handler()
	{
		debug("Unregistering interrupt handler for IRQ%d.", this->InterruptNumber);

		forItr(itr, RegisteredEvents)
		{
			if (itr->ID == this->InterruptNumber)
			{
				RegisteredEvents.erase(itr);
				return;
			}
		}
		warn("Event %d not found.", this->InterruptNumber);
	}

#if defined(a64)
	void Handler::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
	{
		trace("Unhandled interrupt IRQ%d", Frame->InterruptNumber - 32);
#elif defined(a32)
	void Handler::OnInterruptReceived(CPU::x32::TrapFrame *Frame)
	{
		trace("Unhandled interrupt received");
#elif defined(aa64)
	void Handler::OnInterruptReceived(CPU::aarch64::TrapFrame *Frame)
	{
		trace("Unhandled interrupt received");
#endif
	}
}
