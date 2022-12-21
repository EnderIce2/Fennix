#include <task.hpp>

#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>

#include "../kernel.h"

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#include "../Architecture/i686/cpu/apic.hpp"
#elif defined(__aarch64__)
#endif

// #define DEBUG_SCHEDULER 1

#ifdef DEBUG_SCHEDULER
#define schedbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define schedbg(m, ...)
#endif

NewLock(TaskingLock);
NewLock(SchedulerLock);

namespace Tasking
{
    extern "C" SafeFunction __no_instrument_function void OneShot(int TimeSlice)
    {
        if (TimeSlice == 0)
            TimeSlice = 10;
#if defined(__amd64__)
        ((APIC::Timer *)Interrupts::apicTimer[GetCurrentCPU()->ID])->OneShot(CPU::x64::IRQ16, TimeSlice);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    void Task::Schedule()
    {
        if (!StopScheduler)
            OneShot(100);
        // APIC::InterruptCommandRegisterLow icr;
        // icr.Vector = CPU::x64::IRQ16;
        // icr.Level = APIC::APICLevel::Assert;
        // ((APIC::APIC *)Interrupts::apic[0])->IPI(GetCurrentCPU()->ID, icr);
    }

    __naked __used __no_stack_protector __no_instrument_function void IdleProcessLoop()
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("IdleLoop:\n"
             "hlt\n"
             "jmp IdleLoop\n");
#elif defined(__aarch64__)
        asmv("IdleLoop:\n"
             "wfe\n"
             "b IdleLoop\n");
#endif
    }

    SafeFunction __no_instrument_function bool Task::InvalidPCB(PCB *pcb)
    {
        if (!pcb)
            return true;
        if (pcb >= (PCB *)0xfffffffffffff000)
            return true;
        if (!Memory::Virtual().Check((void *)pcb))
            return true;
        return false;
    }

    SafeFunction __no_instrument_function bool Task::InvalidTCB(TCB *tcb)
    {
        if (!tcb)
            return true;
        if (tcb >= (TCB *)0xfffffffffffff000)
            return true;
        if (!Memory::Virtual().Check((void *)tcb))
            return true;
        return false;
    }

    SafeFunction __no_instrument_function void Task::RemoveThread(TCB *Thread)
    {
        for (size_t i = 0; i < Thread->Parent->Threads.size(); i++)
            if (Thread->Parent->Threads[i] == Thread)
            {
                trace("Thread \"%s\"(%d) removed from process \"%s\"(%d)",
                      Thread->Name, Thread->ID, Thread->Parent->Name, Thread->Parent->ID);
                // Free memory
                delete Thread->Stack;
                delete Thread->Memory;
                SecurityManager.DestroyToken(Thread->Security.UniqueToken);
                delete Thread->Parent->Threads[i];
                // Remove from the list
                Thread->Parent->Threads.remove(i);
                break;
            }
    }

    SafeFunction __no_instrument_function void Task::RemoveProcess(PCB *Process)
    {
        if (Process == nullptr)
            return;

        if (Process->Status == Terminated)
        {
            foreach (TCB *thread in Process->Threads)
                RemoveThread(thread);

            foreach (PCB *process in Process->Children)
                RemoveProcess(process);

            for (size_t i = 0; i < ListProcess.size(); i++)
            {
                if (ListProcess[i] == Process)
                {
                    trace("Process \"%s\"(%d) removed from the list", Process->Name, Process->ID);
                    // Free memory
                    delete ListProcess[i]->IPCHandles;
                    delete ListProcess[i]->ELFSymbolTable;
                    SecurityManager.DestroyToken(ListProcess[i]->Security.UniqueToken);
                    if (ListProcess[i]->Security.TrustLevel == TaskTrustLevel::User)
                        KernelAllocator.FreePages((void *)ListProcess[i]->PageTable, TO_PAGES(PAGE_SIZE));
                    delete ListProcess[i];
                    // Remove from the list
                    ListProcess.remove(i);
                    break;
                }
            }
        }
        else
        {
            foreach (TCB *thread in Process->Threads)
                if (thread->Status == Terminated)
                    RemoveThread(thread);
        }
    }

    SafeFunction __no_instrument_function void Task::UpdateUserTime(TaskInfo *Info)
    {
        // TODO
        Info->UserTime++;
    }

    SafeFunction __no_instrument_function void Task::UpdateKernelTime(TaskInfo *Info)
    {
        // TODO
        Info->KernelTime++;
    }

