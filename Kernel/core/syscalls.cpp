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
	void *Original;

public:
	AutoSwitchPageTable()
	{
#if defined(__amd64__) || defined(__i386__)
		asmv("mov %%cr3, %0"
			 : "=r"(Original));

		asmv("mov %0, %%cr3"
			 :
			 : "r"(KernelPageTable));
		debug(" +    %#lx %s(%d)", Original,
			  thisProcess->Name, thisProcess->ID);
#endif
	}

	~AutoSwitchPageTable()
	{
#if defined(__amd64__) || defined(__i386__)
		debug("-    %#lx %s(%d)", Original,
			  thisProcess->Name, thisProcess->ID);
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

	uint64_t _ctime = TimeManager->GetTimeNs();
	Tasking::TaskInfo *Ptinfo = &thisProcess->Info;
	Tasking::TaskInfo *Ttinfo = &thisThread->Info;
	uintptr_t ret;

	if (Config.LinuxSubsystem)
	{
		ret = HandleLinuxSyscalls(Frame);
		goto Ret;
	}

	switch (Ttinfo->Compatibility)
	{
	case Tasking::TaskCompatibility::Native:
	{
		ret = HandleNativeSyscalls(Frame);
		break;
	}
	case Tasking::TaskCompatibility::Linux:
	{
		ret = HandleLinuxSyscalls(Frame);
		break;
	}
	case Tasking::TaskCompatibility::Windows:
	{
		error("Windows compatibility not implemented yet.");
		assert(false);
		break;
	}
	default:
	{
		error("Unknown compatibility mode!");
		assert(false);
		break;
	}
	}

Ret:
	Ptinfo->KernelTime += TimeManager->GetTimeNs() - _ctime;
	Ttinfo->KernelTime += TimeManager->GetTimeNs() - _ctime;
	return ret;
}
