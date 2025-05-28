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

#if defined(__amd64__)
#include "../arch/amd64/cpu/apic.hpp"
#include "../arch/amd64/cpu/gdt.hpp"
#include "../arch/amd64/cpu/idt.hpp"
#elif defined(__i386__)
#include "../arch/i386/cpu/apic.hpp"
#include "../arch/i386/cpu/gdt.hpp"
#include "../arch/i386/cpu/idt.hpp"
#elif defined(__aarch64__)
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

#if defined(__amd64__) || defined(__i386__)
	/* APIC::APIC */ void *apic[MAX_CPU] = {nullptr};
	/* APIC::Timer */ void *apicTimer[MAX_CPU] = {nullptr};
#elif defined(__aarch64__)
	/* APIC::APIC */ void *apic[MAX_CPU] = {nullptr};
	/* APIC::Timer */ void *apicTimer[MAX_CPU] = {nullptr};
#endif

	void Initialize(int Core)
	{
#if defined(__amd64__) || defined(__i386__)
		GlobalDescriptorTable::Init(Core);
		InterruptDescriptorTable::Init(Core);
		CPUData *CoreData = GetCPU(Core);
		CoreData->Checksum = CPU_DATA_CHECKSUM;
		CPU::x86::wrmsr(CPU::x86::MSR_GS_BASE, (uint64_t)CoreData);
		CPU::x86::wrmsr(CPU::x86::MSR_SHADOW_GS_BASE, (uint64_t)CoreData);
		CoreData->ID = Core;
		CoreData->IsActive = true;
		CoreData->Stack = (uintptr_t)StackManager.Allocate(STACK_SIZE) + STACK_SIZE;
		if (CoreData->Checksum != CPU_DATA_CHECKSUM)
		{
			KPrint("CPU %d checksum mismatch! %x != %x",
				   Core, CoreData->Checksum, CPU_DATA_CHECKSUM);
			CPU::Stop();
		}
		debug("Stack for core %d is %#lx (Address: %#lx)",
			  Core, CoreData->Stack, CoreData->Stack - STACK_SIZE);
		InitializeSystemCalls();
#elif defined(__aarch64__)
		warn("aarch64 is not supported yet");
#endif
	}

	void Enable(int Core)
	{
#if defined(__amd64__) || defined(__i386__)
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
#elif defined(__aarch64__)
		warn("aarch64 is not supported yet");
#endif
		CPU::Interrupts(CPU::Enable);
	}

	void InitializeTimer(int Core)
	{
		// TODO: This function is called by SMP too. Do not initialize timers that doesn't support multiple cores.
#if defined(__amd64__) || defined(__i386__)
		if (apic[Core] != nullptr)
			apicTimer[Core] = new APIC::Timer((APIC::APIC *)apic[Core]);
		else
		{
			fixme("apic not found");
		}
#elif defined(__aarch64__)
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
		for (auto ev : RegisteredEvents)
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

	nsa hot inline void ReturnFromInterrupt()
	{
#if defined(__amd64__) || defined(__i386__)
		CPUData *CoreData = GetCurrentCPU();
		int Core = CoreData->ID;

		/* TODO: This should be done when the os is idle */
		if (SortEvents++ > SORT_ITR)
		{
			debug("Sorting events");
			SortEvents = 0;
			RegisteredEvents.sort([](const Event &a, const Event &b)
								  { return a.Priority < b.Priority; });

#ifdef DEBUG
			for (auto ev : RegisteredEvents)
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
			return;
		}
		else
			fixme("APIC not found for core %d", Core);
		// TODO: Handle PIC too

#endif
		assert(!"EOI not handled.");
		CPU::Stop();
	}

	extern "C" nsa hot void MainInterruptHandler(void *Data)
	{
		class AutoSwitchPageTable
		{
		private:
			void *Original;

		public:
			AutoSwitchPageTable()
			{
#if defined(__amd64__) || defined(__i386__)
				asmv("mov %%cr3, %0" : "=r"(Original));
				if (likely(Original == KernelPageTable))
					return;
				asmv("mov %0, %%cr3" : : "r"(KernelPageTable));
#endif
			}

			~AutoSwitchPageTable()
			{
#if defined(__amd64__) || defined(__i386__)
				if (likely(Original == KernelPageTable))
					return;
				asmv("mov %0, %%cr3" : : "r"(Original));
#endif
			}
		} SwitchPageTable;

#if defined(__amd64__) || defined(__i386__)
		CPU::TrapFrame *Frame = (CPU::TrapFrame *)Data;
		// debug("IRQ%ld", Frame->InterruptNumber - 32);

		/* Halt core interrupt */
		if (unlikely(Frame->InterruptNumber == CPU::x86::IRQ31))
			CPU::Stop();
		assert(Frame->InterruptNumber <= CPU::x86::IRQ223);

		auto it = RegisteredEvents.begin();
		while (it != RegisteredEvents.end())
		{
			int iEvNum = it->IRQ;
#if defined(__amd64__) || defined(__i386__)
			iEvNum += CPU::x86::IRQ0;
#endif
			if (iEvNum == s_cst(int, Frame->InterruptNumber))
				break;
			++it;
		}

		if (it == RegisteredEvents.end())
		{
			warn("IRQ%d is not registered.", Frame->InterruptNumber - 32);
			ReturnFromInterrupt();
			return;
		}

		it->Priority++;

		if (it->IsHandler)
		{
			Handler *hnd = (Handler *)it->Data;
			hnd->OnInterruptReceived(Frame);
		}
		else
		{
			if (it->Context != nullptr)
				it->Callback((CPU::TrapFrame *)it->Context);
			else
				it->Callback(Frame);
		}

		ReturnFromInterrupt();
#endif
	}

	extern "C" nsa hot void SchedulerInterruptHandler(void *Data)
	{
#if defined(__amd64__) || defined(__i386__)
		KernelPageTable->Update();
		CPU::SchedulerFrame *Frame = (CPU::SchedulerFrame *)Data;
#if defined(__amd64__) || defined(__i386__)
		assert(Frame->InterruptNumber == CPU::x86::IRQ16);
#else
		assert(Frame->InterruptNumber == 16);
#endif

		auto it = std::find_if(RegisteredEvents.begin(), RegisteredEvents.end(),
							   [](const Event &ev)
							   {
								   return ev.IRQ == 16;
							   });

		if (it == RegisteredEvents.end())
		{
			warn("Scheduler interrupt is not registered.");
			ReturnFromInterrupt();
			Frame->ppt = Frame->opt;
			debug("opt = %#lx", Frame->opt);
			return;
		}
		assert(it->IsHandler);

		it->Priority++;

		Handler *hnd = (Handler *)it->Data;
		hnd->OnInterruptReceived(Frame);
		ReturnFromInterrupt();
#endif
	}

	Handler::Handler(int InterruptNumber, bool Critical)
	{
		for (auto ev : RegisteredEvents)
		{
			if (ev.IRQ != InterruptNumber)
				continue;

			warn("IRQ%d is already registered.", InterruptNumber);
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
		debug("Registered interrupt handler for IRQ%d.", InterruptNumber);
	}

	Handler::Handler(PCI::PCIDevice Device, bool Critical)
		: Handler(((PCI::PCIHeader0 *)Device.Header)->InterruptLine, Critical)
	{
	}

	Handler::Handler()
	{
		debug("Empty interrupt handler.");
	}

	Handler::~Handler()
	{
		debug("Unregistering interrupt handler for IRQ%d.", this->InterruptNumber);
		forItr(itr, RegisteredEvents)
		{
			if (itr->IRQ != this->InterruptNumber)
				continue;

			RegisteredEvents.erase(itr);
			return;
		}
		warn("Event %d not found.", this->InterruptNumber);
	}

	void Handler::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
#if defined(__amd64__) || defined(__i386__)
		debug("Unhandled interrupt %d", Frame->InterruptNumber);
#endif
	}

	void Handler::OnInterruptReceived(CPU::SchedulerFrame *Frame)
	{
#if defined(__amd64__) || defined(__i386__)
		debug("Unhandled scheduler interrupt %d", Frame->InterruptNumber);
#endif
	}
}
