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
	/** @brief Instruction Pointer */
	typedef __UINTPTR_TYPE__ IP;
	/** @brief Process ID */
	typedef int PID;
	/** @brief Thread ID */
	typedef int TID;
	/* @brief Token */
	typedef __UINTPTR_TYPE__ Token;

	enum TaskArchitecture
	{
		UnknownArchitecture,
		x32,
		x64,
		ARM32,
		ARM64
	};

	enum TaskCompatibility
	{
		UnknownPlatform,
		Native,
		Linux,
		Windows
	};

	enum TaskTrustLevel
	{
		UnknownElevation,
		Kernel,
		System,
		User
	};

	enum TaskStatus
	{
		UnknownStatus,
		Ready,
		Running,
		Sleeping,
		Waiting,
		Stopped,
		Terminated
	};

	enum TaskPriority
	{
		UnknownPriority = 0,
		Idle = 1,
		Low = 2,
		Normal = 5,
		High = 8,
		Critical = 10
	};

	enum KillErrorCodes : int
	{
		KILL_SCHEDULER_DESTRUCTION = -0xFFFF,
		KILL_CXXABI_EXCEPTION = -0xECE97,
		KILL_SYSCALL = -0xCA11,
		KILL_OOM = -0x1008,
		KILL_ERROR = -0x1,
		KILL_SUCCESS = 0,
	};

	struct TaskSecurity
	{
		TaskTrustLevel TrustLevel;
		Token UniqueToken;
		bool IsCritical;
		bool IsDebugEnabled;
		bool IsKernelDebugEnabled;
	};

	struct TaskInfo
	{
		size_t OldUserTime = 0;
		size_t OldKernelTime = 0;

		size_t SleepUntil = 0;
		size_t KernelTime = 0, UserTime = 0, SpawnTime = 0, LastUpdateTime = 0;
		uint64_t Year, Month, Day, Hour, Minute, Second;
		bool Affinity[256]; // MAX_CPU
		TaskPriority Priority;
		TaskArchitecture Architecture;
		TaskCompatibility Compatibility;
	};

	struct TCB
	{
		/** @brief Used by syscall handler */
		uintptr_t SyscallStack; /* gs+0x0 */

		/** @brief Used by syscall handler */
		uintptr_t TempStack; /* gs+0x8 */

		TID ID;
		char Name[256];
		struct PCB *Parent;
		IP EntryPoint;
		int ExitCode;
		Memory::StackGuard *Stack;
		Memory::MemMgr *Memory;
		TaskStatus Status;
#if defined(a64)
		CPU::x64::TrapFrame Registers;
		uintptr_t ShadowGSBase, GSBase, FSBase;
#elif defined(a32)
		CPU::x32::TrapFrame Registers; // TODO
		uintptr_t ShadowGSBase, GSBase, FSBase;
#elif defined(aa64)
		uintptr_t Registers; // TODO
#endif
		uintptr_t IPHistory[128];
		TaskSecurity Security;
		TaskInfo Info;
		CPU::x64::FXState *FPU;

		void Rename(const char *name)
		{
			CriticalSection cs;
			if (strlen(name) > 256 || strlen(name) == 0)
			{
				debug("Invalid thread name");
				return;
			}

			trace("Renaming thread %s to %s", Name, name);
			strncpy(Name, name, 256);
		}

		void SetPriority(TaskPriority priority)
		{
			CriticalSection cs;
			trace("Setting priority of thread %s to %d", Name, priority);
			Info.Priority = priority;
		}

		int GetExitCode() { return ExitCode; }

		void SetCritical(bool Critical)
		{
			CriticalSection cs;
			trace("Setting criticality of thread %s to %s", Name, Critical ? "true" : "false");
			Security.IsCritical = Critical;
		}

		void SetDebugMode(bool Enable)
		{
			CriticalSection cs;
			trace("Setting debug mode of thread %s to %s", Name, Enable ? "true" : "false");
			Security.IsDebugEnabled = Enable;
		}

		void SetKernelDebugMode(bool Enable)
		{
			CriticalSection cs;
			trace("Setting kernel debug mode of thread %s to %s", Name, Enable ? "true" : "false");
			Security.IsKernelDebugEnabled = Enable;
		}

		void SYSV_ABI_Call(uintptr_t Arg1 = 0,
						   uintptr_t Arg2 = 0,
						   uintptr_t Arg3 = 0,
						   uintptr_t Arg4 = 0,
						   uintptr_t Arg5 = 0,
						   uintptr_t Arg6 = 0,
						   void *Function = nullptr)
		{
			CriticalSection cs;
#if defined(a64)
			this->Registers.rdi = Arg1;
			this->Registers.rsi = Arg2;
			this->Registers.rdx = Arg3;
			this->Registers.rcx = Arg4;
			this->Registers.r8 = Arg5;
			this->Registers.r9 = Arg6;
			if (Function != nullptr)
				this->Registers.rip = (uint64_t)Function;
#else
#warning "SYSV ABI not implemented for this architecture"
#endif
		}
	};

	struct PCB
	{
		PID ID;
		char Name[256];
		PCB *Parent;
		int ExitCode;
		TaskStatus Status;
		TaskSecurity Security;
		TaskInfo Info;
		std::vector<TCB *> Threads;
		std::vector<PCB *> Children;
		InterProcessCommunication::IPC *IPC;
		Memory::PageTable *PageTable;
		SymbolResolver::Symbols *ELFSymbolTable;
		VirtualFileSystem::Node *CurrentWorkingDirectory;
		VirtualFileSystem::Node *ProcessDirectory;
		VirtualFileSystem::Node *memDirectory;

		void SetWorkingDirectory(VirtualFileSystem::Node *node)
		{
			CriticalSection cs;
			trace("Setting working directory of process %s to %#lx (%s)", Name, node, node->Name);
			CurrentWorkingDirectory = node;
		}
	};

	/** @brief Token Trust Level */
	enum TTL
	{
		UnknownTrustLevel = 0b0001,
		Untrusted = 0b0010,
		Trusted = 0b0100,
		TrustedByKernel = 0b1000,
		FullTrust = Trusted | TrustedByKernel,
		AllFlags = 0b1111
	};

	class Security
	{
	private:
		struct TokenData
		{
			Token token;
			int TrustLevel;
			uint64_t OwnerID;
			bool Process;
		};

		std::vector<TokenData> Tokens;

	public:
		Token CreateToken();
		bool TrustToken(Token token, TTL TrustLevel);
		bool AddTrustLevel(Token token, TTL TrustLevel);
		bool RemoveTrustLevel(Token token, TTL TrustLevel);
		bool UntrustToken(Token token);
		bool DestroyToken(Token token);
		bool IsTokenTrusted(Token token, TTL TrustLevel);
		bool IsTokenTrusted(Token token, int TrustLevel);
		int GetTokenTrustLevel(Token token);
		Security();
		~Security();
	};

	class Task : public Interrupts::Handler
	{
	private:
		Security SecurityManager;
		PID NextPID = 0;
		TID NextTID = 0;

		std::vector<PCB *> ProcessList;
		PCB *IdleProcess = nullptr;
		TCB *IdleThread = nullptr;
		TCB *CleanupThread = nullptr;
		std::atomic_size_t SchedulerTicks = 0;
		std::atomic_size_t LastTaskTicks = 0;
		std::atomic_int LastCore = 0;
		bool StopScheduler = false;
		bool InvalidPCB(PCB *pcb);
		bool InvalidTCB(TCB *tcb);

		void RemoveThread(TCB *tcb);
		void RemoveProcess(PCB *pcb);

		void UpdateUsage(TaskInfo *Info, TaskSecurity *Security, int Core);

		bool FindNewProcess(void *CPUDataPointer);
		bool GetNextAvailableThread(void *CPUDataPointer);
		bool GetNextAvailableProcess(void *CPUDataPointer);
		bool SchedulerSearchProcessThread(void *CPUDataPointer);
		void UpdateProcessStatus();
		void WakeUpThreads();

#if defined(a64)
		void Schedule(CPU::x64::TrapFrame *Frame);
		void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(a32)
		void Schedule(void *Frame);
		void OnInterruptReceived(CPU::x32::TrapFrame *Frame);
#elif defined(aa64)
		void Schedule(CPU::aarch64::TrapFrame *Frame);
		void OnInterruptReceived(CPU::aarch64::TrapFrame *Frame);
#endif

	public:
		void SetCleanupThread(TCB *Thread) { CleanupThread = Thread; }
		size_t GetSchedulerTicks() { return SchedulerTicks.load(); }
		size_t GetLastTaskTicks() { return LastTaskTicks.load(); }
		int GetLastCore() { return LastCore.load(); }
		std::vector<PCB *> GetProcessList() { return ProcessList; }
		Security *GetSecurityManager() { return &SecurityManager; }
		void CleanupProcessesThread();
		void Panic() { StopScheduler = true; }
		bool IsPanic() { return StopScheduler; }
		__always_inline inline void Schedule()
		{
#if defined(a86)
			asmv("int $0x30"); /* This will trigger the IRQ16 instantly so we won't execute the next instruction */
#elif defined(aa64)
			asmv("svc #0x30"); /* This will trigger the IRQ16 instantly so we won't execute the next instruction */
#endif
		}
		void SignalShutdown();
		void RevertProcessCreation(PCB *Process);
		void RevertThreadCreation(TCB *Thread);

		void KillThread(TCB *tcb, enum KillErrorCodes Code)
		{
			tcb->Status = TaskStatus::Terminated;
			tcb->ExitCode = (int)Code;
		}

		void KillProcess(PCB *pcb, enum KillErrorCodes Code)
		{
			pcb->Status = TaskStatus::Terminated;
			pcb->ExitCode = (int)Code;
		}

		/**
		 * @brief Get the Current Process object
		 * @return PCB*
		 */
		PCB *GetCurrentProcess();

		/**
		 * @brief Get the Current Thread object
		 * @return TCB*
		 */
		TCB *GetCurrentThread();

		PCB *GetProcessByID(PID ID);

		TCB *GetThreadByID(TID ID);

		/** @brief Wait for process to terminate */
		void WaitForProcess(PCB *pcb);

		/** @brief Wait for thread to terminate */
		void WaitForThread(TCB *tcb);

		void WaitForProcessStatus(PCB *pcb, TaskStatus Status);
		void WaitForThreadStatus(TCB *tcb, TaskStatus Status);

		/**
		 * @brief Sleep for a given amount of milliseconds
		 *
		 * @param Milliseconds Amount of milliseconds to sleep
		 */
		void Sleep(uint64_t Milliseconds, bool NoSwitch = false);

		PCB *CreateProcess(PCB *Parent,
						   const char *Name,
						   TaskTrustLevel TrustLevel,
						   void *Image = nullptr,
						   bool DoNotCreatePageTable = false);

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
	};
}

#define PEXIT(Code) TaskManager->GetCurrentProcess()->ExitCode = Code
#define TEXIT(Code) TaskManager->GetCurrentThread()->ExitCode = Code

extern "C" void TaskingScheduler_OneShot(int TimeSlice);

#endif // !__FENNIX_KERNEL_TASKING_H__
