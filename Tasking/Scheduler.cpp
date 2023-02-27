#include <task.hpp>

#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>
#include <io.h>

#include "../kernel.h"

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#include "../Architecture/i686/cpu/apic.hpp"
#elif defined(__aarch64__)
#endif

NewLock(SchedulerLock);

/* FIXME: On screen task manager is corrupting the stack... */
// #define ON_SCREEN_SCHEDULER_TASK_MANAGER 1

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

extern "C" SafeFunction __no_instrument_function void TaskingScheduler_OneShot(int TimeSlice)
{
    if (TimeSlice == 0)
        TimeSlice = 10;
#if defined(__amd64__)
    ((APIC::Timer *)Interrupts::apicTimer[GetCurrentCPU()->ID])->OneShot(CPU::x86::IRQ16, TimeSlice);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
}

namespace Tasking
{
#if defined(__amd64__)
    SafeFunction __no_instrument_function bool Task::FindNewProcess(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;
        fnp_schedbg("%d processes", ListProcess.size());
#ifdef DEBUG_FIND_NEW_PROCESS
        foreach (auto pcb in ListProcess)
            fnp_schedbg("Process %d %s", pcb->ID, pcb->Name);
#endif
        foreach (auto pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;

            switch (pcb->Status)
            {
            case TaskStatus::Ready:
                fnp_schedbg("Ready process (%s)%d", pcb->Name, pcb->ID);
                break;
            default:
                fnp_schedbg("Process \"%s\"(%d) status %d", pcb->Name, pcb->ID, pcb->Status);
                /* We don't actually remove the process. RemoveProcess
                   firstly checks if it's terminated, if not, it will
                   loop through Threads and call RemoveThread on
                   terminated threads. */
                RemoveProcess(pcb);
                continue;
            }

            foreach (auto tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                    continue;

                if (tcb->Status != TaskStatus::Ready)
                    continue;

                if (tcb->Info.Affinity[CurrentCPU->ID] == false)
                    continue;

                CurrentCPU->CurrentProcess = pcb;
                CurrentCPU->CurrentThread = tcb;
                return true;
            }
        }
        fnp_schedbg("No process to run.");
        return false;
    }

