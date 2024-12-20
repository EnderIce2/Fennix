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

#include <interface/syscalls.h>

#include <syscalls.hpp>
#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../kernel.h"

using Tasking::PCB;
using Tasking::TCB;

int sys_brk(SysFrm *, void *end_data)
{
	return -ENOSYS;
}

void *sys_mmap(SysFrm *, void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (length == 0)
		return (void *)-EINVAL;

	bool p_None = prot & __SYS_PROT_NONE;
	bool p_Read = prot & __SYS_PROT_READ;
	bool p_Write = prot & __SYS_PROT_WRITE;
	bool p_Exec = prot & __SYS_PROT_EXEC;

	bool m_Shared = flags & __SYS_MAP_SHARED;
	bool m_Private = flags & __SYS_MAP_PRIVATE;
	bool m_Fixed = flags & __SYS_MAP_FIXED;
	bool m_Anon = flags & __SYS_MAP_ANONYMOUS;

	UNUSED(p_None);
	UNUSED(m_Anon);

	debug("None:%d Read:%d Write:%d Exec:%d",
		  p_None, p_Read, p_Write, p_Exec);

	debug("Shared:%d Private:%d Fixed:%d Anon:%d",
		  m_Shared, m_Private, m_Fixed, m_Anon);

	int unknownFlags = flags & ~(__SYS_MAP_SHARED | __SYS_MAP_PRIVATE |
								 __SYS_MAP_FIXED | __SYS_MAP_ANONYMOUS);
	if (unknownFlags)
	{
		/* We still have some flags missing afaik... */
		fixme("Unknown flags: %x", unknownFlags);
		/* FIXME: Continue? */
	}

	if (offset % PAGE_SIZE)
		return (void *)-EINVAL;

	if (uintptr_t(addr) % PAGE_SIZE && m_Fixed)
		return (void *)-EINVAL;

	if ((m_Shared && m_Private) ||
		(!m_Shared && !m_Private))
		return (void *)-EINVAL;

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;
	if (fd != -1 && !m_Anon)
	{
		fixme("File mapping not fully implemented");
		vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;

		auto _fd = fdt->FileMap.find(fd);
		if (_fd == fdt->FileMap.end())
		{
			debug("Invalid file descriptor %d", fd);
			return (void *)-EBADF;
		}

		if (p_Read)
		{
			void *pBuf = vma->RequestPages(TO_PAGES(length));
			debug("created buffer at %#lx-%#lx",
				  pBuf, (uintptr_t)pBuf + length);

			uintptr_t mFlags = Memory::US;
			if (p_Write)
				mFlags |= Memory::RW;

			if (m_Fixed)
			{
				if (m_Shared)
					return (void *)-ENOSYS;

				int mRet = vma->Map(addr, pBuf, length, mFlags);
				if (mRet < 0)
				{
					debug("Failed to map file: %s", strerror(mRet));
					return (void *)(uintptr_t)mRet;
				}
				off_t oldOff = fdt->usr_lseek(fd, 0, SEEK_CUR);
				fdt->usr_lseek(fd, offset, SEEK_SET);

				ssize_t ret = fdt->usr_read(fd, pBuf, length);
				fdt->usr_lseek(fd, oldOff, SEEK_SET);

				if (ret < 0)
				{
					debug("Failed to read file");
					return (void *)ret;
				}
				return addr;
			}
			else
			{
				int mRet = vma->Map(pBuf, pBuf, length, mFlags);
				if (mRet < 0)
				{
					debug("Failed to map file: %s", strerror(mRet));
					return (void *)(uintptr_t)mRet;
				}
			}

			off_t oldOff = fdt->usr_lseek(fd, 0, SEEK_CUR);
			fdt->usr_lseek(fd, offset, SEEK_SET);

			ssize_t ret = fdt->usr_read(fd, pBuf, length);

			fdt->usr_lseek(fd, oldOff, SEEK_SET);

			if (ret < 0)
			{
				debug("Failed to read file");
				return (void *)ret;
			}
			return pBuf;
		}

		debug("???");
		return (void *)-ENOSYS;
	}

	if (length < PAGE_SIZE * 100)
	{
		debug("length < 100 pages");

		if (addr == nullptr)
		{
			addr = vma->RequestPages(TO_PAGES(length), true);
			debug("Allocated %#lx-%#lx for pt %#lx",
				  addr, (uintptr_t)addr + length, vma->Table);
			return addr;
		}

		void *pAddr = vma->RequestPages(TO_PAGES(length));
		if (pAddr == nullptr)
		{
			debug("Failed to request pages");
			return (void *)-ENOMEM;
		}

		uintptr_t mapFlags = 0;
		if (p_Read)
			mapFlags |= Memory::PTFlag::US;
		if (p_Write)
			mapFlags |= Memory::PTFlag::RW;
		// if (p_Exec)
		// 	mapFlags |= Memory::PTFlag::XD;

		vma->Map(addr, pAddr, length, mapFlags);
		debug("mapped region %#lx-%#lx to %#lx-%#lx",
			  pAddr, (uintptr_t)pAddr + length, addr, (uintptr_t)addr + length);
		return addr;
	}

	debug("Creating CoWRegion");

	void *ret = vma->CreateCoWRegion(addr, length,
									 p_Read, p_Write, p_Exec,
									 m_Fixed, m_Shared);
	debug("ret: %#lx", ret);
	return (void *)ret;
}

int sys_munmap(SysFrm *Frame, void *addr, size_t length)
{
	return 0;
}

int sys_mprotect(SysFrm *Frame, void *addr, size_t length, int prot)
{
	return 0;
}

int sys_madvise(SysFrm *Frame, void *addr, size_t length, int advice)
{
	return 0;
}
