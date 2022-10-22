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

#define DEBUG_SCHEDULER 1

#ifdef DEBUG_SCHEDULER
#define schedbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define schedbg(m, ...)
#endif

NewLock(TaskingLock);

namespace Tasking
{
    extern "C" void OneShot(int TimeSlice)
    {
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

    Vector<PCB *> ListProcess;
    PCB *IdleProcess = nullptr;
    TCB *IdleThread = nullptr;

#if defined(__amd64__)
    __attribute__((no_stack_protector)) void Task::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
    {
        fixme("unimplemented");
        OneShot(500); // test
    }
#elif defined(__i386__)
    __attribute__((no_stack_protector)) void Task::OnInterruptReceived(void *Frame)
    {
        fixme("unimplemented");
    }
#elif defined(__aarch64__)
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
        CPU::Stop();
    }

    PCB *Task::GetCurrentProcess()
    {
        SmartCriticalSection(TaskingLock);
        return GetCurrentCPU()->CurrentProcess;
    }

    TCB *Task::GetCurrentThread()
    {
        SmartCriticalSection(TaskingLock);
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
        Thread->ID = this->NextTID++;
        strcpy(Thread->Name, Parent->Name);
        Thread->Parent = Parent;
        Thread->EntryPoint = EntryPoint;
        Thread->Offset = Offset;
        Thread->ExitCode = 0xdeadbeef;
        Thread->Stack = (void *)((uint64_t)KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE)) + STACK_SIZE);
        Thread->Status = TaskStatus::Ready;

        memset(&Thread->Registers, 0, sizeof(ThreadFrame)); // Just in case
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
            delete Thread;
            return nullptr;
        }
        }

        Thread->Security.TrustLevel = Parent->Security.TrustLevel;
        Thread->Security.UniqueToken = SecurityManager.CreateToken();

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
        Process->Parent = Parent;
        Process->ExitCode = 0xdeadbeef;
        Process->Status = TaskStatus::Ready;

        Process->Security.TrustLevel = TrustLevel;
        Process->Security.UniqueToken = SecurityManager.CreateToken();

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

        Parent->Children.push_back(Process);
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
        TaskingLock.Lock();
#if defined(__amd64__) || defined(__i386__)
        for (int i = 0; i < SMP::CPUCores; i++)
        {
            /* do stuff i guess */
            ((APIC::Timer *)Interrupts::apicTimer[i])->OneShot(CPU::x64::IRQ16, 100);
        }
#endif
        debug("Tasking Started");
    }

    Task::~Task()
    {
        SmartCriticalSection(TaskingLock);
        trace("Stopping tasking");
    }
}
