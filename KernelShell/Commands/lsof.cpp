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

#include "../../kernel.h"

void cmd_lsof(const char *)
{
	printf("PROCESS  FD    NAME\n");
	foreach (auto Proc in TaskManager->GetProcessList())
	{
		if (!Proc)
			continue;

		std::vector<VirtualFileSystem::FileDescriptorTable::FileDescriptor> fds_array = Proc->FileDescriptors->GetFileDescriptors();
		foreach (auto fd in fds_array)
			printf("%s %d: %s\n", Proc->Name, fd.Descriptor,
				   fd.Handle->AbsolutePath.c_str());
	}
}