    SafeFunction __no_instrument_function bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        for (size_t i = 0; i < CurrentCPU->CurrentProcess->Threads.size(); i++)
        {
            if (CurrentCPU->CurrentProcess->Threads[i] == CurrentCPU->CurrentThread.Load())
            {
                size_t TempIndex = i;
            RetryAnotherThread:
                TCB *thread = CurrentCPU->CurrentProcess->Threads[TempIndex + 1];
                if (unlikely(InvalidTCB(thread)))
                {
                    if (TempIndex > CurrentCPU->CurrentProcess->Threads.size())
                        break;
                    TempIndex++;
                    gnat_schedbg("Thread %#lx is invalid", thread);
                    goto RetryAnotherThread;
                }

                gnat_schedbg("\"%s\"(%d) and next thread is \"%s\"(%d)", CurrentCPU->CurrentProcess->Threads[i]->Name, CurrentCPU->CurrentProcess->Threads[i]->ID, thread->Name, thread->ID);

                if (thread->Status != TaskStatus::Ready)
                {
                    gnat_schedbg("Thread %d is not ready", thread->ID);
                    TempIndex++;
                    goto RetryAnotherThread;
                }

                if (thread->Info.Affinity[CurrentCPU->ID] == false)
                    continue;

                CurrentCPU->CurrentThread = thread;
                gnat_schedbg("[thd 0 -> end] Scheduling thread %d parent of %s->%d Procs %d", thread->ID, thread->Parent->Name, CurrentCPU->CurrentProcess->Threads.size(), ListProcess.size());
                return true;
            }
#ifdef DEBUG
            else
            {
                gnat_schedbg("Thread %d is not the current one", CurrentCPU->CurrentProcess->Threads[i]->ID);
            }
#endif
        }
        return false;
    }

    SafeFunction __no_instrument_function bool Task::GetNextAvailableProcess(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        bool Skip = true;
        foreach (auto pcb in ListProcess)
        {
            if (pcb == CurrentCPU->CurrentProcess.Load())
            {
                Skip = false;
                gnap_schedbg("Found current process %#lx", pcb);
                continue;
            }

            if (Skip)
            {
                gnap_schedbg("Skipping process %#lx", pcb);
                continue;
            }

            if (InvalidPCB(pcb))
            {
                gnap_schedbg("Invalid process %#lx", pcb);
                continue;
            }

            if (pcb->Status != TaskStatus::Ready)
            {
                gnap_schedbg("Process %d is not ready", pcb->ID);
                continue;
            }

            foreach (auto tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                {
                    gnap_schedbg("Invalid thread %#lx", tcb);
                    continue;
                }

                if (tcb->Status != TaskStatus::Ready)
                {
                    gnap_schedbg("Thread %d is not ready", tcb->ID);
                    continue;
                }

                if (tcb->Info.Affinity[CurrentCPU->ID] == false)
                    continue;

                CurrentCPU->CurrentProcess = pcb;
                CurrentCPU->CurrentThread = tcb;
                gnap_schedbg("[cur proc+1 -> first thd] Scheduling thread %d %s->%d (Total Procs %d)", tcb->ID, tcb->Name, pcb->Threads.size(), ListProcess.size());
                return true;
            }
        }
        gnap_schedbg("No process to run.");
        return false;
    }

    SafeFunction __no_instrument_function void Task::SchedulerCleanupProcesses()
    {
        foreach (auto pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;
            RemoveProcess(pcb);
        }
    }

    SafeFunction __no_instrument_function bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        foreach (auto pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
            {
                sspt_schedbg("Invalid process %#lx", pcb);
                continue;
            }

            if (pcb->Status != TaskStatus::Ready)
            {
                sspt_schedbg("Process %d is not ready", pcb->ID);
                continue;
            }

            foreach (auto tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                {
                    sspt_schedbg("Invalid thread %#lx", tcb);
                    continue;
                }

                if (tcb->Status != TaskStatus::Ready)
                {
                    sspt_schedbg("Thread %d is not ready", tcb->ID);
                    continue;
                }

                if (tcb->Info.Affinity[CurrentCPU->ID] == false)
                    continue;

                CurrentCPU->CurrentProcess = pcb;
                CurrentCPU->CurrentThread = tcb;
                sspt_schedbg("[proc 0 -> end -> first thd] Scheduling thread %d parent of %s->%d (Procs %d)", tcb->ID, tcb->Parent->Name, pcb->Threads.size(), ListProcess.size());
                return true;
            }
        }
        return false;
    }

    SafeFunction __no_instrument_function void Task::UpdateProcessStatus()
    {
        foreach (auto pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;

            if (pcb->Status == TaskStatus::Terminated ||
                pcb->Status == TaskStatus::Stopped)
                continue;

            bool AllThreadsSleeping = true;
            foreach (auto tcb in pcb->Threads)
            {
                if (tcb->Status != TaskStatus::Sleeping)
                {
                    AllThreadsSleeping = false;
                    break;
                }
            }

            if (AllThreadsSleeping)
                pcb->Status = TaskStatus::Sleeping;
            else if (pcb->Status == TaskStatus::Sleeping)
                pcb->Status = TaskStatus::Ready;
        }
    }

    SafeFunction __no_instrument_function void Task::WakeUpThreads(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;
        foreach (auto pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;

            if (pcb->Status == TaskStatus::Terminated ||
                pcb->Status == TaskStatus::Stopped)
                continue;

            foreach (auto tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                    continue;

                if (tcb->Status != TaskStatus::Sleeping)
                    continue;

                /* Check if the thread is ready to wake up. */
                if (tcb->Info.SleepUntil < TimeManager->GetCounter())
                {
                    if (pcb->Status == TaskStatus::Sleeping)
                        pcb->Status = TaskStatus::Ready;
                    tcb->Status = TaskStatus::Ready;

                    tcb->Info.SleepUntil = 0;
                    wut_schedbg("Thread \"%s\"(%d) woke up.", tcb->Name, tcb->ID);
                }
                else
                {
                    wut_schedbg("Thread \"%s\"(%d) is not ready to wake up. (SleepUntil: %d, Counter: %d)", tcb->Name, tcb->ID, tcb->Info.SleepUntil, TimeManager->GetCounter());
                }
            }
        }
    }

