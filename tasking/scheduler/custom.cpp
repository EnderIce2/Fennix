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
#include "../arch/i386/cpu/gdt.hpp"
#elif defined(aa64)
#endif

// #define DEBUG_SCHEDULER 1
// #define DEBUG_GET_NEXT_AVAILABLE_PROCESS 1
// #define DEBUG_GET_NEXT_AVAILABLE_THREAD 1
// #define DEBUG_FIND_NEW_PROCESS 1
// #define DEBUG_SCHEDULER_SEARCH_PROCESS_THREAD 1
// #define DEBUG_WAKE_UP_THREADS 1

/* Global */
#ifdef DEBUG_SCHEDULER

#define DEBUG_GET_NEXT_AVAILABLE_PROCESS 1
#define DEBUG_GET_NEXT_AVAILABLE_THREAD 1
#define DEBUG_FIND_NEW_PROCESS 1
#define DEBUG_SCHEDULER_SEARCH_PROCESS_THREAD 1
#define DEBUG_WAKE_UP_THREADS 1

#define schedbg(m, ...)      \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define schedbg(m, ...)
#endif

/* GetNextAvailableThread */
#ifdef DEBUG_GET_NEXT_AVAILABLE_PROCESS
#define gnap_schedbg(m, ...) \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define gnap_schedbg(m, ...)
#endif

/* GetNextAvailableProcess */
#ifdef DEBUG_GET_NEXT_AVAILABLE_THREAD
#define gnat_schedbg(m, ...) \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define gnat_schedbg(m, ...)
#endif

/* FindNewProcess */
#ifdef DEBUG_FIND_NEW_PROCESS
#define fnp_schedbg(m, ...)  \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define fnp_schedbg(m, ...)
#endif

/* SchedulerSearchProcessThread */
#ifdef DEBUG_SCHEDULER_SEARCH_PROCESS_THREAD
#define sspt_schedbg(m, ...) \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define sspt_schedbg(m, ...)
#endif

/* WakeUpThreads */
#ifdef DEBUG_WAKE_UP_THREADS
#define wut_schedbg(m, ...)  \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define wut_schedbg(m, ...)
#endif

__naked __used nsa void __custom_sched_idle_loop()
{
#if defined(a86)
	asmv("IdleLoop:");
	asmv("hlt");
	asmv("jmp IdleLoop");
#elif defined(aa64)
	asmv("IdleLoop:");
	asmv("wfe");
	asmv("b IdleLoop");
#endif
}

namespace Tasking::Scheduler
{
	bool Custom::RemoveThread(TCB *Thread)
	{
		debug("Thread \"%s\"(%d) removed from process \"%s\"(%d)",
			  Thread->Name, Thread->ID, Thread->Parent->Name,
			  Thread->Parent->ID);

		delete Thread;
		return true;
	}

