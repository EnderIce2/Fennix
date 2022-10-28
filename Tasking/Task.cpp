#include <task.hpp>

#include <lock.hpp>
#include <debug.h>
#include <smp.hpp>

#include "../kernel.h"

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

// #define DEBUG_SCHEDULER 1

#ifdef DEBUG_SCHEDULER
#define schedbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define schedbg(m, ...)
#endif

NewLock(TaskingLock);
NewLock(TaskingGetCurrentLock);

namespace Tasking
{
    extern "C" __attribute__((no_stack_protector)) void OneShot(int TimeSlice)
    {
        if (TimeSlice == 0)
            TimeSlice = 10;
#if defined(__amd64__)
        ((APIC::Timer *)Interrupts::apicTimer[GetCurrentCPU()->ID])->OneShot(CPU::x64::IRQ16, TimeSlice);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    __attribute__((naked, used, no_stack_protector)) void IdleProcessLoop()
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("IdleLoop:\n"
             "call OneShot\n"
             "hlt\n"
             "jmp IdleLoop\n");
#elif defined(__aarch64__)
        asmv("IdleLoop:\n"
             "wfe\n"
             "b IdleLoop\n");
#endif
    }

#if defined(__amd64__)
    __attribute__((no_stack_protector)) bool Task::FindNewProcess(void *CPUDataPointer)
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
                schedbg("Process %s(%d) status %d", pcb->Name, pcb->ID, pcb->Status);
                // RemoveProcess(pcb); // ignore for now
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

    __attribute__((no_stack_protector)) void Task::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        SmartCriticalSection(TaskingLock);
        CPUData *CurrentCPU = GetCurrentCPU();
        // debug("Scheduler called on CPU %d.", CurrentCPU->ID);

        schedbg("================================================================");
        schedbg("Status: 0-ukn | 1-rdy | 2-run | 3-wait | 4-term");
        schedbg("Technical Informations on regs %#lx", Frame->InterruptNumber);
        schedbg("FS=%#lx  GS=%#lx  SS=%#lx  CS=%#lx  DS=%#lx",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                Frame->ss, Frame->cs, Frame->ds);
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

            // Set the process & thread as ready if it's running.
            if (CurrentCPU->CurrentProcess->Status == TaskStatus::Running)
                CurrentCPU->CurrentProcess->Status = TaskStatus::Ready;
            if (CurrentCPU->CurrentThread->Status == TaskStatus::Running)
                CurrentCPU->CurrentThread->Status = TaskStatus::Ready;

            // Get next available thread from the list.
            for (uint64_t i = 0; i < CurrentCPU->CurrentProcess->Threads.size(); i++)
            {
                // Loop until we find the current thread from the process thread list.
                if (CurrentCPU->CurrentProcess->Threads[i] == CurrentCPU->CurrentThread)
                {
                    // Check if the next thread is valid. If not, we search until we find, but if we reach the end of the list, we go to the next process.
                    uint64_t tmpidx = i;
                RetryAnotherThread:
                    TCB *thread = CurrentCPU->CurrentProcess->Threads[tmpidx + 1];
                    if (InvalidTCB(thread))
                    {
                        if (tmpidx > CurrentCPU->CurrentProcess->Threads.size())
                            break;
                        tmpidx++;
                        goto RetryAnotherThread;
                    }

                    schedbg("%s(%d) and next thread is %s(%d)", CurrentCPU->CurrentProcess->Threads[i]->Name, CurrentCPU->CurrentProcess->Threads[i]->ID, thread->Name, thread->ID);

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
                    goto Success;
                }
            }

