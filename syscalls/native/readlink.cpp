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

/* https://pubs.opengroup.org/onlinepubs/9699919799/functions/readlink.html */
ssize_t sys_readlink(SysFrm *, const char *path, char *buf,
					 size_t bufsize)
{
	if (!path || !buf)
		return -EINVAL;

	if (bufsize > PAGE_SIZE)
	{
		warn("bufsize is too large: %ld", bufsize);
		return -EINVAL;
	}

	PCB *pcb = thisProcess;
	Memory::Virtual vmm(pcb->PageTable);
	if (!vmm.Check((void *)buf, Memory::US))
	{
		warn("Invalid address %#lx", buf);
		return -EFAULT;
	}

	const char *pPath = pcb->PageTable->Get(path);
	char *pBuf = pcb->PageTable->Get(buf);
	function("%s %#lx %ld", pPath, buf, bufsize);
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	int fd = fdt->_open(pPath, O_RDONLY, 0);
	if (fd < 0)
		return -ENOENT;

	vfs::FileDescriptorTable::Fildes fildes = fdt->GetDescriptor(fd);
	vfs::Node *node = fildes.Handle->node;
	fdt->_close(fd);

	if (node->Type != vfs::NodeType::SYMLINK)
		return -EINVAL;

	if (!node->Symlink)
	{
		warn("Symlink null for \"%s\"?", pPath);
		return -EINVAL;
	}

	size_t len = strlen(node->Symlink);
	if (len > bufsize)
		len = bufsize;

	strncpy(pBuf, node->Symlink, len);
	return len;
}