    SafeFunction __no_instrument_function void Task::UpdateUsage(TaskInfo *Info, int Core)
    {
        if (Info->Affinity[Core] == true)
        {
            // TODO: Not working(?)
            uint64_t CounterNow = CPU::Counter();

            Info->OldUserTime = Info->CurrentUserTime;
            Info->OldKernelTime = Info->CurrentKernelTime;

            Info->CurrentUserTime = Info->UserTime;
            Info->CurrentKernelTime = Info->KernelTime;

            Info->Usage[Core] = (Info->CurrentUserTime - Info->OldUserTime) + (Info->CurrentKernelTime - Info->OldKernelTime);
            Info->Usage[Core] = (Info->Usage[Core] * 100) / (CounterNow - Info->SpawnTime);

            Info->OldUserTime = Info->CurrentUserTime;
            Info->OldKernelTime = Info->CurrentKernelTime;

            Info->CurrentUserTime = Info->UserTime;
            Info->CurrentKernelTime = Info->KernelTime;
        }
    }

#if defined(__amd64__)
    SafeFunction __no_instrument_function bool Task::FindNewProcess(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;
        schedbg("%d processes", ListProcess.size());
#ifdef DEBUG_SCHEDULER
        foreach (auto var in ListProcess)
        {
            schedbg("Process %d %s", var->ID, var->Name);
        }
#endif
        // Find a new process to execute.
        foreach (PCB *pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;

            // Check process status.
            switch (pcb->Status)
            {
            case TaskStatus::Ready:
                schedbg("Ready process (%s)%d", pcb->Name, pcb->ID);
                break;
            default:
                schedbg("Process \"%s\"(%d) status %d", pcb->Name, pcb->ID, pcb->Status);
                RemoveProcess(pcb);
                continue;
            }

            // Get first available thread from the list.
            foreach (TCB *tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                    continue;

                if (tcb->Status != TaskStatus::Ready)
                    continue;

                // Set process and thread as the current one's.
                CurrentCPU->CurrentProcess = pcb;
                CurrentCPU->CurrentThread = tcb;
                // Success!
                return true;
            }
        }
        schedbg("No process to run.");
        // No process found. Idling...
        return false;
    }

