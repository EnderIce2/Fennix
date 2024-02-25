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
#include "../arch/amd64/cpu/apic.hpp"
#include "../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../arch/i386/cpu/apic.hpp"
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

	nsa bool Task::InvalidPCB(PCB *pcb)
	{
		if (!pcb)
			return true;

		/* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
		if (pcb >= (PCB *)(UINTPTR_MAX - 0x1FFE))
			return true;

		/* In this section of the memory is reserved by the kernel. */
		if (pcb < (PCB *)(0xFFFFF))
			return true;

		/* Check if it's mapped. */
		if (!Memory::Virtual().Check((void *)pcb))
			return true;

		return false;
	}

	nsa bool Task::InvalidTCB(TCB *tcb)
	{
		if (!tcb)
			return true;

		/* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
		if (tcb >= (TCB *)(UINTPTR_MAX - 0x1FFE))
			return true;

		/* In this section of the memory is reserved by the kernel. */
		if (tcb < (TCB *)(0xFFFFF))
			return true;

		/* Check if it's mapped. */
		if (!Memory::Virtual().Check((void *)tcb))
			return true;

		return false;
	}

	nsa bool Task::RemoveThread(TCB *Thread)
	{
		debug("Thread \"%s\"(%d) removed from process \"%s\"(%d)",
			  Thread->Name, Thread->ID, Thread->Parent->Name,
			  Thread->Parent->ID);

		delete Thread;
		return true;
	}

	nsa bool Task::RemoveProcess(PCB *Process)
	{
		if (unlikely(InvalidPCB(Process)))
			return false;

		if (Process->State == Terminated)
		{
			delete Process;
			return true;
		}

		foreach (TCB *Thread in Process->Threads)
		{
			if (Thread->State == Terminated)
				RemoveThread(Thread);
		}

		return true;
	}

	nsa void Task::UpdateUsage(TaskInfo *Info, TaskExecutionMode Mode, int Core)
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
		{
			if (p->ID == ID)
				return p;
		}
		return nullptr;
	}

	TCB *Task::GetThreadByID(TID ID)
	{
		SmartLock(TaskingLock);
		foreach (auto p in ProcessList)
		{
			foreach (auto t in p->Threads)
			{
				if (t->ID == ID)
					return t;
			}
		}
		return nullptr;
	}

	void Task::WaitForProcess(PCB *pcb)
	{
		if (unlikely(InvalidPCB(pcb)))
			return;

		if (pcb->State == TaskState::UnknownStatus)
			return;

		debug("Waiting for process \"%s\"(%d)",
			  pcb->Name, pcb->ID);

		while (pcb->State != TaskState::Terminated &&
			   pcb->State != TaskState::Zombie &&
			   pcb->State != TaskState::CoreDump)
			this->Yield();
	}

	void Task::WaitForThread(TCB *tcb)
	{
		if (unlikely(InvalidTCB(tcb)))
			return;

		if (tcb->State == TaskState::UnknownStatus)
			return;

		debug("Waiting for thread \"%s\"(%d)",
			  tcb->Name, tcb->ID);

		while (tcb->State != TaskState::Terminated &&
			   tcb->State != TaskState::Zombie &&
			   tcb->State != TaskState::CoreDump)
			this->Yield();
	}

	void Task::WaitForProcessStatus(PCB *pcb, TaskState status)
	{
		if (unlikely(InvalidPCB(pcb)))
			return;

		if (pcb->State == TaskState::UnknownStatus)
			return;

		debug("Waiting for process \"%s\"(%d) to reach status: %d",
			  pcb->Name, pcb->ID, status);

		while (pcb->State != status)
			this->Yield();
	}

	void Task::WaitForThreadStatus(TCB *tcb, TaskState status)
	{
		if (unlikely(InvalidTCB(tcb)))
			return;

		if (tcb->State == TaskState::UnknownStatus)
			return;

		debug("Waiting for thread \"%s\"(%d) to reach status: %d",
			  tcb->Name, tcb->ID, status);

		while (tcb->State != status)
			this->Yield();
	}

	void Task::Sleep(uint64_t Milliseconds, bool NoSwitch)
	{
		TCB *thread = this->GetCurrentThread();
		PCB *process = thread->Parent;

		thread->SetState(TaskState::Sleeping);

		{
			SmartLock(TaskingLock);
			if (process->Threads.size() == 1)
				process->SetState(TaskState::Sleeping);

			thread->Info.SleepUntil =
				TimeManager->CalculateTarget(Milliseconds,
											 Time::Units::Milliseconds);
		}

		// #ifdef DEBUG
		// 		uint64_t TicksNow = TimeManager->GetCounter();
		// #endif
		// 		debug("Thread \"%s\"(%d) is going to sleep until %llu, current %llu, diff %llu",
		// 			  thread->Name, thread->ID, thread->Info.SleepUntil,
		// 			  TicksNow, thread->Info.SleepUntil - TicksNow);

		if (!NoSwitch)
			this->Yield();
	}

	void Task::SignalShutdown()
	{
		debug("Current process is %s(%d) and thread is %s(%d)",
			  GetCurrentProcess()->Name, GetCurrentProcess()->ID,
			  GetCurrentThread()->Name, GetCurrentThread()->ID);

		foreach (auto pcb in ProcessList)
		{
			if (pcb->State == TaskState::Terminated ||
				pcb->State == TaskState::Zombie)
				continue;

			if (pcb == GetCurrentProcess())
				continue;

			debug("Sending SIGTERM to process \"%s\"(%d)",
				  pcb->Name, pcb->ID);
			pcb->SendSignal(SIGTERM);
		}

		// TODO: wait for processes to terminate with timeout.
	}

	__no_sanitize("undefined")
		TCB *Task::CreateThread(PCB *Parent,
								IP EntryPoint,
								const char **argv,
								const char **envp,
								const std::vector<AuxiliaryVector> &auxv,
								TaskArchitecture arch,
								TaskCompatibility Compatibility,
								bool ThreadNotReady)
	{
		SmartLock(TaskingLock);
		return new TCB(this, Parent, EntryPoint,
					   argv, envp, auxv, arch,
					   Compatibility, ThreadNotReady);
	}

	PCB *Task::CreateProcess(PCB *Parent,
							 const char *Name,
							 TaskExecutionMode ExecutionMode,
							 void *Image,
							 bool UseKernelPageTable,
							 uint16_t UserID,
							 uint16_t GroupID)
	{
		SmartLock(TaskingLock);
		return new PCB(this, Parent, Name, ExecutionMode,
					   Image, UseKernelPageTable,
					   UserID, GroupID);
	}

	void Task::StartScheduler()
	{
#if defined(a86)
		if (Interrupts::apicTimer[0])
		{
			((APIC::Timer *)Interrupts::apicTimer[0])->OneShot(CPU::x86::IRQ16, 100);

			/* FIXME: The kernel is not ready for multi-core tasking. */
			return;

			APIC::InterruptCommandRegister icr{};
			bool x2APIC = ((APIC::APIC *)Interrupts::apic[0])->x2APIC;

			if (likely(x2APIC))
			{
				icr.x2.VEC = s_cst(uint8_t, CPU::x86::IRQ16);
				icr.x2.MT = APIC::Fixed;
				icr.x2.L = APIC::Assert;
				icr.x2.DES = 0xFFFFFFFF; /* Broadcast IPI to all local APICs. */
				((APIC::APIC *)Interrupts::apic[0])->ICR(icr);
			}
			else
			{
				icr.VEC = s_cst(uint8_t, CPU::x86::IRQ16);
				icr.MT = APIC::Fixed;
				icr.L = APIC::Assert;

				for (int i = 0; i < SMP::CPUCores; i++)
				{
					icr.DES = uint8_t(i);
					((APIC::APIC *)Interrupts::apic[i])->ICR(icr);
				}
			}
		}
#elif defined(aa64)
#endif
		debug("Tasking Started");
	}

	Task::Task(const IP EntryPoint) : Interrupts::Handler(16) /* IRQ16 */
	{
#if defined(a64)
		// Map the IRQ16 to the first CPU.
		((APIC::APIC *)Interrupts::apic[0])->RedirectIRQ(0, CPU::x86::IRQ16 - CPU::x86::IRQ0, 1);
#elif defined(a32)
#elif defined(aa64)
#endif
		KPrint("Starting tasking instance %#lx with ip: %p (\e666666%s\eCCCCCC)",
			   this, EntryPoint, KernelSymbolTable->GetSymbol(EntryPoint));

#if defined(a64)
		TaskArchitecture Arch = TaskArchitecture::x64;
#elif defined(a32)
		TaskArchitecture Arch = TaskArchitecture::x32;
#elif defined(aa64)
	TaskArchitecture Arch = TaskArchitecture::ARM64;
#endif

		KernelProcess = CreateProcess(nullptr, "Kernel",
									  TaskExecutionMode::Kernel,
									  nullptr, true);
		KernelProcess->PageTable = KernelPageTable;
		KernelProcess->ELFSymbolTable = KernelSymbolTable;
		TCB *kthrd = CreateThread(KernelProcess, EntryPoint,
								  nullptr, nullptr,
								  std::vector<AuxiliaryVector>(), Arch);
		kthrd->Rename("Main Thread");
		debug("Created Kernel Process: %s and Thread: %s",
			  KernelProcess->Name, kthrd->Name);

		bool MONITORSupported = false;
		if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
		{
			CPU::x86::AMD::CPUID0x00000001 cpuid;
			MONITORSupported = cpuid.ECX.MONITOR;
		}
		else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
		{
			CPU::x86::Intel::CPUID0x00000001 cpuid;
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

		IdleProcess = CreateProcess(nullptr, (char *)"Idle",
									TaskExecutionMode::Kernel,
									nullptr, true);
		IdleProcess->ELFSymbolTable = KernelSymbolTable;
		for (int i = 0; i < SMP::CPUCores; i++)
		{
			TCB *thd = CreateThread(IdleProcess, IP(IdleProcessLoop));
			char IdleName[16];
			sprintf(IdleName, "Idle Thread %d", i);
			thd->Rename(IdleName);
			thd->SetPriority(Idle);
			for (int j = 0; j < MAX_CPU; j++)
				thd->Info.Affinity[j] = false;
			thd->Info.Affinity[i] = true;

			if (unlikely(i == 0))
				IdleThread = thd;
		}
		debug("Tasking is ready");
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
					if (Thread == GetCurrentCPU()->CurrentThread.load())
						continue;
					this->KillThread(Thread, KILL_SCHEDULER_DESTRUCTION);
				}

				if (Process == GetCurrentCPU()->CurrentProcess.load())
					continue;
				this->KillProcess(Process, KILL_SCHEDULER_DESTRUCTION);
			}
		}

		debug("Waiting for processes to terminate");
		uint64_t timeout = TimeManager->CalculateTarget(20, Time::Units::Seconds);
		while (ProcessList.size() > 0)
		{
			trace("Waiting for %d processes to terminate", ProcessList.size());
			int NotTerminated = 0;
			foreach (PCB *Process in ProcessList)
			{
				trace("Process %s(%d) is still running (or waiting to be removed state %#lx)",
					  Process->Name, Process->ID, Process->State);

				if (Process->State == TaskState::Terminated)
				{
					debug("Process %s(%d) terminated", Process->Name, Process->ID);
					continue;
				}

				NotTerminated++;
			}
			if (NotTerminated == 1)
				break;

			this->Sleep(1000);
			debug("Current working process is %s(%d)",
				  GetCurrentProcess()->Name, GetCurrentProcess()->ID);

			if (TimeManager->GetCounter() > timeout)
			{
				error("Timeout waiting for processes to terminate");
				break;
			}

			TaskingScheduler_OneShot(100);
		}

		debug("Tasking stopped");
	}
}
