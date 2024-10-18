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
#include "../arch/amd64/cpu/apic.hpp"
#include "../arch/amd64/cpu/gdt.hpp"
#include "../arch/amd64/cpu/idt.hpp"
#elif defined(a32)
#include "../arch/i386/cpu/apic.hpp"
#include "../arch/i386/cpu/gdt.hpp"
#include "../arch/i386/cpu/idt.hpp"
#elif defined(aa64)
#endif

#include "../kernel.h"

void HandleException(CPU::ExceptionFrame *Frame);

extern "C" nsa void ExceptionHandler(void *Frame)
{
	HandleException((CPU::ExceptionFrame *)Frame);
}

namespace Interrupts
{
	struct Event
	{
		/** Interrupt number */
		int IRQ;

		/** Raw pointer to the Handler */
		void *Data;

		/** Is this a handler? */
		bool IsHandler;

		/**
		 * Function to call if this is not a Handler
		 *
		 * Used for C-style callbacks.
		 */
		void (*Callback)(CPU::TrapFrame *);

		/**
		 * Context for the callback
		 *
		 * Used for C-style callbacks if the callback needs a context.
		 * (e.g. a driver)
		 */
		void *Context;

		/**
		 * Priority of the event
		 *
		 * Used for sorting the events.
		 *
		 * This is incremented every time the event is called.
		 *
		 * This will improve performance by reducing the time
		 * spent on searching for the event.
		 */
		unsigned long Priority;

		/**
		 * If this is true, the event is critical.
		 *
		 * This will make sure that the event will not be
		 * removed by the kernel.
		 *
		 * This is used to prevent the kernel from removing
		 * ACPI related handlers. (SCI interrupts)
		 */
		bool Critical;
	};
	std::list<Event> RegisteredEvents;

#ifdef DEBUG
#define SORT_DIVIDER 10
#else
#define SORT_DIVIDER 1
#endif

#define SORT_START 10000
	std::atomic_uint SortEvents = SORT_START / SORT_DIVIDER;
	constexpr uint32_t SORT_ITR = (SORT_START * 100) / SORT_DIVIDER;

#if defined(a86)
	/* APIC::APIC */ void *apic[MAX_CPU] = {nullptr};
	/* APIC::Timer */ void *apicTimer[MAX_CPU] = {nullptr};
#elif defined(aa64)
#endif

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
				((APIC::APIC *)apic[Core])->RedirectIRQs(uint8_t(Core));
		}
		else
		{
			error("LAPIC not found");
			// TODO: PIC
		}
#elif defined(aa64)
		warn("aarch64 is not supported yet");
