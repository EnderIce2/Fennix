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

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/write.html */
ssize_t sys_write(SysFrm *, int fildes,
				  const void *buf, size_t nbyte)
{
	const void *safe_buf = nullptr;
	PCB *pcb = thisProcess;
	SmartHeap sh(nbyte, pcb->vma);
	safe_buf = sh.Get();
	{
		SwapPT swap(pcb->PageTable);
		memcpy((void *)safe_buf, buf, nbyte);
	}

	function("%d, %p, %d", fildes, buf, nbyte);
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t ret = fdt->_write(fildes, safe_buf, nbyte);
	if (ret)
		fdt->_lseek(fildes, ret, SEEK_CUR);
	return ret;
}