    SafeFunction __no_instrument_function bool Task::GetNextAvailableThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        for (size_t i = 0; i < CurrentCPU->CurrentProcess->Threads.size(); i++)
        {
            // Loop until we find the current thread from the process thread list.
            if (CurrentCPU->CurrentProcess->Threads[i] == CurrentCPU->CurrentThread)
            {
                // Check if the next thread is valid. If not, we search until we find, but if we reach the end of the list, we go to the next process.
                size_t TempIndex = i;
            RetryAnotherThread:
                TCB *thread = CurrentCPU->CurrentProcess->Threads[TempIndex + 1];
                if (InvalidTCB(thread))
                {
                    if (TempIndex > CurrentCPU->CurrentProcess->Threads.size())
                        break;
                    TempIndex++;
                    goto RetryAnotherThread;
                }

                schedbg("\"%s\"(%d) and next thread is \"%s\"(%d)", CurrentCPU->CurrentProcess->Threads[i]->Name, CurrentCPU->CurrentProcess->Threads[i]->ID, thread->Name, thread->ID);

                // Check if the thread is ready to be executed.
                if (thread->Status != TaskStatus::Ready)
                {
                    schedbg("Thread %d is not ready", thread->ID);
                    goto RetryAnotherThread;
                }

                // Everything is fine, we can set the new thread as the current one.
                CurrentCPU->CurrentThread = thread;
                schedbg("[thd 0 -> end] Scheduling thread %d parent of %s->%d Procs %d", thread->ID, thread->Parent->Name, CurrentCPU->CurrentProcess->Threads.size(), ListProcess.size());
                // Yay! We found a new thread to execute.
                return true;
            }
        }
        return false;
    }

    SafeFunction __no_instrument_function bool Task::GetNextAvailableProcess(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        for (size_t i = 0; i < ListProcess.size(); i++)
        {
            // Loop until we find the current process from the process list.
            if (ListProcess[i] == CurrentCPU->CurrentProcess)
            {
                // Check if the next process is valid. If not, we search until we find.
                size_t TempIndex = i;
            RetryAnotherProcess:
                PCB *pcb = ListProcess[TempIndex + 1];
                if (InvalidPCB(pcb))
                {
                    if (TempIndex > ListProcess.size())
                        break;
                    TempIndex++;
                    goto RetryAnotherProcess;
                }

                if (pcb->Status != TaskStatus::Ready)
                    goto RetryAnotherProcess;

                // Everything good, now search for a thread.
                for (size_t j = 0; j < pcb->Threads.size(); j++)
                {
                    TCB *tcb = pcb->Threads[j];
                    if (InvalidTCB(tcb))
                        continue;
                    if (tcb->Status != TaskStatus::Ready)
                        continue;
                    // Success! We set as the current one and restore the stuff.
                    CurrentCPU->CurrentProcess = pcb;
                    CurrentCPU->CurrentThread = tcb;
                    schedbg("[cur proc+1 -> first thd] Scheduling thread %d %s->%d (Total Procs %d)", tcb->ID, tcb->Name, pcb->Threads.size(), ListProcess.size());
                    return true;
                }
            }
        }
        return false;
    }

    SafeFunction __no_instrument_function void Task::SchedulerCleanupProcesses()
    {
        foreach (PCB *pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;
            RemoveProcess(pcb);
        }
    }

    SafeFunction __no_instrument_function bool Task::SchedulerSearchProcessThread(void *CPUDataPointer)
    {
        CPUData *CurrentCPU = (CPUData *)CPUDataPointer;

        foreach (PCB *pcb in ListProcess)
        {
            if (InvalidPCB(pcb))
                continue;
            if (pcb->Status != TaskStatus::Ready)
                continue;

            // Now do the thread search!
            foreach (TCB *tcb in pcb->Threads)
            {
                if (InvalidTCB(tcb))
                    continue;
                if (tcb->Status != TaskStatus::Ready)
                    continue;
                // \o/ We found a new thread to execute.
                CurrentCPU->CurrentProcess = pcb;
                CurrentCPU->CurrentThread = tcb;
                schedbg("[proc 0 -> end -> first thd] Scheduling thread %d parent of %s->%d (Procs %d)", tcb->ID, tcb->Parent->Name, pcb->Threads.size(), ListProcess.size());
                return true;
            }
        }
        return false;
    }

    SafeFunction __no_instrument_function void Task::Schedule(CPU::x64::TrapFrame *Frame)
    {
        SmartCriticalSection(SchedulerLock);
        if (StopScheduler)
        {
            warn("Scheduler stopped.");
            return;
        }
        CPU::x64::writecr3({.raw = (uint64_t)KernelPageTable}); // Restore kernel page table for safety reasons.
        CPUData *CurrentCPU = GetCurrentCPU();
        // if (CurrentCPU->ID != 0)
        // debug("Scheduler called from CPU %d", CurrentCPU->ID);
        schedbg("Scheduler called on CPU %d.", CurrentCPU->ID);
        schedbg("%d: %ld%%", CurrentCPU->ID, GetUsage(CurrentCPU->ID));

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

        // Null or invalid process/thread? Let's find a new one to execute.
        if (InvalidPCB(CurrentCPU->CurrentProcess) || InvalidTCB(CurrentCPU->CurrentThread))
        {
            if (this->FindNewProcess(CurrentCPU))
                goto Success;
            else
                goto Idle;
        }
        else
        {
            // Save current process and thread registries, gs, fs, fpu, etc...
            CurrentCPU->CurrentThread->Registers = *Frame;
            CPU::x64::fxsave(CurrentCPU->CurrentThread->FXRegion);
            CurrentCPU->CurrentThread->GSBase = CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
            CurrentCPU->CurrentThread->FSBase = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);

            // Set the process & thread as ready if it's running.
            if (CurrentCPU->CurrentProcess->Status == TaskStatus::Running)
                CurrentCPU->CurrentProcess->Status = TaskStatus::Ready;
            if (CurrentCPU->CurrentThread->Status == TaskStatus::Running)
                CurrentCPU->CurrentThread->Status = TaskStatus::Ready;

            // Get next available thread from the list.
            if (this->GetNextAvailableThread(CurrentCPU))
                goto Success;

            // If the last process didn't find a thread to execute, we search for a new process.
            if (this->GetNextAvailableProcess(CurrentCPU))
                goto Success;

            // Before checking from the beginning, we remove everything that is terminated.
            this->SchedulerCleanupProcesses();

            // If we didn't find anything, we check from the start of the list. This is the last chance to find something or we go to idle.
            if (SchedulerSearchProcessThread(CurrentCPU))
                goto Success;
            else
                goto Idle;
        }
        goto UnwantedReach; // This should never happen.

    Idle:
    {
        CurrentCPU->CurrentProcess = IdleProcess;
        CurrentCPU->CurrentThread = IdleThread;
        goto Success;
    }

    Success:
    {
#ifdef DEBUG_SCHEDULER
        static int sanity;
        const char *Statuses[] = {
            "FF0000", // Unknown
            "AAFF00", // Ready
            "00AA00", // Running
            "FFAA00", // Sleeping
            "FFAA00", // Waiting
            "FF0088", // Stopped
            "FF0000", // Terminated
        };
        const char *StatusesSign[] = {
            "U", // Unknown
            "R", // Ready
            "r", // Running
            "S", // Sleeping
            "W", // Waiting
            "s", // Stopped
            "T", // Terminated
        };
        for (int i = 0; i < 200; i++)
            for (int j = 0; j < 200; j++)
                Display->SetPixel(i, j, 0x222222, 0);
        uint32_t tmpX, tmpY;
        Display->GetBufferCursor(0, &tmpX, &tmpY);
        Display->SetBufferCursor(0, 0, 0);
        foreach (auto var in ListProcess)
        {
            int statuu = var->Status;
            printf_("\e%s-> \eAABBCC%s\eCCCCCC[%d] \e00AAAA%s\n",
                    Statuses[statuu], var->Name, statuu, StatusesSign[statuu]);
            foreach (auto var2 in var->Threads)
            {
                int statui = var2->Status;
                printf_("  \e%s-> \eAABBCC%s\eCCCCCC[%d] \e00AAAA%s\n\eAABBCC",
                        Statuses[statui], var2->Name, statui, StatusesSign[statui]);
            }
        }
        printf_("%d", sanity++);
        if (sanity > 1000)
            sanity = 0;
        Display->SetBufferCursor(0, tmpX, tmpY);
        Display->SetBuffer(0);
#endif
        schedbg("Process \"%s\"(%d) Thread \"%s\"(%d) is now running on CPU %d",
                CurrentCPU->CurrentProcess->Name, CurrentCPU->CurrentProcess->ID,
                CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID, CurrentCPU->ID);

        CurrentCPU->CurrentProcess->Status = TaskStatus::Running;
        CurrentCPU->CurrentThread->Status = TaskStatus::Running;

        *Frame = CurrentCPU->CurrentThread->Registers;

        // FIXME: Untested
        for (int i = 0; i < 128; i++)
        {
            if (CurrentCPU->CurrentThread->IPHistory[i] == 0)
            {
                CurrentCPU->CurrentThread->IPHistory[i] = Frame->rip;
                break;
            }

            if (i == 127)
            {
                for (int j = 0; j < 127; j++)
                    CurrentCPU->CurrentThread->IPHistory[j] = CurrentCPU->CurrentThread->IPHistory[j + 1];
                CurrentCPU->CurrentThread->IPHistory[127] = Frame->rip;
            }
        }
        GlobalDescriptorTable::SetKernelStack((void *)((uintptr_t)CurrentCPU->CurrentThread->Stack->GetStackTop()));
        CPU::x64::writecr3({.raw = (uint64_t)CurrentCPU->CurrentProcess->PageTable});
        // Not sure if this is needed, but it's better to be safe than sorry.
        asmv("movq %cr3, %rax");
        asmv("movq %rax, %cr3");
        CPU::x64::fxrstor(CurrentCPU->CurrentThread->FXRegion);
        CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, CurrentCPU->CurrentThread->GSBase);
        CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, CurrentCPU->CurrentThread->FSBase);

        switch (CurrentCPU->CurrentProcess->Security.TrustLevel)
        {
        case TaskTrustLevel::System:
        case TaskTrustLevel::Idle:
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
        goto End;
    }
    UnwantedReach:
    {
        warn("Unwanted reach!");
        OneShot(100);
        goto RealEnd;
    }
    End:
    {
        // TODO: This is not accurate.
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
        OneShot(CurrentCPU->CurrentThread->Info.Priority);
    }
        {
            if (CurrentCPU->CurrentThread->Security.IsDebugEnabled && CurrentCPU->CurrentThread->Security.IsKernelDebugEnabled)
                trace("%s[%ld]: RIP=%#lx  RBP=%#lx  RSP=%#lx",
                      CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
                      CurrentCPU->CurrentThread->Registers.rip,
                      CurrentCPU->CurrentThread->Registers.rbp,
                      CurrentCPU->CurrentThread->Registers.rsp);
        }
        {
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
        }
    RealEnd:
    {
        __sync_synchronize(); // TODO: Is this really needed?
    }
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

    void ThreadDoExit()
    {
        // TODO: How I can lock the scheduler without causing a deadlock?
        CPUData *CPUData = GetCurrentCPU();
        CPUData->CurrentThread->Status = TaskStatus::Terminated;
        debug("\"%s\"(%d) exited with code: %#lx", CPUData->CurrentThread->Name, CPUData->CurrentThread->ID, CPUData->CurrentThread->ExitCode);
        CPU::Halt(true);
    }

    PCB *Task::GetCurrentProcess() { return GetCurrentCPU()->CurrentProcess; }
    TCB *Task::GetCurrentThread() { return GetCurrentCPU()->CurrentThread; }

    void Task::WaitForProcess(PCB *pcb)
    {
        if (!pcb)
            return;
        if (pcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for process \"%s\"(%d)", pcb->Name, pcb->ID);
        while (pcb->Status != TaskStatus::Terminated)
            CPU::Halt();
    }

    void Task::WaitForThread(TCB *tcb)
    {
        if (!tcb)
            return;
        if (tcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for thread \"%s\"(%d)", tcb->Name, tcb->ID);
        while (tcb->Status != TaskStatus::Terminated)
            CPU::Halt();
    }

    TCB *Task::CreateThread(PCB *Parent,
                            IP EntryPoint,
                            const char **argv,
                            const char **envp,
                            Vector<AuxiliaryVector> &auxv,
                            IPOffset Offset,
                            TaskArchitecture Architecture,
                            TaskCompatibility Compatibility)
    {
        SmartCriticalSection(TaskingLock);
        TCB *Thread = new TCB;
        if (Parent == nullptr)
        {
            Thread->Parent = this->GetCurrentProcess();
            if (Thread->Parent == nullptr)
            {
                error("Failed to get current process. Thread cannot be created.");
                delete Thread;
                return nullptr;
            }
        }
        else
            Thread->Parent = Parent;

        if (!Parent)
        {
            error("Parent is null");
            delete Thread;
            return nullptr;
        }

        Thread->ID = this->NextTID++;
        strcpy(Thread->Name, Parent->Name);
        Thread->EntryPoint = EntryPoint;
        Thread->Offset = Offset;
        Thread->ExitCode = 0xdead;
        Thread->Status = TaskStatus::Ready;

#if defined(__amd64__)
        memset(&Thread->Registers, 0, sizeof(CPU::x64::TrapFrame)); // Just in case
        Thread->Registers.rip = (EntryPoint + Offset);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
        switch (Parent->Security.TrustLevel)
        {
        case TaskTrustLevel::System:
            warn("Trust level not supported.");
            [[fallthrough]];
        case TaskTrustLevel::Idle:
        case TaskTrustLevel::Kernel:
        {
            Thread->Stack = new Memory::StackGuard(false, Parent->PageTable);
#if defined(__amd64__)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TokenTrustLevel::TrustedByKernel);
            Thread->GSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_GS_BASE);
            Thread->FSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_FS_BASE);
            Thread->Registers.cs = GDT_KERNEL_CODE;
            Thread->Registers.ss = GDT_KERNEL_DATA;
            Thread->Registers.rflags.AlwaysOne = 1;
            Thread->Registers.rflags.IF = 1;
            Thread->Registers.rflags.ID = 1;
            Thread->Registers.rsp = ((uintptr_t)Thread->Stack->GetStackTop());
            POKE(uintptr_t, Thread->Registers.rsp) = (uintptr_t)ThreadDoExit;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
            Thread->Stack = new Memory::StackGuard(true, Parent->PageTable);
            Thread->Memory = new Memory::Tracker(Parent->PageTable);
#if defined(__amd64__)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TokenTrustLevel::Untrusted);
            Thread->GSBase = 0;
            Thread->FSBase = 0;
            Thread->Registers.cs = GDT_USER_CODE;
            Thread->Registers.ss = GDT_USER_DATA;
            Thread->Registers.rflags.AlwaysOne = 1;
            // Thread->Registers.rflags.PF = 1;
            // Thread->Registers.rflags.SF = 1;
            // Thread->Registers.rflags.IOPL = 3;
            Thread->Registers.rflags.IF = 1;
            Thread->Registers.rflags.ID = 1;
            Thread->Registers.rsp = ((uintptr_t)Thread->Stack->GetStackTop());

            size_t ArgvSize = 0;
            if (argv)
                while (argv[ArgvSize] != nullptr)
                    ArgvSize++;

            size_t EnvpSize = 0;
            if (envp)
                while (envp[EnvpSize] != nullptr)
                    EnvpSize++;

            debug("ArgvSize: %d", ArgvSize);
            debug("EnvpSize: %d", EnvpSize);

            /* https://articles.manugarg.com/aboutelfauxiliaryvectors.html */
            /* https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf#figure.3.9 */
            // rsp is the top of the stack
            char *Stack = (char *)Thread->Stack->GetStackPhysicalTop();
            // Temporary stack pointer for strings
            char *StackStrings = (char *)Stack;
            char *StackStringsVirtual = (char *)Thread->Stack->GetStackTop();

            // Store string pointers for later
            uintptr_t ArgvStrings[ArgvSize];
            uintptr_t EnvpStrings[EnvpSize];

            for (size_t i = 0; i < ArgvSize; i++)
            {
                // Subtract the length of the string and the null terminator
                StackStrings -= strlen(argv[i]) + 1;
                StackStringsVirtual -= strlen(argv[i]) + 1;
                // Store the pointer to the string
                ArgvStrings[i] = (uintptr_t)StackStringsVirtual;
                // Copy the string to the stack
                strcpy(StackStrings, argv[i]);
            }

            for (size_t i = 0; i < EnvpSize; i++)
            {
                // Subtract the length of the string and the null terminator
                StackStrings -= strlen(envp[i]) + 1;
                StackStringsVirtual -= strlen(envp[i]) + 1;
                // Store the pointer to the string
                EnvpStrings[i] = (uintptr_t)StackStringsVirtual;
                // Copy the string to the stack
                strcpy(StackStrings, envp[i]);
            }

            // Align the stack to 16 bytes
            StackStrings -= (uintptr_t)StackStrings & 0xF;
            // Set "Stack" to the new stack pointer
            Stack = (char *)StackStrings;
            // If argv and envp sizes are odd then we need to align the stack
            Stack -= (ArgvSize + EnvpSize) % 2;

            // We need 8 bit pointers for the stack from here
            uintptr_t *Stack64 = (uintptr_t *)Stack;

            // Store the null terminator
            Stack64--;
            *Stack64 = AT_NULL;

            // Store auxillary vector
            foreach (AuxiliaryVector var in auxv)
            {
                // Subtract the size of the auxillary vector
                Stack64 -= sizeof(Elf64_auxv_t) / sizeof(uintptr_t);
                // Store the auxillary vector
                POKE(Elf64_auxv_t, Stack64) = var.archaux;
                // TODO: Store strings to the stack
            }

            // Store the null terminator
            Stack64--;
            *Stack64 = AT_NULL;

            // Store EnvpStrings[] to the stack
            Stack64 -= EnvpSize; // (1 Stack64 = 8 bits; Stack64 = 8 * EnvpSize)
            for (size_t i = 0; i < EnvpSize; i++)
            {
                *(Stack64 + i) = (uintptr_t)EnvpStrings[i];
                debug("EnvpStrings[%d]: %#lx", i, EnvpStrings[i]);
            }

            // Store the null terminator
            Stack64--;
            *Stack64 = AT_NULL;

            // Store ArgvStrings[] to the stack
            Stack64 -= ArgvSize; // (1 Stack64 = 8 bits; Stack64 = 8 * ArgvSize)
            for (size_t i = 0; i < ArgvSize; i++)
            {
                *(Stack64 + i) = (uintptr_t)ArgvStrings[i];
                debug("ArgvStrings[%d]: %#lx", i, ArgvStrings[i]);
            }

            // Store the argc
            Stack64--;
            *Stack64 = ArgvSize;

            // Set "Stack" to the new stack pointer
            Stack = (char *)Stack64;

            /* We need the virtual address but because we are in the kernel we can't use the process page table.
                So we modify the physical address and store how much we need to subtract to get the virtual address for RSP. */
            uintptr_t SubtractStack = (uintptr_t)Thread->Stack->GetStackPhysicalTop() - (uintptr_t)Stack;
            debug("SubtractStack: %#lx", SubtractStack);

            // Set the stack pointer to the new stack
            Thread->Registers.rsp = ((uintptr_t)Thread->Stack->GetStackTop() - SubtractStack);

#ifdef DEBUG
            DumpData("Stack Data", (void *)((uintptr_t)Thread->Stack->GetStackPhysicalTop() - (uintptr_t)SubtractStack), SubtractStack);
#endif

            Thread->Registers.rdi = (uintptr_t)ArgvSize;                                         // argc
            Thread->Registers.rsi = (uintptr_t)(Thread->Registers.rsp + 8);                      // argv
            Thread->Registers.rcx = (uintptr_t)EnvpSize;                                         // envc
            Thread->Registers.rdx = (uintptr_t)(Thread->Registers.rsp + 8 + (8 * ArgvSize) + 8); // envp

            /* We need to leave the libc's crt to make a syscall when the Thread is exited or we are going to get GPF or PF exception. */

            Memory::Virtual uva = Memory::Virtual(Parent->PageTable);
            if (!uva.Check((void *)Offset, Memory::PTFlag::US))
            {
                error("Offset is not user accessible");
                uva.Map((void *)Offset, (void *)Offset, Memory::PTFlag::RW | Memory::PTFlag::US); // We try one more time.
            }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
#ifdef DEBUG_SCHEDULER
            DumpData(Thread->Name, Thread->Stack, STACK_SIZE);
#endif
            break;
        }
        default:
        {
            error("Unknown elevation.");
            delete Thread->Stack;
            this->NextTID--;
            delete Thread;
            return nullptr;
        }
        }

        Thread->Security.TrustLevel = Parent->Security.TrustLevel;
        // Thread->Security.UniqueToken = SecurityManager.CreateToken();

        Thread->Info = {};
        Thread->Info.SpawnTime = CPU::Counter();
        Thread->Info.Year = 0;
        Thread->Info.Month = 0;
        Thread->Info.Day = 0;
        Thread->Info.Hour = 0;
        Thread->Info.Minute = 0;
        Thread->Info.Second = 0;
        for (int i = 0; i < MAX_CPU; i++)
        {
            Thread->Info.Usage[i] = 0;
            Thread->Info.Affinity[i] = true;
        }
        Thread->Info.Priority = 10;
        Thread->Info.Architecture = Architecture;
        Thread->Info.Compatibility = Compatibility;

