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

#include <smp.hpp>

#include <memory.hpp>
#include <acpi.hpp>
#include <ints.hpp>
#include <assert.h>
#include <cpu.hpp>
#include <atomic>

#include "../../../kernel.h"
#include "apic.hpp"

extern "C" uint64_t _trampoline_start, _trampoline_end;

/* https://wiki.osdev.org/Memory_Map_(x86) */
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

std::atomic_bool CPUEnabled = false;

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static __aligned(PAGE_SIZE) CPUData CPUs[MAX_CPU] = {0};

SafeFunction CPUData *GetCPU(long id) { return &CPUs[id]; }

SafeFunction CPUData *GetCurrentCPU()
{
	if (unlikely(!Interrupts::apic[0]))
		return &CPUs[0]; /* No APIC means we are on the BSP. */

	APIC::APIC *apic = (APIC::APIC *)Interrupts::apic[0];
	int CoreID = 0;
	if (CPUEnabled.load(std::memory_order_acquire) == true)
	{
		if (apic->x2APIC)
			CoreID = int(CPU::x64::rdmsr(CPU::x64::MSR_X2APIC_APICID));
		else
			CoreID = apic->Read(APIC::APIC_ID) >> 24;
	}

	if (unlikely((&CPUs[CoreID])->IsActive != true))
	{
		error("CPU %d is not active!", CoreID);
		assert((&CPUs[0])->IsActive == true); /* We can't continue without the BSP. */
		return &CPUs[0];
	}

	assert((&CPUs[CoreID])->Checksum == CPU_DATA_CHECKSUM); /* This should never happen. */
	return &CPUs[CoreID];
}

extern "C" void StartCPU()
{
	CPU::Interrupts(CPU::Disable);
	int CoreID = (int)*reinterpret_cast<int *>(CORE);
	CPU::InitializeFeatures(CoreID);
	// Initialize GDT and IDT
	Interrupts::Initialize(CoreID);
	Interrupts::Enable(CoreID);
	Interrupts::InitializeTimer(CoreID);
	asmv("mov %0, %%rsp" ::"r"((&CPUs[CoreID])->Stack));

	CPU::Interrupts(CPU::Enable);
	KPrint("\e058C19CPU \e8888FF%d \e058C19is online", CoreID);
	CPUEnabled.store(true, std::memory_order_release);
	CPU::Halt(true);
}

namespace SMP
{
	int CPUCores = 0;

	void Initialize(void *_madt)
	{
		if (!_madt)
		{
			error("MADT is NULL");
			return;
		}

		ACPI::MADT *madt = (ACPI::MADT *)_madt;

		if (madt->lapic.size() < 1)
		{
			error("No CPUs found!");
			return;
		}

		int Cores = madt->CPUCores + 1;

		if (Config.Cores > madt->CPUCores + 1)
			KPrint("More cores requested than available. Using %d cores",
				   madt->CPUCores + 1);
		else if (Config.Cores != 0)
			Cores = Config.Cores;

		CPUCores = Cores;

		uint64_t TrampolineLength = (uintptr_t)&_trampoline_end -
									(uintptr_t)&_trampoline_start;
		Memory::Virtual().Map(0x0, 0x0, Memory::PTFlag::RW);
		/* We reserved the TRAMPOLINE_START address inside Physical class. */
		Memory::Virtual().Map((void *)TRAMPOLINE_START, (void *)TRAMPOLINE_START,
							  TrampolineLength, Memory::PTFlag::RW);
		memcpy((void *)TRAMPOLINE_START, &_trampoline_start, TrampolineLength);
		debug("Trampoline address: %#lx-%#lx",
			  TRAMPOLINE_START,
			  TRAMPOLINE_START + TrampolineLength);

		void *CPUTmpStack = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1));
		asmv("sgdt [0x580]");
		asmv("sidt [0x590]");
		VPOKE(uintptr_t, STACK) = (uintptr_t)CPUTmpStack + STACK_SIZE;
		VPOKE(uintptr_t, PAGE_TABLE) = (uintptr_t)KernelPageTable;
		VPOKE(uintptr_t, START_ADDR) = (uintptr_t)&StartCPU;

		for (int i = 0; i < Cores; i++)
		{
			ACPI::MADT::LocalAPIC *lapic = madt->lapic[i];
			APIC::APIC *apic = (APIC::APIC *)Interrupts::apic[0];

			debug("Initializing CPU %d", lapic->APICId);
			uint8_t APIC_ID = 0;
			if (apic->x2APIC)
				APIC_ID = uint8_t(CPU::x64::rdmsr(CPU::x64::MSR_X2APIC_APICID));
			else
				APIC_ID = uint8_t(apic->Read(APIC::APIC_ID) >> 24);

			if (APIC_ID != lapic->APICId)
			{
				VPOKE(int, CORE) = i;
				if (!apic->x2APIC)
				{
					APIC::InterruptCommandRegister icr{};
					icr.MT = APIC::INIT;
					icr.DES = lapic->APICId;
					apic->ICR(icr);
				}

				apic->SendInitIPI(lapic->APICId);
				TimeManager->Sleep(20, Time::Units::Milliseconds);
				apic->SendStartupIPI(lapic->APICId, TRAMPOLINE_START);
				debug("Waiting for CPU %d to load...", lapic->APICId);

				uint64_t Timeout = TimeManager->CalculateTarget(2, Time::Units::Seconds);
				while (CPUEnabled.load(std::memory_order_acquire) == false)
				{
					if (TimeManager->GetCounter() > Timeout)
					{
						error("CPU %d failed to load!", lapic->APICId);
						KPrint("\eFF8C19CPU \e8888FF%d \eFF8C19failed to load!",
							   lapic->APICId);
						break;
					}
					CPU::Pause();
				}
				trace("CPU %d loaded.", lapic->APICId);
				CPUEnabled.store(false, std::memory_order_release);
			}
			else
				KPrint("\e058C19CPU \e8888FF%d \e058C19is the BSP", lapic->APICId);
		}

		KernelAllocator.FreePages(CPUTmpStack, TO_PAGES(STACK_SIZE + 1));
		/* We are going to unmap the page after we are done with it. */
		Memory::Virtual().Unmap(0x0);
	}
}
