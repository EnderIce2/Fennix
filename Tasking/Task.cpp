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

#include <task.hpp>

#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>
#include <io.h>

#include "../kernel.h"

#if defined(a64)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../Architecture/i386/cpu/apic.hpp"
#elif defined(aa64)
#endif

// #define DEBUG_TASKING 1

#ifdef DEBUG_TASKING
#define tskdbg(m, ...)       \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define tskdbg(m, ...)
#endif

namespace Tasking
{
#if defined(a86)
	__naked __used __no_stack_protector void IdleProcessLoop()
	{
		asmv("IdleLoop:");
		asmv("hlt");
		asmv("jmp IdleLoop");
#elif defined(aa64)
	__used __no_stack_protector void IdleProcessLoop()
	{
		asmv("IdleLoop:");
		asmv("wfe");
		asmv("b IdleLoop");
#endif
	}

	SafeFunction bool Task::InvalidPCB(PCB *pcb)
	{
		if (!pcb)
			return true;

		/* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
		if (pcb >= (PCB *)(UINTPTR_MAX - 0x1ffe))
			return true;

		/* In this section of the memory is reserved by the kernel. */
		if (pcb < (PCB *)(0x1000))
			return true;

		/* Check if it's mapped. */
		if (!Memory::Virtual().Check((void *)pcb))
			return true;

		return false;
	}

	SafeFunction bool Task::InvalidTCB(TCB *tcb)
	{
		if (!tcb)
			return true;

		/* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
		if (tcb >= (TCB *)(UINTPTR_MAX - 0x1ffe))
			return true;

		/* In this section of the memory is reserved by the kernel. */
		if (tcb < (TCB *)(0x1000))
			return true;

		/* Check if it's mapped. */
		if (!Memory::Virtual().Check((void *)tcb))
			return true;

		return false;
	}

	SafeFunction void Task::RemoveThread(TCB *Thread)
	{
		foreach (TCB *tcb in Thread->Parent->Threads)
		{
			if (tcb == Thread)
			{
				debug("Thread \"%s\"(%d) removed from process \"%s\"(%d)",
					  Thread->Name, Thread->ID, Thread->Parent->Name,
					  Thread->Parent->ID);
				delete tcb;
				break;
			}
		}
	}

	SafeFunction void Task::RemoveProcess(PCB *Process)
	{
		if (InvalidPCB(Process))
			return;

		if (Process->Status == Terminated)
		{
			delete Process;
			return;
		}

		foreach (TCB *thread in Process->Threads)
		{
			if (thread->Status == Terminated)
				RemoveThread(thread);
		}
	}

	SafeFunction void Task::UpdateUsage(TaskInfo *Info, TaskExecutionMode Mode, int Core)
	{
		UNUSED(Core);
		uint64_t CurrentTime = TimeManager->GetCounter();
		uint64_t TimePassed = CurrentTime - Info->LastUpdateTime;
		// Info->LastUpdateTime = CurrentTime;

		if (Mode == TaskExecutionMode::User)
			Info->UserTime += TimePassed;
		else
			Info->KernelTime += TimePassed;
	}

	PCB *Task::GetCurrentProcess()
	{
		return GetCurrentCPU()->CurrentProcess.load();
	}

	TCB *Task::GetCurrentThread()
	{
		return GetCurrentCPU()->CurrentThread.load();
	}

	PCB *Task::GetProcessByID(TID ID)
	{
		SmartLock(TaskingLock);
		foreach (auto p in ProcessList)
			if (p->ID == ID)
				return p;
		return nullptr;
	}

	TCB *Task::GetThreadByID(TID ID)
	{
		SmartLock(TaskingLock);
		foreach (auto p in ProcessList)
		{
			foreach (auto t in p->Threads)
				if (t->ID == ID)
					return t;
		}
		return nullptr;
	}

	void Task::WaitForProcess(PCB *pcb)
	{
		if (InvalidPCB(pcb))
			return;

		if (pcb->Status == TaskStatus::UnknownStatus)
			return;

		debug("Waiting for process \"%s\"(%d)",
			  pcb->Name, pcb->ID);

		while (pcb->Status != TaskStatus::Terminated)
			this->Yield();
	}

	void Task::WaitForThread(TCB *tcb)
	{
		if (InvalidTCB(tcb))
			return;

		if (tcb->Status == TaskStatus::UnknownStatus)
			return;

		debug("Waiting for thread \"%s\"(%d)",
			  tcb->Name, tcb->ID);

		while (tcb->Status != TaskStatus::Terminated)
			this->Yield();
	}

	void Task::WaitForProcessStatus(PCB *pcb, TaskStatus status)
	{
		if (InvalidPCB(pcb))
			return;

		if (pcb->Status == TaskStatus::UnknownStatus)
			return;

		debug("Waiting for process \"%s\"(%d) to reach status: %d",
			  pcb->Name, pcb->ID, status);

		while (pcb->Status != status)
			this->Yield();
	}

	void Task::WaitForThreadStatus(TCB *tcb, TaskStatus status)
	{
		if (InvalidTCB(tcb))
			return;

		if (tcb->Status == TaskStatus::UnknownStatus)
			return;

		debug("Waiting for thread \"%s\"(%d) to reach status: %d",
			  tcb->Name, tcb->ID, status);

		while (tcb->Status != status)
			this->Yield();
	}

	void Task::Sleep(uint64_t Milliseconds, bool NoSwitch)
	{
		TCB *thread = this->GetCurrentThread();
		PCB *process = this->GetCurrentProcess();

		thread->Status = TaskStatus::Sleeping;

		{
			SmartLock(TaskingLock);
			if (process->Threads.size() == 1)
				process->Status = TaskStatus::Sleeping;

			thread->Info.SleepUntil =
				TimeManager->CalculateTarget(Milliseconds,
											 Time::Units::Milliseconds);
		}

#ifdef DEBUG
		uint64_t TicksNow = TimeManager->GetCounter();
#endif
		debug("Thread \"%s\"(%d) is going to sleep until %llu, current %llu, diff %llu",
			  thread->Name, thread->ID, thread->Info.SleepUntil,
			  TicksNow, thread->Info.SleepUntil - TicksNow);

		if (!NoSwitch)
			this->Yield();
	}

	void Task::SignalShutdown()
	{
		fixme("SignalShutdown()");
		// TODO: Implement this
		// This should hang until all processes are terminated
	}

	void Task::CleanupProcessesThread()
	{
		thisThread->Rename("Tasking Cleanup");
		while (true)
		{
			this->Sleep(2000);
			{
				SmartLock(TaskingLock);
				foreach (auto process in ProcessList)
				{
					if (InvalidPCB(process))
						continue;

					RemoveProcess(process);
				}
			}
		}
	}

	__no_sanitize("undefined")
		TCB *Task::CreateThread(PCB *Parent,
								IP EntryPoint,
								const char **argv,
								const char **envp,
								const std::vector<AuxiliaryVector> &auxv,
								TaskArchitecture Architecture,
								TaskCompatibility Compatibility,
								bool ThreadNotReady)
	{
		SmartLock(TaskingLock);
		return new TCB(this, Parent, EntryPoint,
					   argv, envp, auxv, Architecture,
					   Compatibility, ThreadNotReady);
	}

	PCB *Task::CreateProcess(PCB *Parent,
							 const char *Name,
							 TaskExecutionMode ExecutionMode,
							 void *Image,
							 bool DoNotCreatePageTable,
							 uint16_t UserID,
							 uint16_t GroupID)
	{
		SmartLock(TaskingLock);
		return new PCB(this, Parent, Name, ExecutionMode,
					   Image, DoNotCreatePageTable,
					   UserID, GroupID);
	}

	Task::Task(const IP EntryPoint) : Interrupts::Handler(16) /* IRQ16 */
	{
#if defined(a64)
		// Map the IRQ16 to the first CPU.
		((APIC::APIC *)Interrupts::apic[0])->RedirectIRQ(0, CPU::x86::IRQ16 - CPU::x86::IRQ0, 1);
#elif defined(a32)
#elif defined(aa64)
#endif
		KPrint("Starting Tasking With Instruction Pointer: %p (\e666666%s\eCCCCCC)",
			   EntryPoint, KernelSymbolTable->GetSymbolFromAddress(EntryPoint));

#if defined(a64)
		TaskArchitecture Arch = TaskArchitecture::x64;
#elif defined(a32)
		TaskArchitecture Arch = TaskArchitecture::x32;
#elif defined(aa64)
	TaskArchitecture Arch = TaskArchitecture::ARM64;
#endif

		PCB *kproc = CreateProcess(nullptr, "Kernel", TaskExecutionMode::Kernel);
		kproc->ELFSymbolTable = KernelSymbolTable;
		TCB *kthrd = CreateThread(kproc, EntryPoint, nullptr, nullptr, std::vector<AuxiliaryVector>(), Arch);
		kthrd->Rename("Main Thread");
		debug("Created Kernel Process: %s and Thread: %s",
			  kproc->Name, kthrd->Name);

		bool MONITORSupported = false;
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid;
			cpuid.Get();
			MONITORSupported = cpuid.ECX.MONITOR;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
			cpuid.Get();
			MONITORSupported = cpuid.ECX.MONITOR;
		}

		if (MONITORSupported)
		{
			trace("CPU has MONITOR/MWAIT support.");
		}

		if (!CPU::Interrupts(CPU::Check))
		{
			error("Interrupts are not enabled.");
			CPU::Interrupts(CPU::Enable);
		}

		IdleProcess = CreateProcess(nullptr, (char *)"Idle", TaskExecutionMode::Kernel);
		IdleProcess->ELFSymbolTable = KernelSymbolTable;
		for (int i = 0; i < SMP::CPUCores; i++)
		{
			IdleThread = CreateThread(IdleProcess, reinterpret_cast<uintptr_t>(IdleProcessLoop));
			char IdleName[16];
			sprintf(IdleName, "Idle Thread %d", i);
			IdleThread->Rename(IdleName);
			IdleThread->SetPriority(Idle);
			for (int j = 0; j < MAX_CPU; j++)
				IdleThread->Info.Affinity[j] = false;
			IdleThread->Info.Affinity[i] = true;
		}
		debug("Tasking Started");
#if defined(a64)
		((APIC::Timer *)Interrupts::apicTimer[0])->OneShot(CPU::x86::IRQ16, 100);

		/* FIXME: The kernel is not ready for multi-core tasking. */
		// for (int i = 1; i < SMP::CPUCores; i++)
		// {
		//     ((APIC::Timer *)Interrupts::apicTimer[i])->OneShot(CPU::x86::IRQ16, 100);
		//     APIC::InterruptCommandRegisterLow icr;
		//     icr.Vector = CPU::x86::IRQ16;
		//     icr.Level = APIC::APICLevel::Assert;
		//     ((APIC::APIC *)Interrupts::apic[0])->IPI(i, icr);
		// }
#elif defined(a32)
#elif defined(aa64)
#endif
	}

	Task::~Task()
	{
		debug("Destructor called");
		{
			SmartLock(TaskingLock);
			foreach (PCB *Process in ProcessList)
			{
				foreach (TCB *Thread in Process->Threads)
				{
					if (Thread == GetCurrentCPU()->CurrentThread.load() ||
						Thread == CleanupThread)
						continue;
					this->KillThread(Thread, KILL_SCHEDULER_DESTRUCTION);
				}

				if (Process == GetCurrentCPU()->CurrentProcess.load())
					continue;
				this->KillProcess(Process, KILL_SCHEDULER_DESTRUCTION);
			}
		}

		while (ProcessList.size() > 0)
		{
			trace("Waiting for %d processes to terminate", ProcessList.size());
			int NotTerminated = 0;
			foreach (PCB *Process in ProcessList)
			{
				debug("Process %s(%d) is still running (or waiting to be removed status %#lx)",
					  Process->Name, Process->ID, Process->Status);
				if (Process->Status == TaskStatus::Terminated)
					continue;
				NotTerminated++;
			}
			if (NotTerminated == 1)
				break;
			TaskingScheduler_OneShot(100);
		}

		debug("Tasking stopped");
	}
}
