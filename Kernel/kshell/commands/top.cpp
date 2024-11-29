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

#include "../cmds.hpp"

#include <filesystem.hpp>
#include <task.hpp>

#include "../../kernel.h"

using namespace vfs;
using namespace Tasking;

const char *TaskStateStrings[] = {
	"Unknown",	// Unknown
	"Ready",	// Ready
	"Running",	// Running
	"Sleeping", // Sleeping
	"Blocked",	// Blocked
	"Stopped",	// Stopped
	"Waiting",	// Waiting

	"CoreDump",	  // Core dump
	"Zombie",	  // Zombie
	"Terminated", // Terminated
	"Frozen",	  // Frozen
};

void cmd_top(const char *)
{
	printf("PID    Name                State    Priority    Memory Usage    CPU Usage\n");
	foreach (auto Proc in TaskManager->GetProcessList())
	{
#if defined(__amd64__)
		printf("%-4d %-20s %s       %d           %ld KiB         %ld\n",
			   Proc->ID, Proc->Name, TaskStateStrings[Proc->State.load()],
			   Proc->Info.Priority, TO_KiB(Proc->GetSize()),
			   Proc->Info.UserTime + Proc->Info.KernelTime);
#elif defined(__i386__)
		printf("%-4d %-20s %s       %d           %lld KiB         %lld\n",
			   Proc->ID, Proc->Name, TaskStateStrings[Proc->State.load()],
			   Proc->Info.Priority, TO_KiB(Proc->GetSize()),
			   Proc->Info.UserTime + Proc->Info.KernelTime);
#endif

		foreach (auto Thrd in Proc->Threads)
		{
#if defined(__amd64__)
			printf(" %-4d %-20s %s       %d           %ld KiB         %ld\n",
				   Thrd->ID, Thrd->Name, TaskStateStrings[Thrd->State.load()],
				   Thrd->Info.Priority, TO_KiB(Thrd->GetSize()),
				   Thrd->Info.UserTime + Thrd->Info.KernelTime);
#elif defined(__i386__)
			printf(" %-4d %-20s %s       %d           %lld KiB         %lld\n",
				   Thrd->ID, Thrd->Name, TaskStateStrings[Thrd->State.load()],
				   Thrd->Info.Priority, TO_KiB(Thrd->GetSize()),
				   Thrd->Info.UserTime + Thrd->Info.KernelTime);
#endif
		}
	}
}
