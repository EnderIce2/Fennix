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

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/munmap.html */
int sys_munmap(SysFrm *,
			   void *addr, size_t len)
{
	if (uintptr_t(addr) % PAGE_SIZE)
		return -EINVAL;

	if (len == 0)
		return -EINVAL;

	PCB *pcb = thisProcess;
	VirtualMemoryArea *vma = pcb->vma;
	Virtual vmm = Virtual(pcb->PageTable);

	for (uintptr_t i = uintptr_t(addr);
		 i < uintptr_t(addr) + len;
		 i += PAGE_SIZE)
	{
		if (likely(!vmm.Check((void *)i, G)))
			vmm.Remap((void *)i, (void *)i, P | RW);
		else
			warn("%p is a global page", (void *)i);
	}

	/* TODO: Check if the page is allocated
				and not only mapped */
	vma->FreePages((void *)addr, TO_PAGES(len) + 1);
	return 0;
}