#endif
		CPU::Interrupts(CPU::Enable);
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

	nsa void RemoveAll()
	{
		forItr(itr, RegisteredEvents)
		{
			if (itr->Critical)
				continue;
			RegisteredEvents.erase(itr);
		}
	}

	void AddHandler(void (*Callback)(CPU::TrapFrame *),
					int InterruptNumber,
					void *ctx, bool Critical)
	{
		/* Just log a warning if the interrupt is already registered. */
		foreach (auto ev in RegisteredEvents)
		{
			if (ev.IRQ == InterruptNumber &&
				ev.Callback == Callback)
			{
				warn("IRQ%d is already registered.",
					 InterruptNumber);
			}
		}

		Event newEvent =
			{InterruptNumber, /* IRQ */
			 nullptr,		  /* Data */
			 false,			  /* IsHandler */
			 Callback,		  /* Callback */
			 ctx,			  /* Context */
			 0,				  /* Priority */
			 Critical};		  /* Critical */
		RegisteredEvents.push_back(newEvent);
		debug("Registered interrupt handler for IRQ%d to %#lx",
			  InterruptNumber, Callback);
	}

	void RemoveHandler(void (*Callback)(CPU::TrapFrame *), int InterruptNumber)
	{
		forItr(itr, RegisteredEvents)
		{
			if (itr->IRQ == InterruptNumber &&
				itr->Callback == Callback)
			{
				RegisteredEvents.erase(itr);
				debug("Unregistered interrupt handler for IRQ%d to %#lx",
					  InterruptNumber, Callback);
				return;
			}
		}
		warn("Event %d not found.", InterruptNumber);
	}

	void RemoveHandler(void (*Callback)(CPU::TrapFrame *))
	{
		forItr(itr, RegisteredEvents)
		{
			if (itr->Callback == Callback)
			{
				debug("Removing handle %d %#lx", itr->IRQ,
					  itr->IsHandler
						  ? itr->Data
						  : (void *)itr->Callback);

				RegisteredEvents.erase(itr);
			}
		}
		warn("Handle not found.");
	}

	void RemoveHandler(int InterruptNumber)
	{
		forItr(itr, RegisteredEvents)
		{
			if (itr->IRQ == InterruptNumber)
			{
				debug("Removing handle %d %#lx", itr->IRQ,
					  itr->IsHandler
						  ? itr->Data
						  : (void *)itr->Callback);

				RegisteredEvents.erase(itr);
			}
		}
		warn("IRQ%d not found.", InterruptNumber);
	}

	extern "C" nsa void MainInterruptHandler(void *Data)
	{
		CPU::TrapFrame *Frame = (CPU::TrapFrame *)Data;
		// debug("IRQ%ld", Frame->InterruptNumber - 32);

		/* If this is false, we have a big problem. */
		if (unlikely(Frame->InterruptNumber >= CPU::x86::IRQ223 ||
					 Frame->InterruptNumber <= CPU::x86::ISR0))
		{
			error("Interrupt number %d is out of range.",
				  Frame->InterruptNumber);
			assert(!"Interrupt number is out of range.");
		}

		/* Halt core interrupt */
		if (unlikely(Frame->InterruptNumber == CPU::x86::IRQ31))
			CPU::Stop();

		bool InterruptHandled = false;
		int iEvNum = -1;
		foreach (auto &ev in RegisteredEvents)
		{
			iEvNum = ev.IRQ;
#if defined(a86)
			iEvNum += CPU::x86::IRQ0;
#endif
			if (iEvNum != s_cst(int, Frame->InterruptNumber))
				continue;

			if (ev.IsHandler)
			{
				Handler *hnd = (Handler *)ev.Data;
				hnd->OnInterruptReceived(Frame);
			}
			else
			{
				if (ev.Context != nullptr)
					ev.Callback((CPU::TrapFrame *)ev.Context);
				else
					ev.Callback(Frame);
			}
			ev.Priority++;
			InterruptHandled = true;
		}

		CPUData *CoreData = GetCurrentCPU();
		int Core = CoreData->ID;

		if (unlikely(!InterruptHandled))
		{
			error("IRQ%d is unhandled on CPU %d.",
				  Frame->InterruptNumber - 32, Core);
		}

		/* TODO: This should be done when the os is idle */
		if (SortEvents++ > SORT_ITR)
		{
			debug("Sorting events");
			SortEvents = 0;
			RegisteredEvents.sort([](const Event &a, const Event &b)
								  { return a.Priority < b.Priority; });

#ifdef DEBUG
			foreach (auto ev in RegisteredEvents)
			{
				void *fct = ev.IsHandler
								? ev.Data
								: (void *)ev.Callback;
				const char *symbol = ev.IsHandler
										 ? "class"
										 : KernelSymbolTable->GetSymbol((uintptr_t)fct);

				debug("Event IRQ%d [%#lx %s] has priority %ld",
					  ev.IRQ, fct, symbol, ev.Priority);
			}
#endif
		}

		if (likely(apic[Core]))
		{
			APIC::APIC *this_apic = (APIC::APIC *)apic[Core];
			this_apic->EOI();
			// TODO: Handle PIC too
			return;
		}
		else
			fixme("APIC not found for core %d", Core);
		// TODO: PIC

		assert(!"Interrupt EOI not handled.");
		CPU::Stop();
	}

	Handler::Handler(int InterruptNumber, bool Critical)
	{
		foreach (auto ev in RegisteredEvents)
		{
			if (ev.IRQ == InterruptNumber)
			{
				warn("IRQ%d is already registered.",
					 InterruptNumber);
			}
		}

		this->InterruptNumber = InterruptNumber;

		Event newEvent =
			{InterruptNumber, /* IRQ */
			 this,			  /* Data */
			 true,			  /* IsHandler */
			 nullptr,		  /* Callback */
			 nullptr,		  /* Context */
			 0,				  /* Priority */
			 Critical};		  /* Critical */
		RegisteredEvents.push_back(newEvent);
		debug("Registered interrupt handler for IRQ%d.",
			  InterruptNumber);
	}

	Handler::Handler(PCI::PCIDevice Device, bool Critical)
	{
		PCI::PCIHeader0 *hdr0 =
			(PCI::PCIHeader0 *)Device.Header;
		Handler(hdr0->InterruptLine, Critical);
	}

	Handler::Handler()
	{
		debug("Empty interrupt handler.");
	}

	Handler::~Handler()
	{
		debug("Unregistering interrupt handler for IRQ%d.",
			  this->InterruptNumber);

		forItr(itr, RegisteredEvents)
		{
			if (itr->IRQ == this->InterruptNumber)
			{
				RegisteredEvents.erase(itr);
				return;
			}
		}
		warn("Event %d not found.", this->InterruptNumber);
	}

	void Handler::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		trace("Unhandled interrupt %d",
			  Frame->InterruptNumber);
	}
}
