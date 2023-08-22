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

#ifndef __FENNIX_KERNEL_TASKING_H__
#define __FENNIX_KERNEL_TASKING_H__

#include <types.h>

#include <filesystem.hpp>
#include <symbols.hpp>
#include <memory.hpp>
#include <ints.hpp>
#include <ipc.hpp>
#include <debug.h>
#include <vector>
#include <atomic>
#include <abi.h>

namespace Tasking
{
	using VirtualFileSystem::FileDescriptorTable;
	using VirtualFileSystem::Node;

	/** Instruction Pointer */
	typedef __UINTPTR_TYPE__ IP;
	/** Process ID */
	typedef int PID;
	/** Thread ID */
	typedef int TID;

	enum TaskArchitecture
	{
		UnknownArchitecture,
		x32,
		x64,
		ARM32,
		ARM64,

		_ArchitectureMin = UnknownArchitecture,
		_ArchitectureMax = ARM64
	};

	enum TaskCompatibility
	{
		UnknownPlatform,
		Native,
		Linux,
		Windows,

		_CompatibilityMin = UnknownPlatform,
		_CompatibilityMax = Windows
	};

	enum TaskExecutionMode
	{
		UnknownExecutionMode,
		Kernel,
		System,
		User,

		_ExecuteModeMin = UnknownExecutionMode,
		_ExecuteModeMax = User
	};

	enum TaskStatus : int
	{
		UnknownStatus,
		Ready,
		Running,
		Sleeping,
		Blocked,
		Zombie,
		Terminated,

		_StatusMin = UnknownStatus,
		_StatusMax = Terminated
	};

	enum TaskPriority
	{
		UnknownPriority = 0,
		Idle = 1,
		Low = 2,
		Normal = 5,
		High = 8,
		Critical = 10,

		_PriorityMin = UnknownPriority,
		_PriorityMax = Critical
	};

	enum KillErrorCodes : int
	{
		KILL_SCHEDULER_DESTRUCTION = -0xFFFF,
		KILL_CXXABI_EXCEPTION = -0xECE97,
		KILL_BY_OTHER_PROCESS = -0x7A55,
		KILL_SYSCALL = -0xCA11,
		KILL_CRASH = -0xDEAD,
		KILL_OOM = -0x1008,
		KILL_ERROR = -0x1,
		KILL_SUCCESS = 0,
	};

	struct TaskInfo
	{
		uint64_t OldUserTime = 0;
		uint64_t OldKernelTime = 0;

		uint64_t SleepUntil = 0;
		uint64_t KernelTime = 0, UserTime = 0, SpawnTime = 0, LastUpdateTime = 0;
		uint64_t Year = 0, Month = 0, Day = 0, Hour = 0, Minute = 0, Second = 0;
		bool Affinity[256] = {true}; // MAX_CPU
		TaskPriority Priority = TaskPriority::Normal;
		TaskArchitecture Architecture = TaskArchitecture::UnknownArchitecture;
		TaskCompatibility Compatibility = TaskCompatibility::UnknownPlatform;
	};

	/**
	 * TCB struct for gs register
	 */
	struct gsTCB
	{
		/**
		 * Used by syscall handler
		 *
		 * gs+0x0
		 */
		uintptr_t SyscallStack = __UINTPTR_MAX__;

		/**
		 * Used by syscall handler
		 *
		 * gs+0x8
		 */
		uintptr_t TempStack = __UINTPTR_MAX__;

		/**
		 * The current thread class
		 */
		class TCB *t;
	};

	class TCB
	{
	private:
		class Task *ctx = nullptr;

		void SetupUserStack_x86_64(const char **argv,
								   const char **envp,
								   const std::vector<AuxiliaryVector> &auxv);

		void SetupUserStack_x86_32(const char **argv,
								   const char **envp,
								   const std::vector<AuxiliaryVector> &auxv);

		void SetupUserStack_aarch64(const char **argv,
									const char **envp,
									const std::vector<AuxiliaryVector> &auxv);

	public:
		/* Basic info */
		TID ID = -1;
		const char *Name = nullptr;
		class PCB *Parent = nullptr;
		IP EntryPoint = 0;

		/* Statuses */
		std::atomic_int ExitCode;
		std::atomic<TaskStatus> Status = TaskStatus::Zombie;
		int ErrorNumber;

		/* Memory */
		Memory::MemMgr *Memory;
		Memory::StackGuard *Stack;

		/* CPU state */
#if defined(a64)
		CPU::x64::TrapFrame Registers{};
		uintptr_t ShadowGSBase, GSBase, FSBase;
#elif defined(a32)
		CPU::x32::TrapFrame Registers; // TODO
		uintptr_t ShadowGSBase, GSBase, FSBase;
#elif defined(aa64)
		uintptr_t Registers; // TODO
#endif
		uintptr_t IPHistory[128];
		CPU::x64::FXState *FPU;

