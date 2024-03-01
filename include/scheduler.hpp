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

#pragma once

#include <task.hpp>
#include <lock.hpp>

namespace Tasking::Scheduler
{
	class Base
	{
	public:
		Task *ctx = nullptr;
		std::atomic_size_t SchedulerTicks = 0;
		std::atomic_size_t LastTaskTicks = 0;
		std::atomic_int LastCore = 0;
		std::atomic_bool StopScheduler = false;
		std::atomic_bool SchedulerUpdateTrapFrame = false;

		/**
		 * Remove a thread from the scheduler
		 *
		 * @note This function is NOT thread safe
		 * @note This function does not check if
		 * the thread is valid nor if it has
		 * Terminated status
		 */
		virtual bool RemoveThread(TCB *tcb)
		{
			assert(!"RemoveThread not implemented");
		};

		/**
		 * @note This function is NOT thread safe
		 */
		virtual bool RemoveProcess(PCB *pcb)
		{
			assert(!"RemoveProcess not implemented");
		}

		virtual PCB *GetProcessByID(TID ID)
		{
			assert(!"GetProcessByID not implemented");
		}

		virtual TCB *GetThreadByID(TID ID)
		{
			assert(!"GetThreadByID not implemented");
		}

		virtual std::list<PCB *> &GetProcessList()
		{
			assert(!"GetProcessList not implemented");
		}

		virtual void StartIdleProcess()
		{
			assert(!"StartIdleProcess not implemented");
		}

		virtual void StartScheduler()
		{
			assert(!"StartScheduler not implemented");
		}

		virtual void Yield()
		{
			assert(!"Yield not implemented");
		}

		virtual void PushProcess(PCB *pcb)
		{
			assert(!"PushProcess not implemented");
		}

		virtual void PopProcess(PCB *pcb)
		{
			assert(!"PopProcess not implemented");
		}

		virtual std::pair<PCB *, TCB *> GetIdle()
		{
			assert(!"GetIdle not implemented");
		}

		Base(Task *_ctx)
			: ctx(_ctx) {}

		~Base() {}
	};

	class Custom : public Base,
				   public Interrupts::Handler
	{
	private:
		NewLock(SchedulerLock);

	public:
		std::list<PCB *> ProcessList;

		PCB *IdleProcess = nullptr;
		TCB *IdleThread = nullptr;

		bool RemoveThread(TCB *tcb) final;
		bool RemoveProcess(PCB *pcb) final;
		PCB *GetProcessByID(TID ID) final;
		TCB *GetThreadByID(TID ID) final;
		std::list<PCB *> &GetProcessList() final;
		void StartIdleProcess() final;
		void StartScheduler() final;
		void Yield() final;
		void PushProcess(PCB *pcb) final;
		void PopProcess(PCB *pcb) final;
		std::pair<PCB *, TCB *> GetIdle() final;

		void OneShot(int TimeSlice);

		void UpdateUsage(TaskInfo *Info,
						 TaskExecutionMode Mode,
						 int Core);

		bool FindNewProcess(void *CPUDataPointer);
		bool GetNextAvailableThread(void *CPUDataPointer);
		bool GetNextAvailableProcess(void *CPUDataPointer);
		bool SchedulerSearchProcessThread(void *CPUDataPointer);
		void UpdateProcessState();
		void WakeUpThreads();
		void CleanupTerminated();

		void Schedule(CPU::TrapFrame *Frame);
		void OnInterruptReceived(CPU::TrapFrame *Frame) final;

		Custom(Task *ctx);
		virtual ~Custom();
	};

	class RoundRobin : public Base,
					   public Interrupts::Handler
	{
	};
}