#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
    int SuccessSource = 0;
    int sanity;

    const char *Statuses[] = {
        "FF0000", /* Unknown */
        "AAFF00", /* Ready */
        "00AA00", /* Running */
        "FFAA00", /* Sleeping */
        "FFAA00", /* Waiting */
        "FF0088", /* Stopped */
        "FF0000", /* Terminated */
    };

    const char *StatusesSign[] = {
        "Unknown",
        "Ready",
        "Run",
        "Sleep",
        "Wait",
        "Stop",
        "Terminated",
    };

    const char *SuccessSourceStrings[] = {
        "Unknown",
        "GetNextAvailableThread",
        "GetNextAvailableProcess",
        "SchedulerSearchProcessThread",
    };

    SafeFunction __no_instrument_function void OnScreenTaskManagerUpdate()
    {
        TimeManager->Sleep(100);
        Video::ScreenBuffer *sb = Display->GetBuffer(0);
        for (short i = 0; i < 340; i++)
        {
            for (short j = 0; j < 200; j++)
            {
                uint32_t *Pixel = (uint32_t *)((uintptr_t)sb->Buffer + (j * sb->Width + i) * (bInfo->Framebuffer[0].BitsPerPixel / 8));
                *Pixel = 0x222222;
            }
        }

        uint32_t tmpX, tmpY;
        Display->GetBufferCursor(0, &tmpX, &tmpY);
        Display->SetBufferCursor(0, 0, 0);
        printf("\eF02C21Task Manager\n");
        foreach (auto Proc in TaskManager->GetProcessList())
        {
            int Status = Proc->Status;
            printf("\e%s-> \eAABBCC%s \e00AAAA%s\n",
                   Statuses[Status], Proc->Name, StatusesSign[Status]);

            foreach (auto Thd in Proc->Threads)
            {
                Status = Thd->Status;
                printf("  \e%s-> \eAABBCC%s \e00AAAA%s\n\eAABBCC",
                       Statuses[Status], Thd->Name, StatusesSign[Status]);
            }
        }
        register uintptr_t CurrentStackAddress asm("rsp");
        printf("Sanity: %d, Stack: %#lx\nSched. Source: %s", sanity++, CurrentStackAddress, SuccessSourceStrings[SuccessSource]);
        if (sanity > 1000)
            sanity = 0;
        Display->SetBufferCursor(0, tmpX, tmpY);
        Display->SetBuffer(0);
        TimeManager->Sleep(100);
    }