	bool Custom::RemoveProcess(PCB *Process)
	{
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

	PCB *Custom::GetProcessByID(TID ID)
	{
		foreach (auto p in ProcessList)
		{
			if (p->ID == ID)
				return p;
		}
		return nullptr;
	}

	TCB *Custom::GetThreadByID(TID ID, PCB *Parent)
	{
		foreach (auto t in Parent->Threads)
		{
			if (t->ID == ID)
				return t;
		}
		return nullptr;
	}

	void Custom::StartIdleProcess()
	{
		IdleProcess = ctx->CreateProcess(nullptr, (char *)"Idle",
										 TaskExecutionMode::Kernel, true);
		for (int i = 0; i < SMP::CPUCores; i++)
		{
			TCB *thd = ctx->CreateThread(IdleProcess, IP(__custom_sched_idle_loop));
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
	}

	std::list<PCB *> &Custom::GetProcessList()
	{
		return ProcessList;
	}

	void Custom::StartScheduler()
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
#endif
	}

	void Custom::Yield()
	{
		/* This will trigger the IRQ16
		instantly so we won't execute
		the next instruction */
#if defined(a86)
		asmv("int $0x30");
#elif defined(aa64)
		asmv("svc #0x30");
#endif
	}

	void Custom::PushProcess(PCB *pcb)
	{
		this->ProcessList.push_back(pcb);
	}

	void Custom::PopProcess(PCB *pcb)
	{
		this->ProcessList.remove(pcb);
	}

	std::pair<PCB *, TCB *> Custom::GetIdle()
	{
		return std::make_pair(IdleProcess, IdleThread);
	}

	/* --------------------------------------------------------------- */

	nsa void Custom::OneShot(int TimeSlice)
	{
		if (TimeSlice == 0)
			TimeSlice = Tasking::TaskPriority::Normal;

#ifdef DEBUG
		if (DebuggerIsAttached)
			TimeSlice += 10;
#endif

#if defined(a86)
		((APIC::Timer *)Interrupts::apicTimer[GetCurrentCPU()->ID])->OneShot(CPU::x86::IRQ16, TimeSlice);
#elif defined(aa64)
#endif
	}

	nsa void Custom::UpdateUsage(TaskInfo *Info, TaskExecutionMode Mode, int Core)
	{
		UNUSED(Core);
		uint64_t CurrentTime = TimeManager->GetCounter();
		uint64_t TimePassed = Info->LastUpdateTime - CurrentTime;
		Info->LastUpdateTime = CurrentTime;

		if (Mode == TaskExecutionMode::User)
			Info->UserTime += TimePassed;
		else
			Info->KernelTime += TimePassed;
	}

	nsa NIF bool Custom::FindNewProcess(void *CPUDataPointer)
	{
		CPUData *CurrentCPU = (CPUData *)CPUDataPointer;
		fnp_schedbg("%d processes", ProcessList.size());
#ifdef DEBUG_FIND_NEW_PROCESS
		foreach (auto process in ProcessList)
			fnp_schedbg("Process %d %s", process->ID,
						process->Name);
#endif
		foreach (auto process in ProcessList)
		{
			switch (process->State.load())
			{
			case TaskState::Ready:
				fnp_schedbg("Ready process (%s)%d",
							process->Name, process->ID);
				break;
			default:
				fnp_schedbg("Process \"%s\"(%d) status %d",
							process->Name, process->ID,
							process->State);

				/* We don't actually remove the process. RemoveProcess
				   firstly checks if it's terminated, if not, it will
				   loop through Threads and call RemoveThread on
				   terminated threads. */
				RemoveProcess(process);
				continue;
			}

			foreach (auto thread in process->Threads)
			{
				if (thread->State.load() != TaskState::Ready)
					continue;

				if (thread->Info.Affinity[CurrentCPU->ID] == false)
					continue;

				CurrentCPU->CurrentProcess = process;
				CurrentCPU->CurrentThread = thread;
				return true;
			}
		}
		fnp_schedbg("No process to run.");
		return false;
	}

	nsa NIF bool Custom::GetNextAvailableThread(void *CPUDataPointer)
	{
		CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

		size_t ThreadsSize = CurrentCPU->CurrentProcess->Threads.size();

		for (size_t i = 0; i < ThreadsSize; i++)
		{
			if (CurrentCPU->CurrentProcess->Threads[i] == CurrentCPU->CurrentThread.load())
			{
				size_t TempIndex = i;
			RetryAnotherThread:
				if (TempIndex + 1 >= ThreadsSize)
					break;

				TCB *nextThread = CurrentCPU->CurrentProcess->Threads[TempIndex + 1];

				gnat_schedbg("\"%s\"(%d) and next thread is \"%s\"(%d)",
							 CurrentCPU->CurrentProcess->Threads[i]->Name,
							 CurrentCPU->CurrentProcess->Threads[i]->ID,
							 nextThread->Name, nextThread->ID);

				if (nextThread->State.load() != TaskState::Ready)
				{
					gnat_schedbg("Thread %d is not ready", nextThread->ID);
					TempIndex++;
					goto RetryAnotherThread;
				}

				if (nextThread->Info.Affinity[CurrentCPU->ID] == false)
					continue;

				CurrentCPU->CurrentThread = nextThread;
				gnat_schedbg("[thd 0 -> end] Scheduling thread %d parent of %s->%d Procs %d",
							 nextThread->ID, nextThread->Parent->Name,
							 ThreadsSize, ProcessList.size());
				return true;
			}
#ifdef DEBUG
			else
			{
				gnat_schedbg("Thread %d is not the current one",
							 CurrentCPU->CurrentProcess->Threads[i]->ID);
			}
#endif
		}
		return false;
	}

	nsa NIF bool Custom::GetNextAvailableProcess(void *CPUDataPointer)
	{
		CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

		bool Skip = true;
		foreach (auto process in ProcessList)
		{
			if (process == CurrentCPU->CurrentProcess.load())
			{
				Skip = false;
				gnap_schedbg("Found current process %#lx", process);
				continue;
			}

			if (Skip)
			{
				gnap_schedbg("Skipping process %#lx", process);
				continue;
			}

			if (process->State.load() != TaskState::Ready)
			{
				gnap_schedbg("Process %d is not ready", process->ID);
				continue;
			}

			foreach (auto thread in process->Threads)
			{
				if (thread->State.load() != TaskState::Ready)
				{
					gnap_schedbg("Thread %d is not ready", thread->ID);
					continue;
				}

				if (thread->Info.Affinity[CurrentCPU->ID] == false)
					continue;

				CurrentCPU->CurrentProcess = process;
				CurrentCPU->CurrentThread = thread;
				gnap_schedbg("[cur proc+1 -> first thd] Scheduling thread %d %s->%d (Total Procs %d)",
							 thread->ID, thread->Name, process->Threads.size(), ProcessList.size());
				return true;
			}
		}
		gnap_schedbg("No process to run.");
		return false;
	}

	nsa NIF bool Custom::SchedulerSearchProcessThread(void *CPUDataPointer)
	{
		CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

		foreach (auto process in ProcessList)
		{
			if (process->State.load() != TaskState::Ready)
			{
				sspt_schedbg("Process %d is not ready", process->ID);
				continue;
			}

			foreach (auto thread in process->Threads)
			{
				if (thread->State.load() != TaskState::Ready)
				{
					sspt_schedbg("Thread %d is not ready", thread->ID);
					continue;
				}

				if (thread->Info.Affinity[CurrentCPU->ID] == false)
					continue;

				CurrentCPU->CurrentProcess = process;
				CurrentCPU->CurrentThread = thread;
				sspt_schedbg("[proc 0 -> end -> first thd] Scheduling thread %d parent of %s->%d (Procs %d)",
							 thread->ID, thread->Parent->Name, process->Threads.size(), ProcessList.size());
				return true;
			}
		}
		return false;
	}

	nsa NIF void Custom::UpdateProcessState()
	{
		foreach (auto process in ProcessList)
		{
			if (process->State.load() == TaskState::Terminated)
				continue;

			if (process->Threads.size() == 1)
			{
				process->State.exchange(process->Threads.front()->State.load());
				continue;
			}

			bool AllThreadsSleeping = true;
			foreach (auto thread in process->Threads)
			{
				if (thread->State.load() == TaskState::Terminated)
					continue;

				if (thread->State.load() != TaskState::Sleeping)
				{
					AllThreadsSleeping = false;
					break;
				}
			}

			if (AllThreadsSleeping)
				process->State.store(TaskState::Sleeping);
			else if (process->State.load() == TaskState::Sleeping)
				process->State.store(TaskState::Ready);
		}
	}

	nsa NIF void Custom::WakeUpThreads()
	{
		foreach (auto process in ProcessList)
		{
			Tasking::TaskState pState = process->State.load();
			if (pState != TaskState::Ready &&
				pState != TaskState::Sleeping &&
				pState != TaskState::Blocked)
				continue;

			foreach (auto thread in process->Threads)
			{
				if (likely(thread->State.load() != TaskState::Sleeping))
					continue;

				/* Check if the thread is ready to wake up. */
				if (unlikely(thread->Info.SleepUntil < TimeManager->GetCounter()))
				{
					if (pState == TaskState::Sleeping)
						process->State.store(TaskState::Ready);
					thread->State.store(TaskState::Ready);

					thread->Info.SleepUntil = 0;
					wut_schedbg("Thread \"%s\"(%d) woke up.", thread->Name, thread->ID);
				}
				else
				{
					wut_schedbg("Thread \"%s\"(%d) is not ready to wake up. (SleepUntil: %d, Counter: %d)",
								thread->Name, thread->ID, thread->Info.SleepUntil, TimeManager->GetCounter());
				}
			}
		}
	}

	nsa NIF void Custom::CleanupTerminated()
	{
		foreach (auto pcb in ProcessList)
		{
			if (pcb->State.load() == TaskState::Terminated)
			{
				delete pcb;
				continue;
			}

			foreach (TCB *tcb in pcb->Threads)
			{
				if (tcb->State == Terminated)
					delete tcb;
			}
		}
	}

	nsa NIF void Custom::Schedule(CPU::TrapFrame *Frame)
	{
		if (unlikely(StopScheduler))
		{
			warn("Scheduler stopped.");
			return;
		}
		bool ProcessNotChanged = false;
		/* Restore kernel page table for safety reasons. */
		if (!SchedulerUpdateTrapFrame)
			KernelPageTable->Update();
		uint64_t SchedTmpTicks = TimeManager->GetCounter();
		this->LastTaskTicks.store(size_t(SchedTmpTicks - this->SchedulerTicks.load()));
		CPUData *CurrentCPU = GetCurrentCPU();
		this->LastCore.store(CurrentCPU->ID);
		schedbg("Scheduler called on CPU %d.", CurrentCPU->ID);

		if (unlikely(!CurrentCPU->CurrentProcess.load() ||
					 !CurrentCPU->CurrentThread.load()))
		{
			schedbg("Invalid process or thread. Finding a new one.");
			ProcessNotChanged = true;
			if (this->FindNewProcess(CurrentCPU))
				goto Success;
			else
				goto Idle;
		}
		else
		{
			CurrentCPU->CurrentThread->Registers = *Frame;
			CPU::x64::fxsave(&CurrentCPU->CurrentThread->FPU);
#ifdef a64
			CurrentCPU->CurrentThread->ShadowGSBase = CPU::x64::rdmsr(CPU::x64::MSR_SHADOW_GS_BASE);
			CurrentCPU->CurrentThread->GSBase = CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
			CurrentCPU->CurrentThread->FSBase = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);
#else
			CurrentCPU->CurrentThread->ShadowGSBase = uintptr_t(CPU::x32::rdmsr(CPU::x32::MSR_SHADOW_GS_BASE));
			CurrentCPU->CurrentThread->GSBase = uintptr_t(CPU::x32::rdmsr(CPU::x32::MSR_GS_BASE));
			CurrentCPU->CurrentThread->FSBase = uintptr_t(CPU::x32::rdmsr(CPU::x32::MSR_FS_BASE));
#endif

			if (CurrentCPU->CurrentProcess->State.load() == TaskState::Running)
				CurrentCPU->CurrentProcess->State.store(TaskState::Ready);
			if (CurrentCPU->CurrentThread->State.load() == TaskState::Running)
				CurrentCPU->CurrentThread->State.store(TaskState::Ready);

			this->CleanupTerminated();
			schedbg("Passed CleanupTerminated");

			this->UpdateProcessState();
			schedbg("Passed UpdateProcessState");

			this->WakeUpThreads();
			schedbg("Passed WakeUpThreads");

			if (this->SchedulerUpdateTrapFrame)
			{
				debug("Updating trap frame");
				this->SchedulerUpdateTrapFrame = false;
				CurrentCPU->CurrentProcess->State.store(TaskState::Running);
				CurrentCPU->CurrentThread->State.store(TaskState::Running);
				*Frame = CurrentCPU->CurrentThread->Registers;
				this->SchedulerTicks.store(size_t(TimeManager->GetCounter() - SchedTmpTicks));
				return;
			}

			if (this->GetNextAvailableThread(CurrentCPU))
			{
				ProcessNotChanged = true;
				goto Success;
			}
			schedbg("Passed GetNextAvailableThread");

			if (this->GetNextAvailableProcess(CurrentCPU))
			{
				goto Success;
			}
			schedbg("Passed GetNextAvailableProcess");

			if (SchedulerSearchProcessThread(CurrentCPU))
			{
				schedbg("Passed SchedulerSearchProcessThread");
				goto Success;
			}
			else
			{
				schedbg("SchedulerSearchProcessThread failed. Going idle.");
				goto Idle;
			}
		}

		assert(!"Unwanted code execution");

	Idle:
		ProcessNotChanged = true;
		CurrentCPU->CurrentProcess = IdleProcess;
		CurrentCPU->CurrentThread = IdleThread;

	Success:
		schedbg("Process \"%s\"(%d) Thread \"%s\"(%d) is now running on CPU %d",
				CurrentCPU->CurrentProcess->Name, CurrentCPU->CurrentProcess->ID,
				CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID, CurrentCPU->ID);

		if (!ProcessNotChanged)
			UpdateUsage(&CurrentCPU->CurrentProcess->Info,
						CurrentCPU->CurrentProcess->Security.ExecutionMode,
						CurrentCPU->ID);

		UpdateUsage(&CurrentCPU->CurrentThread->Info,
					CurrentCPU->CurrentThread->Security.ExecutionMode,
					CurrentCPU->ID);

		CurrentCPU->CurrentProcess->State.store(TaskState::Running);
		CurrentCPU->CurrentThread->State.store(TaskState::Running);

		*Frame = CurrentCPU->CurrentThread->Registers;

#ifdef a64
		GlobalDescriptorTable::SetKernelStack((void *)((uintptr_t)CurrentCPU->CurrentThread->Stack->GetStackTop()));
		CPU::x64::fxrstor(&CurrentCPU->CurrentThread->FPU);
		CPU::x64::wrmsr(CPU::x64::MSR_SHADOW_GS_BASE, CurrentCPU->CurrentThread->ShadowGSBase);
		CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, CurrentCPU->CurrentThread->GSBase);
		CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, CurrentCPU->CurrentThread->FSBase);
