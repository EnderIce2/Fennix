#include <task.hpp>
#include <debug.h>
#include <smp.hpp>
#include <lock.hpp>

NewLock(TaskingLock);

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

namespace Tasking
{
#if defined(__amd64__)
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

    void a()
    {
        return;
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
                             char *Name,
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
        trace("Starting tasking with IP: %#lx", EntryPoint);
    }

    Task::~Task()
    {
        SmartCriticalSection(TaskingLock);
        trace("Stopping tasking");
    }
}