#endif

    SafeFunction __no_instrument_function void Task::Schedule(CPU::x64::TrapFrame *Frame)
    {
        SmartCriticalSection(SchedulerLock);
        if (StopScheduler)
        {
            warn("Scheduler stopped.");
            return;
        }
        CPU::x64::writecr3({.raw = (uint64_t)KernelPageTable}); /* Restore kernel page table for safety reasons. */
        uint64_t SchedTmpTicks = CPU::Counter();
        this->LastTaskTicks.Store(SchedTmpTicks - this->SchedulerTicks.Load());
        CPUData *CurrentCPU = GetCurrentCPU();
        schedbg("Scheduler called on CPU %d.", CurrentCPU->ID);
        schedbg("%d: %ld%%", CurrentCPU->ID, GetUsage(CurrentCPU->ID));

#ifdef DEBUG_SCHEDULER
        {
            schedbg("================================================================");
            schedbg("Status: 0-ukn | 1-rdy | 2-run | 3-wait | 4-term");
            schedbg("Technical Informations on regs %#lx", Frame->InterruptNumber);
            size_t ds;
            asmv("mov %%ds, %0"
                 : "=r"(ds));
            schedbg("FS=%#lx  GS=%#lx  SS=%#lx  CS=%#lx  DS=%#lx",
                    CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                    Frame->ss, Frame->cs, ds);
            schedbg("R8=%#lx  R9=%#lx  R10=%#lx  R11=%#lx",
                    Frame->r8, Frame->r9, Frame->r10, Frame->r11);
            schedbg("R12=%#lx  R13=%#lx  R14=%#lx  R15=%#lx",
                    Frame->r12, Frame->r13, Frame->r14, Frame->r15);
            schedbg("RAX=%#lx  RBX=%#lx  RCX=%#lx  RDX=%#lx",
                    Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
            schedbg("RSI=%#lx  RDI=%#lx  RBP=%#lx  RSP=%#lx",
                    Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
            schedbg("RIP=%#lx  RFL=%#lx  INT=%#lx  ERR=%#lx",
                    Frame->rip, Frame->rflags, Frame->InterruptNumber, Frame->ErrorCode);
            schedbg("================================================================");
        }
#endif

        if (unlikely(InvalidPCB(CurrentCPU->CurrentProcess.Load()) || InvalidTCB(CurrentCPU->CurrentThread.Load())))
        {
            schedbg("Invalid process or thread. Finding a new one.");
            if (this->FindNewProcess(CurrentCPU))
                goto Success;
            else
                goto Idle;
        }
        else
        {
            CurrentCPU->CurrentThread->Registers = *Frame;
            CPU::x64::fxsave(CurrentCPU->CurrentThread->FPU);
            CurrentCPU->CurrentThread->GSBase = CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
            CurrentCPU->CurrentThread->FSBase = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);

            if (CurrentCPU->CurrentProcess->Status == TaskStatus::Running)
                CurrentCPU->CurrentProcess->Status = TaskStatus::Ready;
            if (CurrentCPU->CurrentThread->Status == TaskStatus::Running)
                CurrentCPU->CurrentThread->Status = TaskStatus::Ready;

            this->UpdateProcessStatus();
            schedbg("Passed UpdateProcessStatus");

            this->WakeUpThreads(CurrentCPU);
            schedbg("Passed WakeUpThreads");

            if (this->GetNextAvailableThread(CurrentCPU))
            {
#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
                SuccessSource = 1;
#endif
                goto Success;
            }
            schedbg("Passed GetNextAvailableThread");

            if (this->GetNextAvailableProcess(CurrentCPU))
            {
#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
                SuccessSource = 2;
#endif
                goto Success;
            }
            schedbg("Passed GetNextAvailableProcess");

            this->SchedulerCleanupProcesses();
            schedbg("Passed SchedulerCleanupProcesses");

            if (SchedulerSearchProcessThread(CurrentCPU))
            {
#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
                SuccessSource = 3;
#endif
                schedbg("Passed SchedulerSearchProcessThread");
                goto Success;
            }
            else
            {
                schedbg("SchedulerSearchProcessThread failed. Going idle.");
                goto Idle;
            }
        }

        /* [this]->RealEnd */
        warn("Unwanted reach!");
        TaskingScheduler_OneShot(100);
        goto RealEnd;

    /* Idle-->Success */
    Idle:
        CurrentCPU->CurrentProcess = IdleProcess;
        CurrentCPU->CurrentThread = IdleThread;

    /* Success-->End */
    Success:
        schedbg("Process \"%s\"(%d) Thread \"%s\"(%d) is now running on CPU %d",
                CurrentCPU->CurrentProcess->Name, CurrentCPU->CurrentProcess->ID,
                CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID, CurrentCPU->ID);

        CurrentCPU->CurrentProcess->Status = TaskStatus::Running;
        CurrentCPU->CurrentThread->Status = TaskStatus::Running;

        *Frame = CurrentCPU->CurrentThread->Registers;

        for (size_t i = 0; i < sizeof(CurrentCPU->CurrentThread->IPHistory) / sizeof(CurrentCPU->CurrentThread->IPHistory[0]); i++)
            CurrentCPU->CurrentThread->IPHistory[i + 1] = CurrentCPU->CurrentThread->IPHistory[i];

        CurrentCPU->CurrentThread->IPHistory[0] = Frame->rip;

        GlobalDescriptorTable::SetKernelStack((void *)((uintptr_t)CurrentCPU->CurrentThread->Stack->GetStackTop()));
        CPU::x64::writecr3({.raw = (uint64_t)CurrentCPU->CurrentProcess->PageTable});
        /* Not sure if this is needed, but it's better to be safe than sorry. */
        asmv("movq %cr3, %rax");
        asmv("movq %rax, %cr3");
        CPU::x64::fxrstor(CurrentCPU->CurrentThread->FPU);
        CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, CurrentCPU->CurrentThread->GSBase);
        CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, CurrentCPU->CurrentThread->FSBase);