		/* Info & Security info */
		struct
		{
			TaskExecutionMode ExecutionMode = UnknownExecutionMode;
			bool IsCritical = false;
			bool IsDebugEnabled = false;
			bool IsKernelDebugEnabled = false;
		} Security{};
		TaskInfo Info{};

		/* Compatibility structures */
		struct
		{
			int *set_child_tid{};
			int *clear_child_tid{};
		} Linux{};

		void Rename(const char *name);
		void SetPriority(TaskPriority priority);
		int GetExitCode() { return ExitCode.load(); }
		void SetCritical(bool Critical);
		void SetDebugMode(bool Enable);
		void SetKernelDebugMode(bool Enable);

		void Block() { Status.store(TaskStatus::Blocked); }
		void Unblock() { Status.store(TaskStatus::Ready); }

		void SYSV_ABI_Call(uintptr_t Arg1 = 0,
						   uintptr_t Arg2 = 0,
						   uintptr_t Arg3 = 0,
						   uintptr_t Arg4 = 0,
						   uintptr_t Arg5 = 0,
						   uintptr_t Arg6 = 0,
						   void *Function = nullptr);

		TCB(class Task *ctx,
			PCB *Parent,
			IP EntryPoint,
			const char **argv = nullptr,
			const char **envp = nullptr,
			const std::vector<AuxiliaryVector> &auxv = std::vector<AuxiliaryVector>(),
			TaskArchitecture Architecture = TaskArchitecture::x64,
			TaskCompatibility Compatibility = TaskCompatibility::Native,
			bool ThreadNotReady = false);

		~TCB();
	};

	class PCB
	{
	private:
		class Task *ctx = nullptr;
		bool OwnPageTable = false;

	public:
		/* Basic info */
		PID ID = -1;
		const char *Name = nullptr;
		PCB *Parent = nullptr;

		/* Statuses */
		std::atomic_int ExitCode;
		std::atomic<TaskStatus> Status = Zombie;

		/* Info & Security info */
		struct
		{
			TaskExecutionMode ExecutionMode = UnknownExecutionMode;
			bool IsCritical = false;
			bool IsDebugEnabled = false;
			bool IsKernelDebugEnabled = false;
			struct
			{
				uint16_t UserID = UINT16_MAX;
				uint16_t GroupID = UINT16_MAX;
			} Real, Effective;
		} Security{};
		TaskInfo Info{};

		/* Filesystem */
		Node *CurrentWorkingDirectory;
		Node *ProcessDirectory;
		Node *memDirectory;
		FileDescriptorTable *FileDescriptors;

		/* Memory */
		Memory::PageTable *PageTable;
		Memory::MemMgr *Memory;
		Memory::ProgramBreak *ProgramBreak;

		/* Other */
		InterProcessCommunication::IPC *IPC;
		SymbolResolver::Symbols *ELFSymbolTable;

		/* Threads & Children */
		std::vector<TCB *> Threads;
		std::vector<PCB *> Children;

	public:
		void Rename(const char *name);
		void SetWorkingDirectory(Node *node);

		PCB(class Task *ctx,
			PCB *Parent,
			const char *Name,
			TaskExecutionMode ExecutionMode,
			void *Image = nullptr,
			bool DoNotCreatePageTable = false,
			uint16_t UserID = -1,
			uint16_t GroupID = -1);

		~PCB();
	};

	class Task : public Interrupts::Handler
	{
	private:
		NewLock(SchedulerLock);
		NewLock(TaskingLock);

		PID NextPID = 0;
		TID NextTID = 0;

		std::vector<PCB *> ProcessList;
		PCB *IdleProcess = nullptr;
		TCB *IdleThread = nullptr;
		TCB *CleanupThread = nullptr;
		std::atomic_size_t SchedulerTicks = 0;
		std::atomic_size_t LastTaskTicks = 0;
		std::atomic_int LastCore = 0;
		std::atomic_bool StopScheduler = false;
		std::atomic_bool SchedulerUpdateTrapFrame = false;

		bool InvalidPCB(PCB *pcb);
		bool InvalidTCB(TCB *tcb);

		/**
		 * @note This function is NOT thread safe
		 */
		void RemoveThread(TCB *tcb);

		/**
		 * @note This function is NOT thread safe
		 */
		void RemoveProcess(PCB *pcb);

		void UpdateUsage(TaskInfo *Info,
						 TaskExecutionMode Mode,
						 int Core);

		/**
		 * @note This function is NOT thread safe
		 */
		bool FindNewProcess(void *CPUDataPointer);

