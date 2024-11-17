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

#ifndef __FENNIX_KERNEL_INTERRUPTS_H__
#define __FENNIX_KERNEL_INTERRUPTS_H__

#include <types.h>
#include <cpu.hpp>
#include <pci.hpp>

namespace Interrupts
{
#ifdef DEBUG // For performance reasons
#define INT_FRAMES_MAX 512
#else
#define INT_FRAMES_MAX 8
#endif

#if defined(a64)
	/* APIC::APIC */ extern void *apic[255];	   // MAX_CPU
	/* APIC::Timer */ extern void *apicTimer[255]; // MAX_CPU
#elif defined(a32)
	/* APIC::APIC */ extern void *apic[255];	   // MAX_CPU
	/* APIC::Timer */ extern void *apicTimer[255]; // MAX_CPU
#elif defined(aa64)
#endif

	void Initialize(int Core);
	void Enable(int Core);
	void InitializeTimer(int Core);
	void RemoveAll();

	void AddHandler(void (*Callback)(CPU::TrapFrame *),
					int InterruptNumber, void *ctx = nullptr,
					bool Critical = false);

	void RemoveHandler(void (*Callback)(CPU::TrapFrame *),
					   int InterruptNumber);

	void RemoveHandler(void (*Callback)(CPU::TrapFrame *));
	void RemoveHandler(int InterruptNumber);

	class Handler
	{
	private:
		int InterruptNumber;

	protected:
		/**
		 * @brief Set a new interrupt number.
		 * @param InterruptNumber The interrupt number. NOT the IRQ number! (IRQ0 != 32)
		 */
		void SetInterruptNumber(int InterruptNumber) { this->InterruptNumber = InterruptNumber; }
		int GetInterruptNumber() { return this->InterruptNumber; }

		/**
		 * @brief Create a new interrupt handler.
		 * @param InterruptNumber The interrupt number. NOT the IRQ number! (IRQ0 != 32)
		 */
		Handler(int InterruptNumber, bool Critical = false);
		Handler(PCI::PCIDevice Device, bool Critical = false);
		Handler();
		~Handler();

	public:
		virtual void OnInterruptReceived(CPU::TrapFrame *Frame);
		virtual void OnInterruptReceived(CPU::SchedulerFrame *Frame);
	};
}

#endif // !__FENNIX_KERNEL_INTERRUPTS_H__