#ifdef DEBUG
#ifdef __amd64__
        debug("Thread offset is %#lx (EntryPoint: %#lx) => RIP: %#lx", Thread->Offset, Thread->EntryPoint, Thread->Registers.rip);
        if (Parent->Security.TrustLevel == TaskTrustLevel::User)
            debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.rsp);
        else
            debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.rsp);
#elif defined(__i386__)
        debug("Thread offset is %#lx (EntryPoint: %#lx) => RIP: %#lx", Thread->Offset, Thread->EntryPoint, Thread->Registers.eip);
        if (Parent->Security.TrustLevel == TaskTrustLevel::User)
            debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.esp);
        else
            debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.esp);
#elif defined(__aarch64__)
#endif
        debug("Created thread \"%s\"(%d) in process \"%s\"(%d)",
              Thread->Name, Thread->ID,
              Thread->Parent->Name, Thread->Parent->ID);
#endif

        Parent->Threads.push_back(Thread);
        return Thread;
    }

    PCB *Task::CreateProcess(PCB *Parent,
                             const char *Name,
                             TaskTrustLevel TrustLevel, void *Image)
    {
        SmartCriticalSection(TaskingLock);
        PCB *Process = new PCB;
        Process->ID = this->NextPID++;
        strcpy(Process->Name, Name);
        if (Parent == nullptr)
            Process->Parent = this->GetCurrentProcess();
        else
            Process->Parent = Parent;
        Process->ExitCode = 0xdead;
        Process->Status = TaskStatus::Ready;

        Process->Security.TrustLevel = TrustLevel;
        // Process->Security.UniqueToken = SecurityManager.CreateToken();

        Process->IPCHandles = new HashMap<InterProcessCommunication::IPCPort, uintptr_t>;

        switch (TrustLevel)
        {
        case TaskTrustLevel::System:
            warn("Trust level not supported.");
            [[fallthrough]];
        case TaskTrustLevel::Idle:
        case TaskTrustLevel::Kernel:
        {
            SecurityManager.TrustToken(Process->Security.UniqueToken, TokenTrustLevel::TrustedByKernel);
#if defined(__amd64__)
            Process->PageTable = (Memory::PageTable4 *)CPU::x64::readcr3().raw;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
            SecurityManager.TrustToken(Process->Security.UniqueToken, TokenTrustLevel::Untrusted);
#if defined(__amd64__)
            Process->PageTable = (Memory::PageTable4 *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
            memcpy(Process->PageTable, (void *)UserspaceKernelOnlyPageTable, PAGE_SIZE);
            for (size_t i = 0; i < TO_PAGES(PAGE_SIZE); i++)
                Memory::Virtual(Process->PageTable).Map((void *)Process->PageTable, (void *)Process->PageTable, Memory::PTFlag::RW); // Make sure the page table is mapped.
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        default:
        {
            error("Unknown elevation.");
            this->NextPID--;
            delete Process;
            return nullptr;
        }
        }

        Process->Info = {};
        Process->Info.SpawnTime = CPU::Counter();
        Process->Info.Year = 0;
        Process->Info.Month = 0;
        Process->Info.Day = 0;
        Process->Info.Hour = 0;
        Process->Info.Minute = 0;
        Process->Info.Second = 0;
        for (int i = 0; i < MAX_CPU; i++)
        {
            Process->Info.Usage[i] = 0;
            Process->Info.Affinity[i] = true;
        }
        Process->Info.Priority = 10;

        debug("Process page table: %#lx", Process->PageTable);
        debug("Created process \"%s\"(%d) in process \"%s\"(%d)",
              Process->Name, Process->ID,
              Parent ? Process->Parent->Name : "None",
              Parent ? Process->Parent->ID : 0);

        if (Image)
        {
            // TODO: Check if it's ELF
            Process->ELFSymbolTable = new SymbolResolver::Symbols((uintptr_t)Image);
        }
        else
            debug("No image provided for process \"%s\"(%d)", Process->Name, Process->ID);

        if (Parent)
            Parent->Children.push_back(Process);
        ListProcess.push_back(Process);
        return Process;
    }

    Task::Task(const IP EntryPoint) : Interrupts::Handler(CPU::x64::IRQ16)
    {
        SmartCriticalSection(TaskingLock);
#if defined(__amd64__)
        for (int i = 0; i < SMP::CPUCores; i++)
            ((APIC::APIC *)Interrupts::apic[i])->RedirectIRQ(i, CPU::x64::IRQ16 - CPU::x64::IRQ0, 1);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
        KPrint("Starting Tasking With Instruction Pointer: %p (\e666666%s\eCCCCCC)", EntryPoint, KernelSymbolTable->GetSymbolFromAddress(EntryPoint));
        TaskingLock.Unlock();

#if defined(__amd64__)
        TaskArchitecture Arch = TaskArchitecture::x64;
#elif defined(__i386__)
        TaskArchitecture Arch = TaskArchitecture::x32;
#elif defined(__aarch64__)
        TaskArchitecture Arch = TaskArchitecture::ARM64;
#endif
        PCB *kproc = CreateProcess(nullptr, "Kernel", TaskTrustLevel::Kernel);
        Vector<AuxiliaryVector> auxv;
        TCB *kthrd = CreateThread(kproc, EntryPoint, nullptr, nullptr, auxv, 0, Arch);
        kthrd->Rename("Main Thread");
        debug("Created Kernel Process: %s and Thread: %s", kproc->Name, kthrd->Name);
        TaskingLock.Lock(__FUNCTION__);

#if defined(__amd64__)
        uint32_t rax, rbx, rcx, rdx;
        CPU::x64::cpuid(0x1, &rax, &rbx, &rcx, &rdx);
        if (rcx & CPU::x64::CPUID_FEAT_RCX_MONITOR)
        {
            trace("CPU has MONITOR/MWAIT support.");
        }

        if (!CPU::Interrupts(CPU::Check))
        {
            error("Interrupts are not enabled.");
            CPU::Interrupts(CPU::Enable);
        }
#endif
        TaskingLock.Unlock();
        IdleProcess = CreateProcess(nullptr, (char *)"Idle", TaskTrustLevel::Idle);
        for (int i = 0; i < SMP::CPUCores; i++)
        {
            Vector<AuxiliaryVector> auxv;
            IdleThread = CreateThread(IdleProcess, reinterpret_cast<uintptr_t>(IdleProcessLoop), nullptr, nullptr, auxv);
            char IdleName[16];
            sprintf_(IdleName, "Idle Thread %d", i);
            IdleThread->Rename(IdleName);
            IdleThread->SetPriority(1);
            break;
        }
        debug("Tasking Started");
#if defined(__amd64__)
        ((APIC::Timer *)Interrupts::apicTimer[0])->OneShot(CPU::x64::IRQ16, 100);

        for (int i = 1; i < SMP::CPUCores; i++)
        {
            // ((APIC::Timer *)Interrupts::apicTimer[i])->OneShot(CPU::x64::IRQ16, 100);
            // TODO: Lock was the fault here. Now crash handler should support SMP.
            // APIC::InterruptCommandRegisterLow icr;
            // icr.Vector = CPU::x64::IRQ16;
            // icr.Level = APIC::APICLevel::Assert;
            // ((APIC::APIC *)Interrupts::apic[0])->IPI(i, icr);
        }
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    Task::~Task()
    {
        SmartCriticalSection(TaskingLock);
        trace("Stopping tasking");
    }
}