#else
		GlobalDescriptorTable::SetKernelStack((void *)((uintptr_t)CurrentCPU->CurrentThread->Stack->GetStackTop()));
		CPU::x32::fxrstor(&CurrentCPU->CurrentThread->FPU);
		CPU::x32::wrmsr(CPU::x32::MSR_SHADOW_GS_BASE, CurrentCPU->CurrentThread->ShadowGSBase);
		CPU::x32::wrmsr(CPU::x32::MSR_GS_BASE, CurrentCPU->CurrentThread->GSBase);
		CPU::x32::wrmsr(CPU::x32::MSR_FS_BASE, CurrentCPU->CurrentThread->FSBase);
#endif

		CurrentCPU->CurrentProcess->Signals.HandleSignal(Frame, CurrentCPU->CurrentThread.load());

		if (!ProcessNotChanged)
			(&CurrentCPU->CurrentProcess->Info)->LastUpdateTime = TimeManager->GetCounter();
		(&CurrentCPU->CurrentThread->Info)->LastUpdateTime = TimeManager->GetCounter();
		this->OneShot(CurrentCPU->CurrentThread->Info.Priority);

		if (CurrentCPU->CurrentThread->Security.IsDebugEnabled &&
			CurrentCPU->CurrentThread->Security.IsKernelDebugEnabled)
		{
#ifdef a64
			trace("%s[%ld]: RIP=%#lx  RBP=%#lx  RSP=%#lx",
				  CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
				  CurrentCPU->CurrentThread->Registers.rip,
				  CurrentCPU->CurrentThread->Registers.rbp,
				  CurrentCPU->CurrentThread->Registers.rsp);
#else
			trace("%s[%ld]: EIP=%#lx  EBP=%#lx  ESP=%#lx",
				  CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
				  CurrentCPU->CurrentThread->Registers.eip,
				  CurrentCPU->CurrentThread->Registers.ebp,
				  CurrentCPU->CurrentThread->Registers.esp);
#endif
		}

		this->SchedulerTicks.store(size_t(TimeManager->GetCounter() - SchedTmpTicks));
		CurrentCPU->CurrentProcess->PageTable->Update();
	}

	nsa NIF void Custom::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		SmartCriticalSection(SchedulerLock);
		this->Schedule(Frame);
	}

	Custom::Custom(Task *ctx) : Base(ctx), Interrupts::Handler(16) /* IRQ16 */
	{
#if defined(a86)
		// Map the IRQ16 to the first CPU.
		((APIC::APIC *)Interrupts::apic[0])->RedirectIRQ(0, CPU::x86::IRQ16 - CPU::x86::IRQ0, 1);
#endif
	}

	Custom::~Custom()
	{
		foreach (PCB *Process in ProcessList)
		{
			foreach (TCB *Thread in Process->Threads)
			{
				if (Thread == GetCurrentCPU()->CurrentThread.load())
					continue;
				ctx->KillThread(Thread, KILL_SCHEDULER_DESTRUCTION);
			}

			if (Process == GetCurrentCPU()->CurrentProcess.load())
				continue;
			ctx->KillProcess(Process, KILL_SCHEDULER_DESTRUCTION);
		}

		debug("Waiting for processes to terminate");
		uint64_t timeout = TimeManager->CalculateTarget(20, Time::Units::Seconds);
		while (this->GetProcessList().size() > 0)
		{
			trace("Waiting for %d processes to terminate", this->GetProcessList().size());
			int NotTerminated = 0;
			foreach (PCB *Process in this->GetProcessList())
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

			ctx->Sleep(1000);
			debug("Current working process is %s(%d)",
				  ctx->GetCurrentProcess()->Name,
				  ctx->GetCurrentProcess()->ID);

			if (TimeManager->GetCounter() > timeout)
			{
				error("Timeout waiting for processes to terminate");
				break;
			}

			this->OneShot(100);
		}
	}
}