		/**
		 * @note This function is NOT thread safe
		 */
		bool GetNextAvailableThread(void *CPUDataPointer);

		/**
		 * @note This function is NOT thread safe
		 */
		bool GetNextAvailableProcess(void *CPUDataPointer);

		/**
		 * @note This function is NOT thread safe
		 */
		bool SchedulerSearchProcessThread(void *CPUDataPointer);

		/**
		 * @note This function is NOT thread safe
		 */
		void UpdateProcessStatus();

		/**
		 * @note This function is NOT thread safe
		 */
		void WakeUpThreads();

#if defined(a64)
		/**
		 * @note This function is NOT thread safe
		 */
		void Schedule(CPU::x64::TrapFrame *Frame);

		void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(a32)
		/**
		 * @note This function is NOT thread safe
		 */
		void Schedule(void *Frame);

		void OnInterruptReceived(CPU::x32::TrapFrame *Frame);
#elif defined(aa64)
		/**
		 * @note This function is NOT thread safe
		 */
		void Schedule(CPU::aarch64::TrapFrame *Frame);

		void OnInterruptReceived(CPU::aarch64::TrapFrame *Frame);
#endif

	public:
		void SetCleanupThread(TCB *Thread) { CleanupThread = Thread; }
		size_t GetSchedulerTicks() { return SchedulerTicks.load(); }
		size_t GetLastTaskTicks() { return LastTaskTicks.load(); }
		int GetLastCore() { return LastCore.load(); }
		std::vector<PCB *> GetProcessList() { return ProcessList; }
		void CleanupProcessesThread();
		void Panic() { StopScheduler = true; }
		bool IsPanic() { return StopScheduler; }

		/**
		 * Yield the current thread and switch
		 * to another thread if available
		 */
		__always_inline inline void Yield()
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

		/**
		 * Update the current thread's trap frame
		 * without switching to another thread
		 */
		__always_inline inline void UpdateFrame()
		{
			SchedulerUpdateTrapFrame = true;
			Yield();
		}

		void SignalShutdown();

		void KillThread(TCB *tcb, enum KillErrorCodes Code)
		{
			tcb->Status = TaskStatus::Terminated;
			tcb->ExitCode = (int)Code;
			debug("Killing thread %s(%d) with exit code %d",
				  tcb->Name, tcb->ID, Code);
		}

		void KillProcess(PCB *pcb, enum KillErrorCodes Code)
		{
			pcb->Status = TaskStatus::Terminated;
			pcb->ExitCode = (int)Code;
			debug("Killing process %s(%d) with exit code %d",
				  pcb->Name, pcb->ID, Code);
		}

		/**
		 * Get the Current Process object
		 * @return PCB*
		 */
		PCB *GetCurrentProcess();

		/**
		 * Get the Current Thread object
		 * @return TCB*
		 */
		TCB *GetCurrentThread();

		PCB *GetProcessByID(PID ID);

		TCB *GetThreadByID(TID ID);

		/** Wait for process to terminate */
		void WaitForProcess(PCB *pcb);

		/** Wait for thread to terminate */
		void WaitForThread(TCB *tcb);

		void WaitForProcessStatus(PCB *pcb, TaskStatus Status);
		void WaitForThreadStatus(TCB *tcb, TaskStatus Status);

		/**
		 * Sleep for a given amount of milliseconds
		 *
		 * @param Milliseconds Amount of milliseconds to sleep
		 */
		void Sleep(uint64_t Milliseconds, bool NoSwitch = false);

		PCB *CreateProcess(PCB *Parent,
						   const char *Name,
						   TaskExecutionMode TrustLevel,
						   void *Image = nullptr,
						   bool DoNotCreatePageTable = false,
						   uint16_t UserID = UINT16_MAX,
						   uint16_t GroupID = UINT16_MAX);

		TCB *CreateThread(PCB *Parent,
						  IP EntryPoint,
						  const char **argv = nullptr,
						  const char **envp = nullptr,
						  const std::vector<AuxiliaryVector> &auxv = std::vector<AuxiliaryVector>(),
						  TaskArchitecture Architecture = TaskArchitecture::x64,
						  TaskCompatibility Compatibility = TaskCompatibility::Native,
						  bool ThreadNotReady = false);

		Task(const IP EntryPoint);
		~Task();

		friend PCB;
		friend TCB;
	};
}

#define thisProcess TaskManager->GetCurrentProcess()
#define thisThread TaskManager->GetCurrentThread()

#define PEXIT(Code) thisProcess->ExitCode = Code
#define TEXIT(Code) thisThread->ExitCode = Code

extern "C" void TaskingScheduler_OneShot(int TimeSlice);

#endif // !__FENNIX_KERNEL_TASKING_H__
