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
    typedef unsigned long IP;
    typedef __UINTPTR_TYPE__ IPOffset;
    typedef unsigned long UPID;
    typedef unsigned long UTID;
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
        uint64_t OldUserTime = 0;
        uint64_t OldKernelTime = 0;

        uint64_t SleepUntil = 0;
        uint64_t KernelTime = 0, UserTime = 0, SpawnTime = 0, LastUpdateTime = 0;
        uint64_t Year, Month, Day, Hour, Minute, Second;
        bool Affinity[256];  // MAX_CPU
        TaskPriority Priority;
        TaskArchitecture Architecture;
        TaskCompatibility Compatibility;
    };

    struct TCB
    {
        UTID ID;
        char Name[256];
        struct PCB *Parent;
        IP EntryPoint;
        IPOffset Offset;
        int ExitCode;
        Memory::StackGuard *Stack;
        Memory::MemMgr *Memory;
        TaskStatus Status;
#if defined(a64)
        CPU::x64::TrapFrame Registers;
        uint64_t GSBase, FSBase;
#elif defined(a32)
        CPU::x32::TrapFrame Registers; // TODO
        uint64_t GSBase, FSBase;
#elif defined(aa64)
        uint64_t Registers; // TODO
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
    };

    struct PCB
    {
        UPID ID;
        char Name[256];
        PCB *Parent;
        int ExitCode;
        TaskStatus Status;
        TaskSecurity Security;
        TaskInfo Info;
        std::vector<TCB *> Threads;
        std::vector<PCB *> Children;
        InterProcessCommunication::IPC *IPC;
        Memory::PageTable4 *PageTable;
        SymbolResolver::Symbols *ELFSymbolTable;
        VirtualFileSystem::Node *ProcessDirectory;
        VirtualFileSystem::Node *memDirectory;
    };

    /** @brief Token Trust Level */
    enum TTL
    {
        UnknownTrustLevel = 0b0001,
        Untrusted = 0b0010,
        Trusted = 0b0100,
        TrustedByKernel = 0b1000,
        FullTrust = Trusted | TrustedByKernel
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
        UPID NextPID = 0;
        UTID NextTID = 0;

        std::vector<PCB *> ProcessList;
        PCB *IdleProcess = nullptr;
        TCB *IdleThread = nullptr;
        TCB *CleanupThread = nullptr;
        std::atomic_uint64_t SchedulerTicks = 0;
        std::atomic_uint64_t LastTaskTicks = 0;
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
        uint64_t GetSchedulerTicks() { return SchedulerTicks.load(); }
        uint64_t GetLastTaskTicks() { return LastTaskTicks.load(); }
        std::vector<PCB *> GetProcessList() { return ProcessList; }
        Security *GetSecurityManager() { return &SecurityManager; }
        void CleanupProcessesThread();
        void Panic() { StopScheduler = true; }
        void Schedule();
        void SignalShutdown();
        void RevertProcessCreation(PCB *Process);
        void RevertThreadCreation(TCB *Thread);

        void KillThread(TCB *tcb, int Code)
        {
            tcb->Status = TaskStatus::Terminated;
            tcb->ExitCode = Code;
        }

        void KillProcess(PCB *pcb, int Code)
        {
            pcb->Status = TaskStatus::Terminated;
            pcb->ExitCode = Code;
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

        PCB *GetProcessByID(UPID ID);

        TCB *GetThreadByID(UTID ID);

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
        void Sleep(uint64_t Milliseconds);

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
                          IPOffset Offset = 0,
                          TaskArchitecture Architecture = TaskArchitecture::x64,
                          TaskCompatibility Compatibility = TaskCompatibility::Native);

        Task(const IP EntryPoint);
        ~Task();
    };
}

#define PEXIT(Code) TaskManager->GetCurrentProcess()->ExitCode = Code
#define TEXIT(Code) TaskManager->GetCurrentThread()->ExitCode = Code

extern "C" void TaskingScheduler_OneShot(int TimeSlice);

#endif // !__FENNIX_KERNEL_TASKING_H__
