#ifndef __FENNIX_KERNEL_TASKING_H__
#define __FENNIX_KERNEL_TASKING_H__

#include <types.h>

#include <filesystem.hpp>
#include <ints.hpp>
#include <symbols.hpp>
#include <vector.hpp>
#include <memory.hpp>
#include <atomic.hpp>
#include <ipc.hpp>
#include <debug.h>
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
        uint64_t SleepUntil = 0;
        uint64_t SpawnTime = 0;
        uint64_t OldUserTime = 0, CurrentUserTime = 0;
        uint64_t OldKernelTime = 0, CurrentKernelTime = 0;
        uint64_t KernelTime = 0, UserTime = 0;
        uint64_t Year, Month, Day, Hour, Minute, Second;
        uint64_t Usage[256]; // MAX_CPU
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
#if defined(__amd64__)
        CPU::x64::TrapFrame Registers;
        uint64_t GSBase, FSBase;
#elif defined(__i386__)
        CPU::x32::TrapFrame Registers; // TODO
        uint64_t GSBase, FSBase;
#elif defined(__aarch64__)
        uint64_t Registers; // TODO
#endif
        uintptr_t IPHistory[128];
        TaskSecurity Security;
        TaskInfo Info;
        CPU::x64::FXState *FPU;

        void Rename(const char *name)
        {
            CriticalSection cs;
            if (!Name[0])
            {
                warn("Tried to rename thread %d to NULL", ID);
                return;
            }
            trace("Renaming thread %s to %s", Name, name);
            for (int i = 0; i < 256; i++)
            {
                Name[i] = name[i];
                if (name[i] == '\0')
                    break;
            }
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
        Vector<TCB *> Threads;
        Vector<PCB *> Children;
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

        Vector<TokenData> Tokens;

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

        Vector<PCB *> ListProcess;
        PCB *IdleProcess = nullptr;
        TCB *IdleThread = nullptr;
        Atomic<uint64_t> SchedulerTicks = 0;
        Atomic<uint64_t> LastTaskTicks = 0;

        bool InvalidPCB(PCB *pcb);
        bool InvalidTCB(TCB *tcb);

        void RemoveThread(TCB *tcb);
        void RemoveProcess(PCB *pcb);

        void UpdateUserTime(TaskInfo *Info);
        void UpdateKernelTime(TaskInfo *Info);
        void UpdateUsage(TaskInfo *Info, int Core);

        bool FindNewProcess(void *CPUDataPointer);
        bool GetNextAvailableThread(void *CPUDataPointer);
        bool GetNextAvailableProcess(void *CPUDataPointer);
        void SchedulerCleanupProcesses();
        bool SchedulerSearchProcessThread(void *CPUDataPointer);
        void UpdateProcessStatus();
        void WakeUpThreads(void *CPUDataPointer);

#if defined(__amd64__)
        void Schedule(CPU::x64::TrapFrame *Frame);
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(__i386__)
        void Schedule(void *Frame);
        void OnInterruptReceived(void *Frame);
#elif defined(__aarch64__)
        void Schedule(void *Frame);
        void OnInterruptReceived(void *Frame);
#endif
        bool StopScheduler = false;

    public:
        uint64_t GetSchedulerTicks() { return SchedulerTicks.Load(); }
        uint64_t GetLastTaskTicks() { return LastTaskTicks.Load(); }
        Vector<PCB *> GetProcessList() { return ListProcess; }
        Security *GetSecurityManager() { return &SecurityManager; }
        void Panic() { StopScheduler = true; }
        void Schedule();
        void SignalShutdown();
        void RevertProcessCreation(PCB *Process);
        void RevertThreadCreation(TCB *Thread);

        long GetUsage(int Core)
        {
            if (IdleProcess)
                return 100 - IdleProcess->Info.Usage[Core];
            else
                return 0;
        }

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
                          const Vector<AuxiliaryVector> &auxv = Vector<AuxiliaryVector>(),
                          IPOffset Offset = 0,
                          TaskArchitecture Architecture = TaskArchitecture::x64,
                          TaskCompatibility Compatibility = TaskCompatibility::Native);

        Task(const IP EntryPoint);
        ~Task();
    };
}

extern "C" void TaskingScheduler_OneShot(int TimeSlice);

#endif // !__FENNIX_KERNEL_TASKING_H__
