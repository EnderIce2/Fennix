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

/* https://pubs.opengroup.org/onlinepubs/009604499/functions/mprotect.html */
int sys_mprotect(SysFrm *,
				 void *addr, size_t len, int prot)
{
	if (len == 0)
		return -EINVAL;

	if (uintptr_t(addr) % PAGE_SIZE)
		return -EINVAL;

	bool p_None = prot & sc_PROT_NONE;
	bool p_Read = prot & sc_PROT_READ;
	bool p_Write = prot & sc_PROT_WRITE;
	// bool p_Exec = prot & sc_PROT_EXEC;

	PCB *pcb = thisProcess;
	Virtual vmm = Virtual(pcb->PageTable);

	for (uintptr_t i = uintptr_t(addr);
		 i < uintptr_t(addr) + len;
		 i += PAGE_SIZE)
	{
		if (likely(!vmm.Check((void *)i, G)))
		{
			PageTableEntry *pte = vmm.GetPTE(addr);
			if (!pte->Present ||
				(!pte->UserSupervisor && p_Read) ||
				(!pte->ReadWrite && p_Write))
			{
				debug("Page %p is not mapped with the correct permissions",
					  (void *)i);
				return -EACCES;
			}

			pte->Present = p_None;
			pte->UserSupervisor = p_Read;
			pte->ReadWrite = p_Write;
			// pte->ExecuteDisable = p_Exec;

#if defined(a64)
			CPU::x64::invlpg(addr);
#elif defined(a32)
			CPU::x32::invlpg(addr);
#elif defined(aa64)
			asmv("dsb sy");
			asmv("tlbi vae1is, %0"
				 :
				 : "r"(addr)
				 : "memory");
			asmv("dsb sy");
			asmv("isb");
#endif
		}
		else
		{
			warn("%p is a global page", (void *)i);
			return -ENOMEM;
		}
	}

	return 0;
}
