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

using namespace VirtualFileSystem;
using namespace Tasking;

void cmd_top(const char *)
{
	printf("\e9400A1PID    \e9CA100Name                \e00A15BState    \eCCCCCCPriority    Memory Usage    CPU Usage\n");
	foreach (auto Proc in TaskManager->GetProcessList())
	{
		printf("\e9400A1%-4d \e9CA100%-20s \e00A15B%s       \eCCCCCC%d           %ld            %ld\n",
			   Proc->ID, Proc->Name, Proc->Status == Running ? "Running" : "Stopped",
			   Proc->Info.Priority, Proc->Memory->GetAllocatedMemorySize(),
			   Proc->Info.UserTime + Proc->Info.KernelTime);

		foreach (auto Thrd in Proc->Threads)
		{
			printf(" \eA80011%-4d \e9CA100%-20s \e00A15B%s       \eCCCCCC%d           %ld            %ld\n",
				   Thrd->ID, Thrd->Name, Thrd->Status == Running ? "Running" : "Stopped",
				   Thrd->Info.Priority, Thrd->Memory->GetAllocatedMemorySize(),
				   Thrd->Info.UserTime + Thrd->Info.KernelTime);
		}
	}
}