            // If the last process didn't find a thread to execute, we search for a new process.
            for (uint64_t i = 0; i < ListProcess.size(); i++)
            {
                // Loop until we find the current process from the process list.
                if (ListProcess[i] == CurrentCPU->CurrentProcess)
                {
                    // Check if the next process is valid. If not, we search until we find.
                    uint64_t tmpidx = i;
                RetryAnotherProcess:
                    PCB *pcb = ListProcess[tmpidx + 1];
                    if (InvalidPCB(pcb))
                    {
                        if (tmpidx > ListProcess.size())
                            break;
                        tmpidx++;
                        goto RetryAnotherProcess;
                    }

                    if (pcb->Status != TaskStatus::Ready)
                        goto RetryAnotherProcess;

                    // Everything good, now search for a thread.
                    for (uint64_t j = 0; j < pcb->Threads.size(); j++)
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
                        goto Success;
                    }
                }
            }

            // Before checking from the beginning, we remove everything that is terminated.
            foreach (PCB *pcb in ListProcess)
            {
                if (InvalidPCB(pcb))
                    continue;
                // RemoveProcess(pcb); // comment this until i will find a way to handle properly vectors, the memory need to be 0ed after removing.
            }

            // If we didn't find anything, we check from the start of the list. This is the last chance to find something or we go to idle.
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
                    goto Success;
                }
            }
        }

    Idle:
    {
        // Should I remove processes that are no longer have any threads? Remove only from userspace?
        if (IdleProcess == nullptr)
        {
            schedbg("Idle process created");
            IdleProcess = CreateProcess(nullptr, (char *)"idle", TaskTrustLevel::Idle);
            IdleThread = CreateThread(IdleProcess, reinterpret_cast<uint64_t>(IdleProcessLoop), 0);
        }
        CurrentCPU->CurrentProcess = IdleProcess;
        CurrentCPU->CurrentThread = IdleThread;

        *Frame = CurrentCPU->CurrentThread->Registers;
        CPU::x64::writecr3({.raw = (uint64_t)CurrentCPU->CurrentProcess->PageTable});
        CPU::x64::fxrstor(CurrentCPU->CurrentThread->FXRegion);
        goto End;
    }

    Success:
    {
        schedbg("Success Prc:%s(%d) Thd:%s(%d)->RIP:%#lx-RSP:%#lx(STACK: %#lx)",
                CurrentCPU->CurrentProcess->Name, CurrentCPU->CurrentProcess->ID,
                CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID,
                CurrentCPU->CurrentThread->Registers.rip, CurrentCPU->CurrentThread->Registers.rsp, CurrentCPU->CurrentThread->Stack);

        CurrentCPU->CurrentProcess->Status = TaskStatus::Running;
        CurrentCPU->CurrentThread->Status = TaskStatus::Running;

        *Frame = CurrentCPU->CurrentThread->Registers;
        CPU::x64::writecr3({.raw = (uint64_t)CurrentCPU->CurrentProcess->PageTable});

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
        CPU::x64::fxrstor(CurrentCPU->CurrentThread->FXRegion);
    }
    End:
    {
        // UpdateTimeUsed(&CurrentCPU->CurrentProcess->Info);
        // UpdateTimeUsed(&CurrentCPU->CurrentThread->Info);
        // UpdateCPUUsage(&CurrentCPU->CurrentProcess->Info);
        // UpdateCPUUsage(&CurrentCPU->CurrentThread->Info);
        OneShot(CurrentCPU->CurrentThread->Info.Priority);
        schedbg("Scheduler end");
    }
        schedbg("================================================================");
        schedbg("Technical Informations on Thread %s[%ld]:", CurrentCPU->CurrentThread->Name, CurrentCPU->CurrentThread->ID);
        schedbg("FS=%#lx  GS=%#lx  SS=%#lx  CS=%#lx  DS=%#lx",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                Frame->ss, Frame->cs, Frame->ds);
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

#elif defined(__i386__)
    __attribute__((no_stack_protector)) bool Task::FindNewProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    __attribute__((no_stack_protector)) void Task::OnInterruptReceived(void *Frame)
    {
        fixme("unimplemented");
    }

#elif defined(__aarch64__)
    __attribute__((no_stack_protector)) bool Task::FindNewProcess(void *CPUDataPointer)
    {
        fixme("unimplemented");
    }

    __attribute__((no_stack_protector)) void Task::OnInterruptReceived(void *Frame)
    {
        fixme("unimplemented");
    }
