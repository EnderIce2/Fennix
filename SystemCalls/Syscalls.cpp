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
    SmartLock(SyscallsLock); /* TODO: This should be replaced or moved somewhere else. */

    Tasking::TaskInfo *Ptinfo = &TaskManager->GetCurrentProcess()->Info;
    Tasking::TaskInfo *Ttinfo = &TaskManager->GetCurrentThread()->Info;
    uint64_t TempTimeCalc = TimeManager->GetCounter();

    switch (Ttinfo->Compatibility)
    {
    case Tasking::TaskCompatibility::Native:
    {
        uintptr_t ret = HandleNativeSyscalls(Frame);
        Ptinfo->KernelTime += TimeManager->GetCounter() - TempTimeCalc;
        Ttinfo->KernelTime += TimeManager->GetCounter() - TempTimeCalc;
        return ret;
    }
    case Tasking::TaskCompatibility::Linux:
    {
        uintptr_t ret = HandleLinuxSyscalls(Frame);
        Ptinfo->KernelTime += TimeManager->GetCounter() - TempTimeCalc;
        Ttinfo->KernelTime += TimeManager->GetCounter() - TempTimeCalc;
        return ret;
    }
    case Tasking::TaskCompatibility::Windows:
    {
        error("Windows compatibility not implemented yet.");
        break;
    }
    default:
    {
        error("Unknown compatibility mode! Killing thread...");
        TaskManager->KillThread(TaskManager->GetCurrentThread(), Tasking::KILL_SYSCALL);
        break;
    }
    }
    assert(false); /* Should never reach here. */
    return 0;
}
