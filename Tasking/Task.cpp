#include <task.hpp>

#include <interrupts.hpp>
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

    extern "C" __attribute__((naked, used, no_stack_protector)) void IdleProcessLoop()
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

    /** @brief Called by the IDT (IRQ16 for x64 and x32) */
    extern "C" __attribute__((naked, used, no_stack_protector)) void SchedulerInterruptStub()
    {
#if defined(__amd64__)
        asm("cld\n"
            "pushq %rax\n"
            "pushq %rbx\n"
            "pushq %rcx\n"
            "pushq %rdx\n"
            "pushq %rsi\n"
            "pushq %rdi\n"
            "pushq %rbp\n"
            "pushq %r8\n"
            "pushq %r9\n"
            "pushq %r10\n"
            "pushq %r11\n"
            "pushq %r12\n"
            "pushq %r13\n"
            "pushq %r14\n"
            "pushq %r15\n"
            "movq %ds, %rax\n"
            "pushq %rax\n"
            "movq %rsp, %rdi\n"
            "call SchedulerInterruptHandler\n"
            "popq %rax\n" // Pop the DS register
            "popq %r15\n"
            "popq %r14\n"
            "popq %r13\n"
            "popq %r12\n"
            "popq %r11\n"
            "popq %r10\n"
            "popq %r9\n"
            "popq %r8\n"
            "popq %rbp\n"
            "popq %rdi\n"
            "popq %rsi\n"
            "popq %rdx\n"
            "popq %rcx\n"
            "popq %rbx\n"
            "popq %rax\n"
            "addq $16, %rsp\n"
            "iretq");
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }

    extern "C" __attribute__((no_stack_protector)) void SchedulerInterruptHandler(ThreadFrame *Frame)
    {
        fixme("SchedulerInterruptHandler: %d", GetCurrentCPU()->ID);
#if defined(__amd64__)
        ((APIC::APIC *)Interrupts::apic[GetCurrentCPU()->ID])->EOI();
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
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

    PCB *Task::CreateProcess(PCB *Parent,
                             const char *Name,
                             TaskElevation Elevation)
    {
        SmartCriticalSection(TaskingLock);
        PCB *Process = new PCB;
#if defined(__amd64__)
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
        return Process;
    }

    TCB *Task::CreateThread(PCB *Parent,
                            IP EntryPoint)
    {
        SmartCriticalSection(TaskingLock);
        TCB *Thread = new TCB;
#if defined(__amd64__)
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
        return Thread;
    }

    Task::Task(const IP EntryPoint)
    {
        SmartCriticalSection(TaskingLock);
        KPrint("Starting Tasking With Instruction Pointer: %p (\e666666%s\eCCCCCC)", EntryPoint, KernelSymbolTable->GetSymbolFromAddress(EntryPoint));
#if defined(__amd64__) || defined(__i386__)
        for (int i = 0; i < SMP::CPUCores; i++)
            ((APIC::APIC *)Interrupts::apic[i])->RedirectIRQ(i, CPU::x64::IRQ16 - CPU::x64::IRQ0, 1);
#endif

        TaskingLock.Unlock();
        PCB *kproc = CreateProcess(nullptr, "Kernel", TaskElevation::Kernel);
        TCB *kthrd = CreateThread(kproc, EntryPoint);
        kthrd->Rename("Main Thread");
        TaskingLock.Lock();
        OneShot(100);
        debug("Tasking Started");
    }

    Task::~Task()
    {
        SmartCriticalSection(TaskingLock);
        trace("Stopping tasking");
    }
}
