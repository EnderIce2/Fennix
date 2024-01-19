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

#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../../syscalls.h"
#include "../../kernel.h"

using Tasking::PCB;
using namespace Memory;

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/open.html */
int sys_open(SysFrm *,
			 const char *path,
			 int oflag, mode_t mode)
{
	const char *safe_path = nullptr;
	PCB *pcb = thisProcess;
	SmartHeap sh(512, pcb->vma);
	safe_path = (const char *)sh.Get();
	{
		SwapPT swap(pcb->PageTable);
		size_t len = strlen(path);
		memcpy((void *)safe_path, path, MAX(len, size_t(511)));
	}

	function("%s, %d, %d", safe_path, oflag, mode);
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_open(safe_path, oflag, mode);
}
