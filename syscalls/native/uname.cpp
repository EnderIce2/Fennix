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
#include <utsname.h>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../../syscalls.h"
#include "../../kernel.h"

using Tasking::PCB;
using namespace Memory;

/* https://pubs.opengroup.org/onlinepubs/9699919799/functions/uname.html */
int sys_uname(SysFrm *, struct utsname *buf)
{
	assert(sizeof(struct utsname) < PAGE_SIZE);

	Tasking::PCB *pcb = thisProcess;
	Memory::Virtual vmm(pcb->PageTable);

	if (!vmm.Check(buf, Memory::US))
	{
		warn("Invalid address %#lx", buf);
		return -EFAULT;
	}

	auto pBuf = pcb->PageTable->Get(buf);

	struct utsname uname =
	{
		/* TODO: This shouldn't be hardcoded */
		.sysname = KERNEL_NAME,
		.nodename = "fennix",
		.release = KERNEL_VERSION,
		.version = KERNEL_VERSION,
#if defined(a64)
		.machine = "x86_64",
#elif defined(a32)
		.machine = "i386",
#elif defined(aa64)
		.machine = "arm64",
#elif defined(aa32)
		.machine = "arm",
#endif
	};

	memcpy(pBuf, &uname, sizeof(struct utsname));
	return 0;
}
