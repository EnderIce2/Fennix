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
#include "../Architecture/i686/cpu/apic.hpp"
#elif defined(aa64)
#endif

// #define DEBUG_TASKING 1

#ifdef DEBUG_TASKING
#define tskdbg(m, ...)       \
    debug(m, ##__VA_ARGS__); \
    __sync
#else
#define tskdbg(m, ...)
#endif

NewLock(TaskingLock);

namespace Tasking
{
    void Task::Schedule()
    {
        if (!StopScheduler)
            TaskingScheduler_OneShot(1);
        // APIC::InterruptCommandRegisterLow icr;
        // icr.Vector = CPU::x86::IRQ16;
        // icr.Level = APIC::APICLevel::Assert;
        // ((APIC::APIC *)Interrupts::apic[0])->IPI(GetCurrentCPU()->ID, icr);
    }

    __naked __used __no_stack_protector NIF void IdleProcessLoop()
    {
#if defined(a64) || defined(a32)
        asmv("IdleLoop:\n"
             "hlt\n"
             "jmp IdleLoop\n");
#elif defined(aa64)
        asmv("IdleLoop:\n"
             "wfe\n"
             "b IdleLoop\n");
#endif
    }

    SafeFunction NIF bool Task::InvalidPCB(PCB *pcb)
    {
        if (!pcb)
            return true;
        if (pcb >= (PCB *)(UINTPTR_MAX - 0x1ffe)) /* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
            return true;
        if (pcb < (PCB *)(0x1000)) /* In this section of the memory is reserved by the kernel. */
            return true;
        if (!Memory::Virtual().Check((void *)pcb)) /* Check if it's mapped. */
            return true;
        return false;
    }

    SafeFunction NIF bool Task::InvalidTCB(TCB *tcb)
    {
        if (!tcb)
            return true;
        if (tcb >= (TCB *)(UINTPTR_MAX - 0x1ffe)) /* Uninitialized pointers may have uintptr_t max value instead of nullptr. */
            return true;
        if (tcb < (TCB *)(0x1000)) /* In this section of the memory is reserved by the kernel. */
            return true;
        if (!Memory::Virtual().Check((void *)tcb)) /* Check if it's mapped. */
            return true;
        return false;
    }

    SafeFunction NIF void Task::RemoveThread(TCB *Thread)
    {
        for (size_t i = 0; i < Thread->Parent->Threads.size(); i++)
            if (Thread->Parent->Threads[i] == Thread)
            {
                trace("Thread \"%s\"(%d) removed from process \"%s\"(%d)",
                      Thread->Name, Thread->ID, Thread->Parent->Name, Thread->Parent->ID);
                // Free memory
                delete Thread->Stack, Thread->Stack = nullptr;
                delete Thread->Memory, Thread->Memory = nullptr;
                SecurityManager.DestroyToken(Thread->Security.UniqueToken);
                delete Thread->Parent->Threads[i], Thread->Parent->Threads[i] = nullptr;
                // Remove from the list
                Thread->Parent->Threads.remove(i);
                break;
            }
    }

    SafeFunction NIF void Task::RemoveProcess(PCB *Process)
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
                    delete ListProcess[i]->IPC, ListProcess[i]->IPC = nullptr;
                    delete ListProcess[i]->ELFSymbolTable, ListProcess[i]->ELFSymbolTable = nullptr;
                    SecurityManager.DestroyToken(ListProcess[i]->Security.UniqueToken);
                    if (ListProcess[i]->Security.TrustLevel == TaskTrustLevel::User)
                        KernelAllocator.FreePages((void *)ListProcess[i]->PageTable, TO_PAGES(PAGE_SIZE));

                    // Remove the process from parent's children list
                    if (ListProcess[i]->Parent)
                        for (size_t j = 0; j < ListProcess[i]->Parent->Children.size(); j++)
                        {
                            if (ListProcess[i]->Parent->Children[j] == ListProcess[i])
                            {
                                ListProcess[i]->Parent->Children.remove(j);
                                break;
                            }
                        }

                    // Delete process directory
                    vfs->Delete(ListProcess[i]->ProcessDirectory, true);

                    // Free memory
                    delete ListProcess[i], ListProcess[i] = nullptr;
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

    SafeFunction NIF void Task::UpdateUserTime(TaskInfo *Info)
    {
        // TODO
        Info->UserTime++;
    }

    SafeFunction NIF void Task::UpdateKernelTime(TaskInfo *Info)
    {
        // TODO
        Info->KernelTime++;
    }

    SafeFunction NIF void Task::UpdateUsage(TaskInfo *Info, int Core)
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

    void ThreadDoExit()
    {
        // TODO: How I can lock the scheduler without causing a deadlock?
        CPUData *CPUData = GetCurrentCPU();
        CPUData->CurrentThread->Status = TaskStatus::Terminated;
        debug("\"%s\"(%d) exited with code: %#lx", CPUData->CurrentThread->Name, CPUData->CurrentThread->ID, CPUData->CurrentThread->ExitCode);
        CPU::Halt(true);
    }

    PCB *Task::GetCurrentProcess() { return GetCurrentCPU()->CurrentProcess.Load(); }
    TCB *Task::GetCurrentThread() { return GetCurrentCPU()->CurrentThread.Load(); }

    PCB *Task::GetProcessByID(UPID ID)
    {
        for (size_t i = 0; i < ListProcess.size(); i++)
            if (ListProcess[i]->ID == ID)
                return ListProcess[i];
        return nullptr;
    }

    TCB *Task::GetThreadByID(UTID ID)
    {
        for (size_t i = 0; i < ListProcess.size(); i++)
            for (size_t j = 0; j < ListProcess[i]->Threads.size(); j++)
                if (ListProcess[i]->Threads[j]->ID == ID)
                    return ListProcess[i]->Threads[j];
        return nullptr;
    }

    void Task::WaitForProcess(PCB *pcb)
    {
        if (InvalidPCB(pcb))
            return;
        if (pcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for process \"%s\"(%d)", pcb->Name, pcb->ID);
        while (pcb->Status != TaskStatus::Terminated)
            CPU::Pause();
    }

    void Task::WaitForThread(TCB *tcb)
    {
        if (InvalidTCB(tcb))
            return;
        if (tcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for thread \"%s\"(%d)", tcb->Name, tcb->ID);
        while (tcb->Status != TaskStatus::Terminated)
            CPU::Pause();
    }

    void Task::WaitForProcessStatus(PCB *pcb, TaskStatus status)
    {
        if (InvalidPCB(pcb))
            return;
        if (pcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for process \"%s\"(%d) to reach status: %d", pcb->Name, pcb->ID, status);
        while (pcb->Status != status)
            CPU::Pause();
    }

    void Task::WaitForThreadStatus(TCB *tcb, TaskStatus status)
    {
        if (InvalidTCB(tcb))
            return;
        if (tcb->Status == TaskStatus::UnknownStatus)
            return;
        debug("Waiting for thread \"%s\"(%d) to reach status: %d", tcb->Name, tcb->ID, status);
        while (tcb->Status != status)
            CPU::Pause();
    }

    void Task::Sleep(uint64_t Milliseconds)
    {
        SmartLock(TaskingLock);
        TCB *thread = this->GetCurrentThread();
        thread->Status = TaskStatus::Sleeping;
        if (thread->Parent->Threads.size() == 1)
            thread->Parent->Status = TaskStatus::Sleeping;
        thread->Info.SleepUntil = TimeManager->CalculateTarget(Milliseconds);
        tskdbg("Thread \"%s\"(%d) is going to sleep until %llu", thread->Name, thread->ID, thread->Info.SleepUntil);
        // TaskingScheduler_OneShot(1);
        // IRQ16
        TaskingLock.Unlock();
        asmv("int $0x30"); /* This will trigger the IRQ16 instantly so we won't execute the next instruction */
    }

    void Task::SignalShutdown()
    {
        fixme("SignalShutdown()");
        // TODO: Implement this
        // This should hang until all processes are terminated
    }

    void Task::RevertProcessCreation(PCB *Process)
    {
        for (size_t i = 0; i < ListProcess.size(); i++)
        {
            if (ListProcess[i] == Process)
            {
                SecurityManager.DestroyToken(Process->Security.UniqueToken);
                if (Process->Security.TrustLevel == TaskTrustLevel::User)
                    KernelAllocator.FreePages((void *)Process->PageTable, TO_PAGES(PAGE_SIZE));

                if (Process->Parent)
                    for (size_t j = 0; j < Process->Parent->Children.size(); j++)
                    {
                        if (Process->Parent->Children[j] == Process)
                        {
                            Process->Parent->Children.remove(j);
                            break;
                        }
                    }

                delete Process->IPC, Process->IPC = nullptr;
                delete Process->ELFSymbolTable, Process->ELFSymbolTable = nullptr;
                delete Process, Process = nullptr;
                ListProcess.remove(i);
                NextPID--;
                break;
            }
        }
    }

    void Task::RevertThreadCreation(TCB *Thread)
    {
        for (size_t j = 0; j < Thread->Parent->Threads.size(); j++)
        {
            if (Thread->Parent->Threads[j] == Thread)
            {
                Thread->Parent->Threads.remove(j);
                break;
            }
        }

        delete Thread->Stack, Thread->Stack = nullptr;
        delete Thread->Memory, Thread->Memory = nullptr;
        SecurityManager.DestroyToken(Thread->Security.UniqueToken);
        delete Thread, Thread = nullptr;
        NextTID--;
    }

    TCB *Task::CreateThread(PCB *Parent,
                            IP EntryPoint,
                            const char **argv,
                            const char **envp,
                            const std::vector<AuxiliaryVector> &auxv,
                            IPOffset Offset,
                            TaskArchitecture Architecture,
                            TaskCompatibility Compatibility)
    {
        SmartLock(TaskingLock);
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
        Thread->Memory = new Memory::MemMgr(Parent->PageTable, Parent->memDirectory);
        Thread->FPU = (CPU::x64::FXState *)Thread->Memory->RequestPages(TO_PAGES(sizeof(CPU::x64::FXState)));
        memset(Thread->FPU, 0, FROM_PAGES(TO_PAGES(sizeof(CPU::x64::FXState))));

        Thread->Security.TrustLevel = Parent->Security.TrustLevel;
        Thread->Security.UniqueToken = SecurityManager.CreateToken();

        // TODO: Is really a good idea to use the FPU in kernel mode?
        Thread->FPU->mxcsr = 0b0001111110000000;
        Thread->FPU->mxcsrmask = 0b1111111110111111;
        Thread->FPU->fcw = 0b0000001100111111;

        CPU::x64::fxrstor(Thread->FPU);
        // uint16_t FCW = 0b1100111111;
        // asmv("fldcw %0"
        //      :
        //      : "m"(FCW)
        //      : "memory");
        // uint32_t MXCSR = 0b1111110000000;
        // asmv("ldmxcsr %0"
        //      :
        //      : "m"(MXCSR)
        //      : "memory");
        // CPU::x64::fxsave(Thread->FPU);

#if defined(a64)
        memset(&Thread->Registers, 0, sizeof(CPU::x64::TrapFrame)); // Just in case
        Thread->Registers.rip = (EntryPoint + Offset);
#elif defined(a32)
#elif defined(aa64)
#endif
        switch (Parent->Security.TrustLevel)
        {
        case TaskTrustLevel::System:
            warn("Trust level not supported.");
            [[fallthrough]];
        case TaskTrustLevel::Kernel:
        {
            Thread->Stack = new Memory::StackGuard(false, Parent->PageTable);
#if defined(a64)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TTL::TrustedByKernel);
            Thread->GSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_GS_BASE);
            Thread->FSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_FS_BASE);
            Thread->Registers.cs = GDT_KERNEL_CODE;
            Thread->Registers.ss = GDT_KERNEL_DATA;
            Thread->Registers.rflags.AlwaysOne = 1;
            Thread->Registers.rflags.IF = 1;
            Thread->Registers.rflags.ID = 1;
            Thread->Registers.rsp = ((uintptr_t)Thread->Stack->GetStackTop());
            POKE(uintptr_t, Thread->Registers.rsp) = (uintptr_t)ThreadDoExit;
#elif defined(a32)
#elif defined(aa64)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
            Thread->Stack = new Memory::StackGuard(true, Parent->PageTable);
#if defined(a64)
            SecurityManager.TrustToken(Thread->Security.UniqueToken, TTL::Untrusted);
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

#pragma region

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
            uintptr_t *ArgvStrings = new uintptr_t[ArgvSize];
            uintptr_t *EnvpStrings = new uintptr_t[EnvpSize];

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

            // auxv_array is initialized with auxv elements. If the array is empty then we add a null terminator
            std::vector<AuxiliaryVector> auxv_array = auxv;
            if (auxv_array.size() == 0)
                auxv_array.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});

            // Store auxillary vector
            foreach (AuxiliaryVector var in auxv_array)
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

            delete[] ArgvStrings;
            delete[] EnvpStrings;

#ifdef DEBUG
            DumpData("Stack Data", (void *)((uintptr_t)Thread->Stack->GetStackPhysicalTop() - (uintptr_t)SubtractStack), SubtractStack);
#endif

            Thread->Registers.rdi = (uintptr_t)ArgvSize;                                         // argc
            Thread->Registers.rsi = (uintptr_t)(Thread->Registers.rsp + 8);                      // argv
            Thread->Registers.rcx = (uintptr_t)EnvpSize;                                         // envc
            Thread->Registers.rdx = (uintptr_t)(Thread->Registers.rsp + 8 + (8 * ArgvSize) + 8); // envp

#pragma endregion

            /* We need to leave the libc's crt to make a syscall when the Thread is exited or we are going to get GPF or PF exception. */

            Memory::Virtual uva = Memory::Virtual(Parent->PageTable);
            if (!uva.Check((void *)Offset, Memory::PTFlag::US))
            {
                error("Offset is not user accessible");
                uva.Map((void *)Offset, (void *)Offset, Memory::PTFlag::RW | Memory::PTFlag::US); // We try one more time.
            }
#elif defined(a32)
#elif defined(aa64)
#endif
#ifdef DEBUG_TASKING
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
        Thread->Info.Priority = TaskPriority::Normal;
        Thread->Info.Architecture = Architecture;
        Thread->Info.Compatibility = Compatibility;

#ifdef DEBUG
#ifdef a64
        debug("Thread offset is %#lx (EntryPoint: %#lx) => RIP: %#lx", Thread->Offset, Thread->EntryPoint, Thread->Registers.rip);
        if (Parent->Security.TrustLevel == TaskTrustLevel::User)
            debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.rsp);
        else
            debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.rsp);
