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
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../Architecture/i386/cpu/apic.hpp"
#elif defined(aa64)
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

extern "C" SafeFunction NIF void TaskingScheduler_OneShot(int TimeSlice)
{
    if (TimeSlice == 0)
        TimeSlice = Tasking::TaskPriority::Normal;
#if defined(a64)
    ((APIC::Timer *)Interrupts::apicTimer[GetCurrentCPU()->ID])->OneShot(CPU::x86::IRQ16, TimeSlice);
#elif defined(a32)
#elif defined(aa64)
#endif
}

namespace Tasking
{
#if defined(a64)
    SafeFunction NIF bool Task::FindNewProcess(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;
        fnp_schedbg("%d processes", ProcessList.size());
#ifdef DEBUG_FIND_NEW_PROCESS
        foreach (auto process in ProcessList)
            fnp_schedbg("Process %d %s", process->ID, process->Name);
#endif
        foreach (auto process in ProcessList)
        {
            if (InvalidPCB(process))
                continue;

            switch (process->Status)
            {
            case TaskStatus::Ready:
                fnp_schedbg("Ready process (%s)%d",
                            process->Name, process->ID);
                break;
            default:
                fnp_schedbg("Process \"%s\"(%d) status %d",
                            process->Name, process->ID,
                            process->Status);

                /* We don't actually remove the process. RemoveProcess
                   firstly checks if it's terminated, if not, it will
                   loop through Threads and call RemoveThread on
                   terminated threads. */
                RemoveProcess(process);
                continue;
            }

            foreach (auto thread in process->Threads)
            {
                if (InvalidTCB(thread))
                    continue;

                if (thread->Status != TaskStatus::Ready)
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

    SafeFunction NIF bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        for (size_t i = 0; i < CurrentCPU->CurrentProcess->Threads.size(); i++)
        {
            if (CurrentCPU->CurrentProcess->Threads[i] == CurrentCPU->CurrentThread.load())
            {
                size_t TempIndex = i;
            RetryAnotherThread:
                TCB *nextThread = CurrentCPU->CurrentProcess->Threads[TempIndex + 1];

                if (unlikely(InvalidTCB(nextThread)))
                {
                    if (TempIndex > CurrentCPU->CurrentProcess->Threads.size())
                        break;
                    TempIndex++;

                    gnat_schedbg("Thread %#lx is invalid", nextThread);
                    goto RetryAnotherThread;
                }

                gnat_schedbg("\"%s\"(%d) and next thread is \"%s\"(%d)",
                             CurrentCPU->CurrentProcess->Threads[i]->Name,
                             CurrentCPU->CurrentProcess->Threads[i]->ID,
                             thread->Name, thread->ID);

                if (nextThread->Status != TaskStatus::Ready)
                {
                    gnat_schedbg("Thread %d is not ready", nextThread->ID);
                    TempIndex++;
                    goto RetryAnotherThread;
                }

                if (nextThread->Info.Affinity[CurrentCPU->ID] == false)
                    continue;

                CurrentCPU->CurrentThread = nextThread;
                gnat_schedbg("[thd 0 -> end] Scheduling thread %d parent of %s->%d Procs %d",
                             thread->ID, thread->Parent->Name,
                             CurrentCPU->CurrentProcess->Threads.size(), ProcessList.size());
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

    SafeFunction NIF bool Task::GetNextAvailableProcess(void *CPUDataPointer)
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

            if (InvalidPCB(process))
            {
                gnap_schedbg("Invalid process %#lx", process);
                continue;
            }

            if (process->Status != TaskStatus::Ready)
            {
                gnap_schedbg("Process %d is not ready", process->ID);
                continue;
            }

            foreach (auto thread in process->Threads)
            {
                if (InvalidTCB(thread))
                {
                    gnap_schedbg("Invalid thread %#lx", thread);
                    continue;
                }

                if (thread->Status != TaskStatus::Ready)
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

    SafeFunction NIF bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        foreach (auto process in ProcessList)
        {
            if (InvalidPCB(process))
            {
                sspt_schedbg("Invalid process %#lx", process);
                continue;
            }

            if (process->Status != TaskStatus::Ready)
            {
                sspt_schedbg("Process %d is not ready", process->ID);
                continue;
            }

            foreach (auto thread in process->Threads)
            {
                if (InvalidTCB(thread))
                {
                    sspt_schedbg("Invalid thread %#lx", thread);
                    continue;
                }

                if (thread->Status != TaskStatus::Ready)
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

    SafeFunction NIF void Task::UpdateProcessStatus()
    {
        foreach (auto process in ProcessList)
        {
            if (InvalidPCB(process))
                continue;

            if (process->Status == TaskStatus::Terminated ||
                process->Status == TaskStatus::Stopped)
                continue;

            bool AllThreadsSleeping = true;
            foreach (auto thread in process->Threads)
            {
                if (thread->Status != TaskStatus::Sleeping)
                {
                    AllThreadsSleeping = false;
                    break;
                }
            }

            if (AllThreadsSleeping)
                process->Status = TaskStatus::Sleeping;
            else if (process->Status == TaskStatus::Sleeping)
                process->Status = TaskStatus::Ready;
        }
    }

    SafeFunction NIF void Task::WakeUpThreads()
    {
        foreach (auto process in ProcessList)
        {
            if (InvalidPCB(process))
                continue;

            if (process->Status == TaskStatus::Terminated ||
                process->Status == TaskStatus::Stopped)
                continue;

            foreach (auto thread in process->Threads)
            {
                if (InvalidTCB(thread))
                    continue;

                if (thread->Status != TaskStatus::Sleeping)
                    continue;

                /* Check if the thread is ready to wake up. */
                if (thread->Info.SleepUntil < TimeManager->GetCounter())
                {
                    if (process->Status == TaskStatus::Sleeping)
                        process->Status = TaskStatus::Ready;
                    thread->Status = TaskStatus::Ready;

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

    SafeFunction NIF void OnScreenTaskManagerUpdate()
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

    SafeFunction NIF void Task::Schedule(CPU::x64::TrapFrame *Frame)
    {
        if (StopScheduler)
        {
            warn("Scheduler stopped.");
            return;
        }
        bool ProcessNotChanged = false;
        CPU::x64::writecr3({.raw = (uint64_t)KernelPageTable}); /* Restore kernel page table for safety reasons. */
        uint64_t SchedTmpTicks = TimeManager->GetCounter();
        this->LastTaskTicks.store(SchedTmpTicks - this->SchedulerTicks.load());
        CPUData *CurrentCPU = GetCurrentCPU();
        this->LastCore.store(CurrentCPU->ID);
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

        if (unlikely(InvalidPCB(CurrentCPU->CurrentProcess.load()) || InvalidTCB(CurrentCPU->CurrentThread.load())))
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
            CPU::x64::fxsave(CurrentCPU->CurrentThread->FPU);
            CurrentCPU->CurrentThread->ShadowGSBase = CPU::x64::rdmsr(CPU::x64::MSR_SHADOW_GS_BASE);
            CurrentCPU->CurrentThread->GSBase = CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
            CurrentCPU->CurrentThread->FSBase = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);

            if (CurrentCPU->CurrentProcess->Status == TaskStatus::Running)
                CurrentCPU->CurrentProcess->Status = TaskStatus::Ready;
            if (CurrentCPU->CurrentThread->Status == TaskStatus::Running)
                CurrentCPU->CurrentThread->Status = TaskStatus::Ready;

            this->UpdateProcessStatus();
            schedbg("Passed UpdateProcessStatus");

            this->WakeUpThreads();
            schedbg("Passed WakeUpThreads");

            if (this->GetNextAvailableThread(CurrentCPU))
            {
#ifdef ON_SCREEN_SCHEDULER_TASK_MANAGER
                SuccessSource = 1;
#endif
                ProcessNotChanged = true;
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

        warn("Unwanted reach!");
        TaskingScheduler_OneShot(100);
        goto End;

    Idle:
        ProcessNotChanged = true;
        CurrentCPU->CurrentProcess = IdleProcess;
        CurrentCPU->CurrentThread = IdleThread;

    Success:
        schedbg("Process \"%s\"(%d) Thread \"%s\"(%d) is now running on CPU %d",
                CurrentCPU->CurrentProcess->Name, CurrentCPU->CurrentProcess->ID,
                CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID, CurrentCPU->ID);

        if (!ProcessNotChanged)
            UpdateUsage(&CurrentCPU->CurrentProcess->Info, &CurrentCPU->CurrentProcess->Security, CurrentCPU->ID);
        UpdateUsage(&CurrentCPU->CurrentThread->Info, &CurrentCPU->CurrentThread->Security, CurrentCPU->ID);

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
        CPU::x64::wrmsr(CPU::x64::MSR_SHADOW_GS_BASE, CurrentCPU->CurrentThread->ShadowGSBase);
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
            error("Unknown trust level %d.",
                  CurrentCPU->CurrentProcess->Security.TrustLevel);
            break;
        }

        if (!ProcessNotChanged)
            (&CurrentCPU->CurrentProcess->Info)->LastUpdateTime = TimeManager->GetCounter();
        (&CurrentCPU->CurrentThread->Info)->LastUpdateTime = TimeManager->GetCounter();
        TaskingScheduler_OneShot(CurrentCPU->CurrentThread->Info.Priority);

        if (CurrentCPU->CurrentThread->Security.IsDebugEnabled && CurrentCPU->CurrentThread->Security.IsKernelDebugEnabled)
            trace("%s[%ld]: RIP=%#lx  RBP=%#lx  RSP=%#lx",
                  CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
                  CurrentCPU->CurrentThread->Registers.rip,
                  CurrentCPU->CurrentThread->Registers.rbp,
                  CurrentCPU->CurrentThread->Registers.rsp);

        schedbg("================================================================");
        schedbg("Technical Informations on Thread %s[%ld]:",
                CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID);
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

    End:
        this->SchedulerTicks.store(TimeManager->GetCounter() - SchedTmpTicks);
        __sync; /* TODO: Is this really needed? */
    }

    SafeFunction NIF void Task::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        SmartCriticalSection(SchedulerLock);
        this->Schedule(Frame);
    }
#elif defined(a32)
    SafeFunction bool Task::FindNewProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
        return false;
    }

    SafeFunction bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
        return false;
    }

    SafeFunction bool Task::GetNextAvailableProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
        return false;
    }

    SafeFunction bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
        return false;
    }

    SafeFunction void Task::Schedule(void *Frame)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::OnInterruptReceived(CPU::x32::TrapFrame *Frame) { this->Schedule(Frame); }
#elif defined(aa64)
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

    SafeFunction bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::Schedule(CPU::aarch64::TrapFrame *Frame)
    {
        fixme("unimplemented");
    }

    SafeFunction void Task::OnInterruptReceived(CPU::aarch64::TrapFrame *Frame) { this->Schedule(Frame); }
#endif
}
