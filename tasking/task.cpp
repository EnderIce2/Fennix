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

#include <scheduler.hpp>
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
		return ((Scheduler::Base *)Scheduler)->GetProcessByID(ID);
	}

	TCB *Task::GetThreadByID(TID ID, PCB *Parent)
	{
		return ((Scheduler::Base *)Scheduler)->GetThreadByID(ID, Parent);
	}

	std::vector<PCB *> Task::GetProcessList()
	{
		return ((Scheduler::Base *)Scheduler)->GetProcessList();
	}

	void Task::Panic()
	{
		((Scheduler::Base *)Scheduler)->StopScheduler.store(true);
	}

	bool Task::IsPanic()
	{
		return ((Scheduler::Base *)Scheduler)->StopScheduler;
	}

	void Task::Yield()
	{
		((Scheduler::Base *)Scheduler)->Yield();
	}

	void Task::UpdateFrame()
	{
		((Scheduler::Base *)Scheduler)->SchedulerUpdateTrapFrame = true;
		((Scheduler::Base *)Scheduler)->Yield();
	}

	void Task::PushProcess(PCB *pcb)
	{
		((Scheduler::Base *)Scheduler)->PushProcess(pcb);
	}

	void Task::PopProcess(PCB *pcb)
	{
		((Scheduler::Base *)Scheduler)->PopProcess(pcb);
	}

	void Task::WaitForProcess(PCB *pcb)
	{
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
		if (pcb->State == TaskState::UnknownStatus)
			return;

		debug("Waiting for process \"%s\"(%d) to reach status: %d",
			  pcb->Name, pcb->ID, status);

		while (pcb->State != status)
			this->Yield();
	}

	void Task::WaitForThreadStatus(TCB *tcb, TaskState status)
	{
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

		foreach (auto pcb in((Scheduler::Base *)Scheduler)->GetProcessList())
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
		TCB *Task::CreateThread(PCB *Parent, IP EntryPoint,
								const char **argv, const char **envp,
								const std::vector<AuxiliaryVector> &auxv,
								TaskArchitecture arch, TaskCompatibility Compatibility,
								bool ThreadNotReady)
	{
		SmartLock(TaskingLock);
		return new TCB(this, Parent, EntryPoint,
					   argv, envp, auxv, arch,
					   Compatibility, ThreadNotReady);
	}

	PCB *Task::CreateProcess(PCB *Parent, const char *Name,
							 TaskExecutionMode ExecutionMode, bool UseKernelPageTable,
							 uint16_t UserID, uint16_t GroupID)
	{
		SmartLock(TaskingLock);
		return new PCB(this, Parent, Name, ExecutionMode,
					   UseKernelPageTable, UserID, GroupID);
	}

	void Task::StartScheduler()
	{
		((Scheduler::Base *)Scheduler)->StartScheduler();
		debug("Tasking Started");
	}

	Task::Task(const IP EntryPoint)
	{
		/* I don't know if this is the best way to do this. */
		Scheduler::Custom *custom_sched = new Scheduler::Custom(this);
		Scheduler::Base *sched = r_cst(Scheduler::Base *, custom_sched);
		__sched_ctx = custom_sched;
		Scheduler = sched;

		KernelProcess = CreateProcess(nullptr, "Kernel",
									  TaskExecutionMode::Kernel, true);
		KernelProcess->PageTable = KernelPageTable;
		TCB *kthrd = CreateThread(KernelProcess, EntryPoint,
								  nullptr, nullptr,
								  std::vector<AuxiliaryVector>(), GetKArch());
		kthrd->Rename("Main Thread");
		debug("Created Kernel Process: %s and Thread: %s",
			  KernelProcess->Name, kthrd->Name);

		if (!CPU::Interrupts(CPU::Check))
		{
			error("Interrupts are not enabled.");
			CPU::Interrupts(CPU::Enable);
		}

		((Scheduler::Base *)Scheduler)->StartIdleProcess();
		debug("Tasking is ready");
	}

	Task::~Task()
	{
		delete (Scheduler::Custom *)__sched_ctx;
	}
}