#endif

    void ThreadDoExit(int Code)
    {
        SmartCriticalSection(TaskingLock);
        CPUData *CPUData = GetCurrentCPU();
        CPUData->CurrentThread->Status = TaskStatus::Terminated;
        CPUData->CurrentThread->ExitCode = Code;
        debug("parent:%s tid:%d, code:%016p", CPUData->CurrentProcess->Name, CPUData->CurrentThread->ID, Code);
        trace("Exiting thread %d(%s)...", CPUData->CurrentThread->ID, CPUData->CurrentThread->Name);
        OneShot(CPUData->CurrentThread->Info.Priority);
        CPU::Halt(true);
    }

    PCB *Task::GetCurrentProcess()
    {
        SmartCriticalSection(TaskingGetCurrentLock);
        return GetCurrentCPU()->CurrentProcess;
    }

    TCB *Task::GetCurrentThread()
    {
        SmartCriticalSection(TaskingGetCurrentLock);
        return GetCurrentCPU()->CurrentThread;
    }

    TCB *Task::CreateThread(PCB *Parent,
                            IP EntryPoint,
                            IPOffset Offset,
                            TaskArchitecture Architecture,
                            TaskCompatibility Compatibility)
    {
        SmartCriticalSection(TaskingLock);
        TCB *Thread = new TCB;
        if (Parent == nullptr)
            Thread->Parent = this->GetCurrentProcess();
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
        Thread->ExitCode = 0xdeadbeef;
        Thread->Stack = (void *)((uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE);
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
#if defined(__amd64__)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TokenTrustLevel::TrustedByKernel);
            Thread->Registers.cs = GDT_KERNEL_CODE;
            Thread->Registers.ds = GDT_KERNEL_DATA;
            Thread->Registers.ss = GDT_KERNEL_DATA;
            Thread->Registers.rflags.AlwaysOne = 1;
            Thread->Registers.rflags.IF = 1;
            Thread->Registers.rflags.ID = 1;
            Thread->Registers.rsp = (uint64_t)Thread->Stack;
            POKE(uint64_t, Thread->Registers.rsp) = (uint64_t)ThreadDoExit;
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
#if defined(__amd64__)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TokenTrustLevel::Untrusted);
            Thread->Registers.cs = GDT_USER_CODE;
            Thread->Registers.ds = GDT_USER_DATA;
            Thread->Registers.ss = GDT_USER_DATA;
            Thread->Registers.rflags.AlwaysOne = 1;
            Thread->Registers.rflags.IF = 1;
            Thread->Registers.rflags.ID = 1;
            Thread->Registers.rsp = (uint64_t)Thread->Stack;
            /* We need to leave the libc's crt to make a syscall when the Thread is exited or we are going to get GPF or PF exception. */
            for (uint64_t i = 0; i < TO_PAGES(STACK_SIZE); i++)
                Memory::Virtual().Map((void *)((uint64_t)Thread->Stack + (i * PAGE_SIZE)),
                                      (void *)((uint64_t)Thread->Stack + (i * PAGE_SIZE)),
                                      Memory::PTFlag::US);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        default:
        {
            error("Unknown elevation.");
            KernelAllocator.FreePages((void *)((uint64_t)Thread->Stack - STACK_SIZE), TO_PAGES(STACK_SIZE));
            this->NextTID--;
            delete Thread;
            return nullptr;
        }
        }

        Thread->Security.TrustLevel = Parent->Security.TrustLevel;
        // Thread->Security.UniqueToken = SecurityManager.CreateToken();

        Thread->Info.SpawnTime = 0;
        Thread->Info.UsedTime = 0;
        Thread->Info.OldUsedTime = 0;
        Thread->Info.OldSystemTime = 0;
        Thread->Info.CurrentSystemTime = 0;
        Thread->Info.Year = 0;
        Thread->Info.Month = 0;
        Thread->Info.Day = 0;
        Thread->Info.Hour = 0;
        Thread->Info.Minute = 0;
        Thread->Info.Second = 0;
        for (int i = 0; i < MAX_CPU; i++)
        {
            Thread->Info.Usage[i] = 0;
            Thread->Info.Affinity[i] = 0;
        }
        Thread->Info.Priority = 0;
        Thread->Info.Architecture = Architecture;
        Thread->Info.Compatibility = Compatibility;

        debug("Created thread %d(%s) in process %d(%s)", Thread->ID, Thread->Name, Thread->Parent->ID, Thread->Parent->Name);

        Parent->Threads.push_back(Thread);
        return Thread;
    }

    PCB *Task::CreateProcess(PCB *Parent,
                             const char *Name,
                             TaskTrustLevel TrustLevel)
    {
        SmartCriticalSection(TaskingLock);
        PCB *Process = new PCB;
        Process->ID = this->NextPID++;
        strcpy(Process->Name, Name);
        if (Parent == nullptr)
            Process->Parent = this->GetCurrentProcess();
        else
            Process->Parent = Parent;
        Process->ExitCode = 0xdeadbeef;
        Process->Status = TaskStatus::Ready;

        Process->Security.TrustLevel = TrustLevel;
        // Process->Security.UniqueToken = SecurityManager.CreateToken();

        Process->IPCHandles = new HashMap<InterProcessCommunication::IPCPort, uint64_t>;

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
            Process->PageTable = (Memory::PageTable *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
            memset(Process->PageTable, 0, PAGE_SIZE);
            CPU::x64::CR3 cr3 = CPU::x64::readcr3();
            memcpy(Process->PageTable, (void *)cr3.raw, PAGE_SIZE);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
            SecurityManager.TrustToken(Process->Security.UniqueToken, TokenTrustLevel::Untrusted);
#if defined(__amd64__)
            Process->PageTable = (Memory::PageTable *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
            // TODO: Do mapping for page table
            fixme("User process page table mapping not implemented.");
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

        Process->Info.SpawnTime = 0;
        Process->Info.UsedTime = 0;
        Process->Info.OldUsedTime = 0;
        Process->Info.OldSystemTime = 0;
        Process->Info.CurrentSystemTime = 0;
        Process->Info.Year = 0;
        Process->Info.Month = 0;
        Process->Info.Day = 0;
        Process->Info.Hour = 0;
        Process->Info.Minute = 0;
        Process->Info.Second = 0;
        for (int i = 0; i < MAX_CPU; i++)
        {
            Process->Info.Usage[i] = 0;
            Process->Info.Affinity[i] = 0;
        }
        Process->Info.Priority = 0;

        debug("Created process %d(%s) in process %d(%s)", Process->ID, Process->Name, Parent ? Process->Parent->ID : 0, Parent ? Process->Parent->Name : "None");

        if (Parent)
            Parent->Children.push_back(Process);
        ListProcess.push_back(Process);
        return Process;
    }

    Task::Task(const IP EntryPoint) : Interrupts::Handler(CPU::x64::IRQ16)
    {
        SmartCriticalSection(TaskingLock);
        for (int i = 0; i < SMP::CPUCores; i++)
            ((APIC::APIC *)Interrupts::apic[i])->RedirectIRQ(i, CPU::x64::IRQ16 - CPU::x64::IRQ0, 1);

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
        TCB *kthrd = CreateThread(kproc, EntryPoint, 0, Arch);
        kthrd->Rename("Main Thread");
        debug("Created Kernel Process: %s and Thread: %s", kproc->Name, kthrd->Name);
        TaskingLock.Lock(__FUNCTION__);

#if defined(__amd64__) || defined(__i386__)
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

        for (int i = 0; i < SMP::CPUCores; i++)
        {
            /* do stuff i guess */
            ((APIC::Timer *)Interrupts::apicTimer[i])->OneShot(CPU::x64::IRQ16, 200);
        }
        // ((APIC::Timer *)Interrupts::apicTimer[0])->OneShot(CPU::x64::IRQ16, 100);
#endif
        TaskingLock.Unlock();
        this->IPCManager = new InterProcessCommunication::IPC;
        TaskingLock.Lock(__FUNCTION__);
        debug("Tasking Started");
    }

    Task::~Task()
    {
        SmartCriticalSection(TaskingLock);
        trace("Stopping tasking");
    }
}
