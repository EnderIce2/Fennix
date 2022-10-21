#ifndef __FENNIX_KERNEL_TASKING_H__
#define __FENNIX_KERNEL_TASKING_H__

#include <types.h>

namespace Tasking
{
    typedef unsigned long IP;
    typedef unsigned long UPID;
    typedef unsigned long UTID;
    typedef unsigned long Token;

    enum TaskElevation
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
        Running,
        Sleeping,
        Waiting,
        Stopped,
        Terminated
    };

    struct TaskSecurity
    {
        TaskElevation Elevation;
        Token UniqueToken;
    };

    struct PCB
    {
        UPID PID;
        char Name[256];
        TaskSecurity Security;
        TaskStatus Status;
    };

    struct TCB
    {
        UTID TID;
        PCB *Parent;
        IP EntryPoint;
    };

    class Task
    {
    public:
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

        PCB *CreateProcess(PCB *Parent,
                           char *Name,
                           TaskElevation Elevation);

        TCB *CreateThread(PCB *Parent,
                          IP EntryPoint);

        Task(const IP EntryPoint);
        ~Task();
    };
}

#endif // !__FENNIX_KERNEL_TASKING_H__
