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