#elif defined(a32)
        debug("Thread offset is %#lx (EntryPoint: %#lx) => RIP: %#lx", Thread->Offset, Thread->EntryPoint, Thread->Registers.eip);
        if (Parent->Security.TrustLevel == TaskTrustLevel::User)
            debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.esp);
        else
            debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx", Thread->Stack->GetStackBottom(), Thread->Stack->GetStackTop(), Thread->Registers.esp);
#elif defined(aa64)
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
                             TaskTrustLevel TrustLevel, void *Image,
                             bool DoNotCreatePageTable)
    {
        SmartLock(TaskingLock);
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
        Process->Security.UniqueToken = SecurityManager.CreateToken();

        char ProcFSName[16];
        sprintf(ProcFSName, "%ld", Process->ID);
        Process->ProcessDirectory = vfs->Create(ProcFSName, VirtualFileSystem::NodeFlags::DIRECTORY, ProcFS);
        Process->memDirectory = vfs->Create("mem", VirtualFileSystem::NodeFlags::DIRECTORY, Process->ProcessDirectory);
        Process->IPC = new InterProcessCommunication::IPC((void *)Process);

        switch (TrustLevel)
        {
        case TaskTrustLevel::System:
            warn("Trust level not supported.");
            [[fallthrough]];
        case TaskTrustLevel::Kernel:
        {
            SecurityManager.TrustToken(Process->Security.UniqueToken, TTL::TrustedByKernel);
#if defined(a64)
            if (!DoNotCreatePageTable)
                Process->PageTable = (Memory::PageTable4 *)CPU::x64::readcr3().raw;
#elif defined(a32)
#elif defined(aa64)
#endif
            break;
        }
        case TaskTrustLevel::User:
        {
            SecurityManager.TrustToken(Process->Security.UniqueToken, TTL::Untrusted);
#if defined(a64)
            if (!DoNotCreatePageTable)
            {
                Process->PageTable = (Memory::PageTable4 *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
                memcpy(Process->PageTable, (void *)UserspaceKernelOnlyPageTable, PAGE_SIZE);
                for (size_t i = 0; i < TO_PAGES(PAGE_SIZE); i++)
                    Memory::Virtual(Process->PageTable).Map((void *)Process->PageTable, (void *)Process->PageTable, Memory::PTFlag::RW); // Make sure the page table is mapped.
            }
#elif defined(a32)
#elif defined(aa64)
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
        Process->Info.Priority = TaskPriority::Normal;

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

    Task::Task(const IP EntryPoint) : Interrupts::Handler(16) /* IRQ16 */
    {
        SmartLock(TaskingLock);
#if defined(a64)
        // Map the IRQ16 to the first CPU.
        ((APIC::APIC *)Interrupts::apic[0])->RedirectIRQ(0, CPU::x86::IRQ16 - CPU::x86::IRQ0, 1);
#elif defined(a32)
#elif defined(aa64)
#endif
        KPrint("Starting Tasking With Instruction Pointer: %p (\e666666%s\eCCCCCC)", EntryPoint, KernelSymbolTable->GetSymbolFromAddress(EntryPoint));
        TaskingLock.Unlock();

#if defined(a64)
        TaskArchitecture Arch = TaskArchitecture::x64;
#elif defined(a32)
        TaskArchitecture Arch = TaskArchitecture::x32;
#elif defined(aa64)
        TaskArchitecture Arch = TaskArchitecture::ARM64;
#endif
        PCB *kproc = CreateProcess(nullptr, "Kernel", TaskTrustLevel::Kernel);
        TCB *kthrd = CreateThread(kproc, EntryPoint, nullptr, nullptr, std::vector<AuxiliaryVector>(), 0, Arch);
        kthrd->Rename("Main Thread");
        debug("Created Kernel Process: %s and Thread: %s", kproc->Name, kthrd->Name);
        TaskingLock.Lock(__FUNCTION__);

        bool MONITORSupported = false;
        if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_AMD) == 0)
        {
#if defined(a64)
            CPU::x64::AMD::CPUID0x1 cpuid1amd;
#elif defined(a32)
            CPU::x32::AMD::CPUID0x1 cpuid1amd;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1amd.EAX.raw), "=b"(cpuid1amd.EBX.raw), "=c"(cpuid1amd.ECX.raw), "=d"(cpuid1amd.EDX.raw)
                 : "a"(0x1));
#endif
            MONITORSupported = cpuid1amd.ECX.MONITOR;
        }
        else if (strcmp(CPU::Vendor(), x86_CPUID_VENDOR_INTEL) == 0)
        {
#if defined(a64)
            CPU::x64::Intel::CPUID0x1 cpuid1intel;
#elif defined(a32)
            CPU::x32::Intel::CPUID0x1 cpuid1intel;
#endif
#if defined(a64) || defined(a32)
            asmv("cpuid"
                 : "=a"(cpuid1intel.EAX.raw), "=b"(cpuid1intel.EBX.raw), "=c"(cpuid1intel.ECX.raw), "=d"(cpuid1intel.EDX.raw)
                 : "a"(0x1));
#endif
            MONITORSupported = cpuid1intel.ECX.MONITOR;
        }

        if (MONITORSupported)
        {
            trace("CPU has MONITOR/MWAIT support.");
        }

        if (!CPU::Interrupts(CPU::Check))
        {
            error("Interrupts are not enabled.");
            CPU::Interrupts(CPU::Enable);
        }

        TaskingLock.Unlock();
        IdleProcess = CreateProcess(nullptr, (char *)"Idle", TaskTrustLevel::Kernel);
        for (int i = 0; i < SMP::CPUCores; i++)
        {
            IdleThread = CreateThread(IdleProcess, reinterpret_cast<uintptr_t>(IdleProcessLoop));
            char IdleName[16];
            sprintf(IdleName, "Idle Thread %d", i);
            IdleThread->Rename(IdleName);
            IdleThread->SetPriority(Idle);
            for (int j = 0; j < MAX_CPU; j++)
                IdleThread->Info.Affinity[j] = false;
            IdleThread->Info.Affinity[i] = true;
        }
        debug("Tasking Started");
#if defined(a64)
        ((APIC::Timer *)Interrupts::apicTimer[0])->OneShot(CPU::x86::IRQ16, 100);

        /* FIXME: The kernel is not ready for multi-core tasking. */
        // for (int i = 1; i < SMP::CPUCores; i++)
        // {
        //     ((APIC::Timer *)Interrupts::apicTimer[i])->OneShot(CPU::x86::IRQ16, 100);
        //     APIC::InterruptCommandRegisterLow icr;
        //     icr.Vector = CPU::x86::IRQ16;
        //     icr.Level = APIC::APICLevel::Assert;
        //     ((APIC::APIC *)Interrupts::apic[0])->IPI(i, icr);
        // }
#elif defined(a32)
#elif defined(aa64)
#endif
    }

    Task::~Task()
    {
        debug("Destructor called");
        SmartLock(TaskingLock);
        trace("Stopping tasking");
        foreach (PCB *Process in ListProcess)
        {
            foreach (TCB *Thread in Process->Threads)
                Thread->Status = TaskStatus::Terminated;

            Process->Status = TaskStatus::Terminated;
        }

        TaskingLock.Unlock();

        while (ListProcess.size() > 0)
        {
            trace("Waiting for %d processes to terminate", ListProcess.size());
            int NotTerminated = 0;
            foreach (PCB *Process in ListProcess)
            {
                debug("Process %s(%d) is still running (or waiting to be removed status %#lx)", Process->Name, Process->ID, Process->Status);
                if (Process->Status == TaskStatus::Terminated)
                    continue;
                NotTerminated++;
            }
            if (NotTerminated == 0)
                break;
            TaskingScheduler_OneShot(100);
        }

        trace("Tasking stopped");
    }
}
