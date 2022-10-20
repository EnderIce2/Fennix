#include <task.hpp>
#include <debug.h>

namespace Tasking
{
    PCB *Task::CreateProcess(char *Name, ExecutionElevation Elevation)
    {
        PCB *Process = new PCB;
        return Process;
    }

    TCB *Task::CreateThread(PCB *Parent, IP EntryPoint)
    {
        TCB *Thread = new TCB;
        return Thread;
    }

    Task::Task(const IP EntryPoint)
    {
        trace("Starting tasking with IP: %#lx", EntryPoint);
    }

    Task::~Task()
    {
    }
}
