#ifndef __FENNIX_KERNEL_TASKING_H__
#define __FENNIX_KERNEL_TASKING_H__

#include <types.h>

#include <interrupts.hpp>
#include <vector.hpp>
#include <memory.hpp>
#include <hashmap.hpp>
#include <ipc.hpp>
#include <debug.h>
#include <abi.h>

namespace Tasking
{
    typedef unsigned long IP;
    typedef unsigned long IPOffset;
    typedef unsigned long UPID;
    typedef unsigned long UTID;
    typedef unsigned long Token;

    enum TaskArchitecture
    {
        UnknownArchitecture,
        x32,
        x64,
        ARM,
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
        Idle,
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

    struct TaskSecurity
    {
        TaskTrustLevel TrustLevel;
        Token UniqueToken;
        bool IsCritical;
    };

    struct TaskInfo
    {
        uint64_t SpawnTime = 0;
        uint64_t OldUserTime = 0, CurrentUserTime = 0;
        uint64_t OldKernelTime = 0, CurrentKernelTime = 0;
        uint64_t KernelTime = 0, UserTime = 0;
        uint64_t Year, Month, Day, Hour, Minute, Second;
        uint64_t Usage[256]; // MAX_CPU
        bool Affinity[256];  // MAX_CPU
        int Priority;
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
        TaskStatus Status;
#if defined(__amd64__)
        CPU::x64::TrapFrame Registers;
        uint64_t GSBase, FSBase;
#elif defined(__i386__)
        uint32_t Registers; // TODO
#elif defined(__aarch64__)
        uint64_t Registers; // TODO
#endif
        TaskSecurity Security;
        TaskInfo Info;
        char FXRegion[512] __attribute__((aligned(16)));

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

        void SetPriority(int priority)
        {
            CriticalSection cs;
            trace("Setting priority of thread %s to %d", Name, priority);
            Info.Priority = priority;
        }

        int GetExitCode() { return ExitCode; }

        void SetCritical(bool critical)
        {
            CriticalSection cs;
            trace("Setting criticality of thread %s to %s", Name, critical ? "true" : "false");
            Security.IsCritical = critical;
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
        HashMap<InterProcessCommunication::IPCPort, uint64_t> *IPCHandles;
        Memory::PageTable4 *PageTable;
    };

    enum TokenTrustLevel
    {
        UnknownTrustLevel,
        Untrusted,
        Trusted,
        TrustedByKernel
    };

    class Security
    {
    public:
        Token CreateToken();
        bool TrustToken(Token token,
                        TokenTrustLevel TrustLevel);
        bool UntrustToken(Token token);
        bool DestroyToken(Token token);
        Security();
        ~Security();
    };

    class Task : public Interrupts::Handler
    {
    private:
        Security SecurityManager;
        InterProcessCommunication::IPC *IPCManager = nullptr;
        UPID NextPID = 0;
        UTID NextTID = 0;

        Vector<PCB *> ListProcess;
        PCB *IdleProcess = nullptr;
        TCB *IdleThread = nullptr;

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
        void InitIPC()
        {
            static int once = 0;
            if (!once++)
                this->IPCManager = new InterProcessCommunication::IPC();
        }
        Vector<PCB *> GetProcessList() { return ListProcess; }
        void Panic() { StopScheduler = true; }
        void Schedule();
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

        /** @brief Wait for process to terminate */
        void WaitForProcess(PCB *pcb);

        /** @brief Wait for thread to terminate */
        void WaitForThread(TCB *tcb);

        PCB *CreateProcess(PCB *Parent,
                           const char *Name,
                           TaskTrustLevel TrustLevel);

        TCB *CreateThread(PCB *Parent,
                          IP EntryPoint,
                          const char **argv,
                          const char **envp,
                          Vector<AuxiliaryVector> &auxv,
                          IPOffset Offset = 0,
                          TaskArchitecture Architecture = TaskArchitecture::x64,
                          TaskCompatibility Compatibility = TaskCompatibility::Native);

        Task(const IP EntryPoint);
        ~Task();
    };
}

#endif // !__FENNIX_KERNEL_TASKING_H__