#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
        OnScreenTaskManagerUpdate();
#endif

        switch (CurrentCPU->CurrentProcess->Security.TrustLevel)
        {
        case TaskTrustLevel::System:
        case TaskTrustLevel::Kernel:
            // wrmsr(MSR_SHADOW_GS_BASE, (uint64_t)CurrentCPU->CurrentThread);
            break;
        case TaskTrustLevel::User:
            // wrmsr(MSR_SHADOW_GS_BASE, CurrentCPU->CurrentThread->gs);
            break;
        default:
            error("Unknown trust level %d.", CurrentCPU->CurrentProcess->Security.TrustLevel);
            break;
        }

        /* End-->RealEnd */
        // End:
        /* TODO: This is not accurate. */
        if (CurrentCPU->CurrentProcess->Security.TrustLevel == TaskTrustLevel::User)
            UpdateUserTime(&CurrentCPU->CurrentProcess->Info);
        else
            UpdateKernelTime(&CurrentCPU->CurrentProcess->Info);

        if (CurrentCPU->CurrentThread->Security.TrustLevel == TaskTrustLevel::User)
            UpdateUserTime(&CurrentCPU->CurrentThread->Info);
        else
            UpdateKernelTime(&CurrentCPU->CurrentThread->Info);

        UpdateUsage(&CurrentCPU->CurrentProcess->Info, CurrentCPU->ID);
        UpdateUsage(&CurrentCPU->CurrentThread->Info, CurrentCPU->ID);
        TaskingScheduler_OneShot(CurrentCPU->CurrentThread->Info.Priority);

        if (CurrentCPU->CurrentThread->Security.IsDebugEnabled && CurrentCPU->CurrentThread->Security.IsKernelDebugEnabled)
            trace("%s[%ld]: RIP=%#lx  RBP=%#lx  RSP=%#lx",
                  CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
                  CurrentCPU->CurrentThread->Registers.rip,
                  CurrentCPU->CurrentThread->Registers.rbp,
                  CurrentCPU->CurrentThread->Registers.rsp);

        schedbg("================================================================");
        schedbg("Technical Informations on Thread %s[%ld]:", CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID);
        uint64_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));
        schedbg("FS=%#lx  GS=%#lx  SS=%#lx  CS=%#lx  DS=%#lx",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                Frame->ss, Frame->cs, ds);
        schedbg("R8=%#lx  R9=%#lx  R10=%#lx  R11=%#lx",
                Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        schedbg("R12=%#lx  R13=%#lx  R14=%#lx  R15=%#lx",
                Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        schedbg("RAX=%#lx  RBX=%#lx  RCX=%#lx  RDX=%#lx",
                Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        schedbg("RSI=%#lx  RDI=%#lx  RBP=%#lx  RSP=%#lx",
                Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        schedbg("RIP=%#lx  RFL=%#lx  INT=%#lx  ERR=%#lx",
                Frame->rip, Frame->rflags, Frame->InterruptNumber, Frame->ErrorCode);
        schedbg("================================================================");

    /* RealEnd->[Function Exit] */
    RealEnd:
        this->SchedulerTicks.Store(CPU::Counter() - SchedTmpTicks);
        __sync; /* TODO: Is this really needed? */
    }

    SafeFunction __no_instrument_function void Task::OnInterruptReceived(CPU::x64::TrapFrame *Frame) { this->Schedule(Frame); }
#elif defined(__i386__)
    SafeFunction bool Task::FindNewProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::GetNextAvailableProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::SchedulerCleanupProcesses()
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::Schedule(void *Frame)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::OnInterruptReceived(void *Frame) { this->Schedule(Frame); }
#elif defined(__aarch64__)
    SafeFunction bool Task::FindNewProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::GetNextAvailableProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::SchedulerCleanupProcesses()
    {
        fixme("unimplemented");
    }

    SafeFunction bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::Schedule(void *Frame)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::OnInterruptReceived(void *Frame) { this->Schedule(Frame); }
#endif
}
