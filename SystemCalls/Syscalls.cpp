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

class AutoSwitchPageTable
{
private:
	uintptr_t Original;

public:
	AutoSwitchPageTable()
	{
#if defined(a86)
		asmv("mov %%cr3, %0"
			 : "=r"(Original));

		asmv("mov %0, %%cr3"
			 :
			 : "r"(KernelPageTable));
#endif
	}

	~AutoSwitchPageTable()
	{
#if defined(a86)
		asmv("mov %0, %%cr3"
			 :
			 : "r"(Original));
#endif
	}
};

extern "C" uintptr_t SystemCallsHandler(SyscallsFrame *Frame)
{
	/* Automatically switch to kernel page table
		and switch back when this function returns. */
	AutoSwitchPageTable PageSwitcher;

	uint64_t TempTimeCalc = TimeManager->GetCounter();
	Tasking::TaskInfo *Ptinfo = &thisProcess->Info;
	Tasking::TaskInfo *Ttinfo = &thisThread->Info;

	switch (Ttinfo->Compatibility)
	{
	case Tasking::TaskCompatibility::Native:
	{
		uintptr_t ret = 0;
		if (Config.UseLinuxSyscalls)
			ret = HandleLinuxSyscalls(Frame);
		else
			ret = HandleNativeSyscalls(Frame);
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
		TaskManager->KillThread(thisThread, Tasking::KILL_SYSCALL);
		break;
	}
	}
	assert(false); /* Should never reach here. */
	return 0;
}
