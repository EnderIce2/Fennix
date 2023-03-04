#include <syscalls.hpp>

#include <debug.h>

#include "../kernel.h"

NewLock(SyscallsLock);

extern "C" uintptr_t SystemCallsHandler(SyscallsFrame *Frame)
{
    CPU::Interrupts(CPU::Enable);
    SmartLock(SyscallsLock); /* TODO: This should be replaced or moved somewhere else. */

#if defined(a64)
    switch (TaskManager->GetCurrentThread()->Info.Compatibility)
    {
    case Tasking::TaskCompatibility::Native:
        return HandleNativeSyscalls(Frame);
    case Tasking::TaskCompatibility::Linux:
        return HandleLinuxSyscalls(Frame);
    case Tasking::TaskCompatibility::Windows:
    {
        error("Windows compatibility not implemented yet.");
        break;
    }
    default:
    {
        error("Unknown compatibility mode! Killing thread...");
        TaskManager->KillThread(TaskManager->GetCurrentThread(), -0xCA11);
        break;
    }
    }
#elif defined(a32)
    fixme("System call %lld", Frame->eax);
#elif defined(aa64)
    fixme("System call");
#endif
    return -1;
}
