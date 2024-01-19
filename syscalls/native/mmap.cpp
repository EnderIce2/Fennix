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
#include <debug.h>

#include "../../syscalls.h"
#include "../../kernel.h"

using Tasking::PCB;
using namespace Memory;

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/mmap.html */
void *sys_mmap(SysFrm *,
			   void *addr, size_t len,
			   int prot, int flags,
			   int fildes, off_t off)
{
	if (len == 0)
		return (void *)-EINVAL;

	if (fildes != -1)
		return (void *)-ENOSYS;

	bool p_None = prot & sc_PROT_NONE;
	bool p_Read = prot & sc_PROT_READ;
	bool p_Write = prot & sc_PROT_WRITE;
	bool p_Exec = prot & sc_PROT_EXEC;

	bool m_Shared = flags & sc_MAP_SHARED;
	bool m_Private = flags & sc_MAP_PRIVATE;
	bool m_Fixed = flags & sc_MAP_FIXED;
	bool m_Anon = flags & sc_MAP_ANONYMOUS;

	UNUSED(p_None);
	UNUSED(m_Anon);

	debug("N:%d R:%d W:%d E:%d",
		  p_None, p_Read, p_Write,
		  p_Exec);

	debug("S:%d P:%d F:%d A:%d",
		  m_Shared, m_Private,
		  m_Fixed, m_Anon);

	int UnknownFlags = flags & ~(sc_MAP_SHARED |
								 sc_MAP_PRIVATE |
								 sc_MAP_FIXED |
								 sc_MAP_ANONYMOUS);

	if (UnknownFlags)
	{
		debug("Unknown flags: %x", UnknownFlags);
		return (void *)-EINVAL;
	}

	if (len > PAGE_SIZE_2M)
		fixme("large page 2 MiB (requested %d)",
			  TO_MiB(len));
	else if (len > PAGE_SIZE_1G)
		fixme("huge page 1 GiB (requested %d)",
			  TO_GiB(len));

	if (off % PAGE_SIZE)
		return (void *)-EINVAL;

	if (uintptr_t(addr) % PAGE_SIZE && m_Fixed)
		return (void *)-EINVAL;

	if ((m_Shared && m_Private) ||
		(!m_Shared && !m_Private))
		return (void *)-EINVAL;

	PCB *pcb = thisProcess;
	VirtualMemoryArea *vma = pcb->vma;
	intptr_t ret = (intptr_t)vma->CreateCoWRegion(addr, len,
												  p_Read, p_Write, p_Exec,
												  m_Fixed, m_Shared);

	return (void *)ret;
}
