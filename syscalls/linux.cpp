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

#include <syscall/linux/syscalls_amd64.hpp>
#include <syscall/linux/syscalls_i386.hpp>
#include <syscall/linux/signals.hpp>
#include <syscall/linux/defs.hpp>
#include <syscalls.hpp>

#include <signal.hpp>
#include <utsname.h>
#include <rand.hpp>
#include <limits.h>
#include <exec.hpp>
#include <debug.h>
#include <cpu.hpp>
#include <time.h>

#include <memory.hpp>
#define INI_IMPLEMENTATION
#include <ini.h>

#include "../kernel.h"

using Tasking::PCB;
using Tasking::TCB;

struct SyscallData
{
	const char *Name;
	void *Handler;
};

void linux_fork_return(void *tableAddr)
{
#if defined(a64)
	asmv("movq %0, %%cr3" ::"r"(tableAddr)); /* Load process page table */
	asmv("movq $0, %rax\n");				 /* Return 0 */
	asmv("movq %r8, %rsp\n");				 /* Restore stack pointer */
	asmv("movq %r8, %rbp\n");				 /* Restore base pointer */
	asmv("swapgs\n");						 /* Swap GS back to the user GS */
	asmv("sti\n");							 /* Enable interrupts */
	asmv("sysretq\n");						 /* Return to rcx address in user mode */
#elif defined(a32)
#warning "linux_fork_return not implemented for i386"
#endif
	__builtin_unreachable();
}

/* https://man7.org/linux/man-pages/man2/read.2.html */
static ssize_t linux_read(SysFrm *, int fd, void *buf, size_t count)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	void *pBuf = vma->UserCheckAndGetAddress(buf, count);
	if (pBuf == nullptr)
		return -EFAULT;

	function("%d, %p, %d", fd, buf, count);

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t ret = fdt->_read(fd, pBuf, count);
	if (ret >= 0)
		fdt->_lseek(fd, ret, SEEK_CUR);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/write.2.html */
static ssize_t linux_write(SysFrm *, int fd, const void *buf, size_t count)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const void *pBuf = vma->UserCheckAndGetAddress(buf, count);
	if (pBuf == nullptr)
		return -EFAULT;

	function("%d, %p, %d", fd, buf, count);

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t ret = fdt->_write(fd, pBuf, count);
	if (ret)
		fdt->_lseek(fd, ret, SEEK_CUR);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/open.2.html */
static int linux_open(SysFrm *sf, const char *pathname, int flags, mode_t mode)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const char *pPathname = vma->UserCheckAndGetAddress(pathname, PAGE_SIZE);
	if (pPathname == nullptr)
		return -EFAULT;

	function("%s, %d, %d", pPathname, flags, mode);

	if (flags & 0200000 /* O_DIRECTORY */)
	{
		vfs::Node *node = fs->GetNodeFromPath(pPathname);
		if (!node)
		{
			debug("Couldn't find %s", pPathname);
			return -ENOENT;
		}

		if (node->Type != vfs::NodeType::DIRECTORY)
		{
			debug("%s is not a directory", pPathname);
			return -ENOTDIR;
		}
	}

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_open(pPathname, flags, mode);
}

/* https://man7.org/linux/man-pages/man2/close.2.html */
static int linux_close(SysFrm *, int fd)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_close(fd);
}

/* https://man7.org/linux/man-pages/man2/stat.2.html */
static int linux_stat(SysFrm *, const char *pathname, struct stat *statbuf)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pPathname = vma->UserCheckAndGetAddress(pathname, PAGE_SIZE);
	if (pPathname == nullptr)
		return -EFAULT;

	return fdt->_stat(pPathname, statbuf);
}

/* https://man7.org/linux/man-pages/man2/fstat.2.html */
static int linux_fstat(SysFrm *, int fd, struct stat *statbuf)
{
#undef fstat
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pStatbuf = vma->UserCheckAndGetAddress(statbuf);
	if (pStatbuf == nullptr)
		return -EFAULT;

	return fdt->_fstat(fd, pStatbuf);
}

/* https://man7.org/linux/man-pages/man2/lstat.2.html */
static int linux_lstat(SysFrm *, const char *pathname, struct stat *statbuf)
{
#undef lstat
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pPathname = vma->UserCheckAndGetAddress(pathname, PAGE_SIZE);
	auto pStatbuf = vma->UserCheckAndGetAddress(statbuf);
	if (pPathname == nullptr || pStatbuf == nullptr)
		return -EFAULT;

	return fdt->_lstat(pPathname, pStatbuf);
}

#include "../syscalls.h"
/* https://man7.org/linux/man-pages/man2/lseek.2.html */
static off_t linux_lseek(SysFrm *, int fd, off_t offset, int whence)
{
	static_assert(SEEK_SET == sc_SEEK_SET);
	static_assert(SEEK_CUR == sc_SEEK_CUR);
	static_assert(SEEK_END == sc_SEEK_END);

	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_lseek(fd, offset, whence);
}

/* https://man7.org/linux/man-pages/man2/mmap.2.html */
static void *linux_mmap(SysFrm *, void *addr, size_t length, int prot,
						int flags, int fildes, off_t offset)
{
	static_assert(PROT_NONE == sc_PROT_NONE);
	static_assert(PROT_READ == sc_PROT_READ);
	static_assert(PROT_WRITE == sc_PROT_WRITE);
	static_assert(PROT_EXEC == sc_PROT_EXEC);

	int new_flags = 0;
	if (flags & MAP_SHARED)
	{
		new_flags |= sc_MAP_SHARED;
		flags &= ~MAP_SHARED;
	}
	if (flags & MAP_PRIVATE)
	{
		new_flags |= sc_MAP_PRIVATE;
		flags &= ~MAP_PRIVATE;
	}
	if (flags & MAP_FIXED)
	{
		new_flags |= sc_MAP_FIXED;
		flags &= ~MAP_FIXED;
	}
	if (flags & MAP_ANONYMOUS)
	{
		new_flags |= sc_MAP_ANONYMOUS;
		flags &= ~MAP_ANONYMOUS;
	}
	if (flags)
		fixme("unhandled flags: %#x", flags);
	flags = new_flags;

	if (length == 0)
		return (void *)-EINVAL;

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

	debug("None:%d Read:%d Write:%d Exec:%d",
		  p_None, p_Read, p_Write, p_Exec);

	debug("Shared:%d Private:%d Fixed:%d Anon:%d",
		  m_Shared, m_Private, m_Fixed, m_Anon);

	int UnknownFlags = flags & ~(sc_MAP_SHARED |
								 sc_MAP_PRIVATE |
								 sc_MAP_FIXED |
								 sc_MAP_ANONYMOUS);
	if (UnknownFlags)
	{
		/* We still have some flags missing afaik... */
		fixme("Unknown flags: %x", UnknownFlags);
		/* Allow? */
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
	if (fildes != -1 && !m_Anon)
	{
		fixme("File mapping not fully implemented");
		vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
		vfs::FileDescriptorTable::Fildes &_fd = fdt->GetDescriptor(fildes);
		if (_fd.Descriptor != fildes)
		{
			debug("Invalid file descriptor %d", fildes);
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
				off_t oldOff = fdt->_lseek(fildes, 0, SEEK_CUR);
				fdt->_lseek(fildes, offset, SEEK_SET);

				ssize_t ret = fdt->_read(fildes, pBuf, length);
				fdt->_lseek(fildes, oldOff, SEEK_SET);

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

			off_t oldOff = fdt->_lseek(fildes, 0, SEEK_CUR);
			fdt->_lseek(fildes, offset, SEEK_SET);

			ssize_t ret = fdt->_read(fildes, pBuf, length);

			fdt->_lseek(fildes, oldOff, SEEK_SET);

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

	void *ret = vma->CreateCoWRegion(addr, length,
									 p_Read, p_Write, p_Exec,
									 m_Fixed, m_Shared);
	return (void *)ret;
}
#undef __FENNIX_KERNEL_SYSCALLS_LIST_H__

/* https://man7.org/linux/man-pages/man2/mprotect.2.html */
static int linux_mprotect(SysFrm *, void *addr, size_t len, int prot)
{
	if (len == 0)
		return -EINVAL;

	if (uintptr_t(addr) % PAGE_SIZE)
		return -EINVAL;

	// bool p_None = prot & sc_PROT_NONE;
	bool p_Read = prot & sc_PROT_READ;
	bool p_Write = prot & sc_PROT_WRITE;
	// bool p_Exec = prot & sc_PROT_EXEC;

	PCB *pcb = thisProcess;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	for (uintptr_t i = uintptr_t(addr);
		 i < uintptr_t(addr) + len;
		 i += PAGE_SIZE)
	{
		if (unlikely(vmm.Check((void *)i, Memory::G)))
		{
			warn("%p is a global page", (void *)i);
			return -ENOMEM;
		}

		Memory::PageTableEntry *pte = vmm.GetPTE(addr);
		if (pte == nullptr)
		{
			debug("Page %#lx is not mapped inside %#lx",
				  (void *)i, pcb->PageTable);
			fixme("Page %#lx is not mapped", (void *)i);
			continue;
			return -ENOMEM;
		}

		if (!pte->Present ||
			(!pte->UserSupervisor && p_Read) ||
			(!pte->ReadWrite && p_Write))
		{
			debug("Page %p is not mapped with the correct permissions",
				  (void *)i);
			return -EACCES;
		}

		// pte->Present = !p_None;
		pte->UserSupervisor = p_Read;
		pte->ReadWrite = p_Write;
		// pte->ExecuteDisable = p_Exec;

		debug("Changed permissions of page %#lx to %s %s %s %s",
			  (void *)i,
			  (prot & sc_PROT_NONE) ? "None" : "",
			  p_Read ? "Read" : "",
			  p_Write ? "Write" : "",
			  (prot & sc_PROT_EXEC) ? "Exec" : "");

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

	return 0;
}

/* https://man7.org/linux/man-pages/man2/munmap.2.html */
static int linux_munmap(SysFrm *, void *addr, size_t length)
{
	if (uintptr_t(addr) % PAGE_SIZE)
		return -EINVAL;

	if (length == 0)
		return -EINVAL;

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;
	vma->FreePages((void *)addr, TO_PAGES(length));
	return 0;
}

/* https://man7.org/linux/man-pages/man2/brk.2.html */
static void *linux_brk(SysFrm *, void *addr)
{
	PCB *pcb = thisProcess;
	void *ret = pcb->ProgramBreak->brk(addr);
	debug("brk(%#lx) = %#lx", addr, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/ioctl.2.html */
static int linux_ioctl(SysFrm *, int fd, unsigned long request, void *argp)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pArgp = vma->UserCheckAndGetAddress(argp);
	if (pArgp == nullptr)
		return -EFAULT;

	return fdt->_ioctl(fd, request, pArgp);
}

/* https://man7.org/linux/man-pages/man2/pread.2.html */
static ssize_t linux_pread64(SysFrm *, int fd, void *buf, size_t count, off_t offset)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	void *pBuf = vma->UserCheckAndGetAddress(buf, count);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	off_t oldOff = fdt->_lseek(fd, 0, SEEK_CUR);
	fdt->_lseek(fd, offset, SEEK_SET);
	ssize_t ret = fdt->_read(fd, pBuf, count);
	fdt->_lseek(fd, oldOff, SEEK_SET);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/pread.2.html */
static ssize_t linux_pwrite64(SysFrm *, int fd, const void *buf, size_t count, off_t offset)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const void *pBuf = vma->UserCheckAndGetAddress(buf, count);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	off_t oldOff = fdt->_lseek(fd, 0, SEEK_CUR);
	fdt->_lseek(fd, offset, SEEK_SET);
	ssize_t ret = fdt->_write(fd, pBuf, count);
	fdt->_lseek(fd, oldOff, SEEK_SET);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/readv.2.html */
static ssize_t linux_readv(SysFrm *sf, int fildes, const struct iovec *iov, int iovcnt)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const struct iovec *pIov = vma->UserCheckAndGetAddress(iov, sizeof(struct iovec) * iovcnt);
	if (pIov == nullptr)
		return -EFAULT;

	ssize_t Total = 0;
	for (int i = 0; i < iovcnt; i++)
	{
		debug("%d: iov[%d]: %p %d", fildes, i, pIov[i].iov_base, pIov[i].iov_len);

		if (!pIov[i].iov_base)
		{
			debug("invalid iov_base");
			return -EFAULT;
		}

		if (pIov[i].iov_len == 0)
		{
			debug("invalid iov_len");
			continue;
		}

		ssize_t n = linux_read(sf, fildes, pIov[i].iov_base, pIov[i].iov_len);
		if (n < 0)
			return n;
		debug("n: %d", n);

		Total += n;
		if (n < (ssize_t)pIov[i].iov_len)
		{
			debug("break");
			break;
		}
	}
	debug("readv: %d", Total);
	return Total;
}

/* https://man7.org/linux/man-pages/man2/writev.2.html */
static ssize_t linux_writev(SysFrm *sf, int fildes, const struct iovec *iov, int iovcnt)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const struct iovec *pIov = vma->UserCheckAndGetAddress(iov, sizeof(struct iovec) * iovcnt);
	if (pIov == nullptr)
		return -EFAULT;

	ssize_t Total = 0;
	for (int i = 0; i < iovcnt; i++)
	{
		debug("%d: iov[%d]: %p %d", fildes, i, pIov[i].iov_base, pIov[i].iov_len);

		if (!pIov[i].iov_base)
		{
			debug("invalid iov_base");
			return -EFAULT;
		}

		if (pIov[i].iov_len == 0)
		{
			debug("invalid iov_len");
			continue;
		}

		ssize_t n = linux_write(sf, fildes, pIov[i].iov_base, pIov[i].iov_len);
		if (n < 0)
			return n;
		debug("n: %d", n);

		Total += n;
		if (n < (ssize_t)pIov[i].iov_len)
		{
			debug("break");
			break;
		}
	}
	debug("writev: %d", Total);
	return Total;
}

/* https://man7.org/linux/man-pages/man2/access.2.html */
static int linux_access(SysFrm *, const char *pathname, int mode)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pPathname = vma->UserCheckAndGetAddress(pathname);
	if (pPathname == nullptr)
		return -EFAULT;

	debug("access(%s, %d)", (char *)pPathname, mode);

	if (!fs->PathExists(pPathname, pcb->CurrentWorkingDirectory))
		return -ENOENT;

	stub;
	return 0;
}

/* https://man7.org/linux/man-pages/man2/pipe.2.html */
static int linux_pipe(SysFrm *, int pipefd[2])
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	int *pPipefd = vma->UserCheckAndGetAddress(pipefd);
	debug("pipefd=%#lx", pPipefd);
	fixme("pipefd=[%d, %d]", pPipefd[0], pPipefd[1]);
	return -ENOSYS;
}

/* https://man7.org/linux/man-pages/man2/dup.2.html */
static int linux_dup(SysFrm *, int oldfd)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_dup(oldfd);
}

/* https://man7.org/linux/man-pages/man2/dup.2.html */
static int linux_dup2(SysFrm *, int oldfd, int newfd)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_dup2(oldfd, newfd);
}

/* https://man7.org/linux/man-pages/man2/pause.2.html */
static int linux_pause(SysFrm *)
{
	PCB *pcb = thisProcess;
	return pcb->Signals->WaitAnySignal();
}

/* https://man7.org/linux/man-pages/man2/nanosleep.2.html */
static int linux_nanosleep(SysFrm *,
						   const struct timespec *req,
						   struct timespec *rem)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pReq = vma->UserCheckAndGetAddress(req);
	auto pRem = vma->UserCheckAndGetAddress(rem);
	if (pReq == nullptr || pRem == nullptr)
		return -EFAULT;

	if (pReq->tv_nsec < 0 || pReq->tv_nsec > 999999999)
	{
		debug("Invalid tv_nsec %ld", pReq->tv_nsec);
		return -EINVAL;
	}

	if (pReq->tv_sec < 0)
	{
		debug("Invalid tv_sec %ld", pReq->tv_sec);
		return -EINVAL;
	}

	debug("tv_nsec=%ld tv_sec=%ld",
		  pReq->tv_nsec, pReq->tv_sec);

	uint64_t nanoTime = pReq->tv_nsec;
	uint64_t secTime = pReq->tv_sec * 1000000000; /* Nano */

	uint64_t time = TimeManager->GetCounter();
	uint64_t sleepTime = TimeManager->CalculateTarget(nanoTime + secTime, Time::Nanoseconds);

	debug("time=%ld secTime=%ld nanoTime=%ld sleepTime=%ld",
		  time, secTime, nanoTime, sleepTime);

	while (time < sleepTime)
	{
		if (pcb->Signals->HasPendingSignal())
		{
			debug("sleep interrupted by signal");
			return -EINTR;
		}

		pcb->GetContext()->Yield();
		time = TimeManager->GetCounter();
	}
	debug("time=     %ld", time);
	debug("sleepTime=%ld", sleepTime);

	if (rem)
	{
		pRem->tv_sec = 0;
		pRem->tv_nsec = 0;
	}
	return 0;
}

/* https://man7.org/linux/man-pages/man2/getpid.2.html */
static pid_t linux_getpid(SysFrm *)
{
	return thisProcess->ID;
}

/* https://man7.org/linux/man-pages/man2/shutdown.2.html */
static int linux_shutdown(SysFrm *, int sockfd, int how)
{
	stub;
	return -ENOSYS;
}

/* https://man7.org/linux/man-pages/man2/fork.2.html */
static pid_t linux_fork(SysFrm *sf)
{
	TCB *Thread = thisThread;
	PCB *Parent = Thread->Parent;

	PCB *NewProcess =
		TaskManager->CreateProcess(Parent, Parent->Name,
								   Parent->Security.ExecutionMode,
								   true);
	if (unlikely(!NewProcess))
	{
		error("Failed to create process for fork");
		return -EAGAIN;
	}

	NewProcess->PageTable = Parent->PageTable->Fork();
	NewProcess->vma->Table = NewProcess->PageTable;
	NewProcess->vma->Fork(Parent->vma);
	NewProcess->ProgramBreak->SetTable(NewProcess->PageTable);
	NewProcess->FileDescriptors->Fork(Parent->FileDescriptors);
	NewProcess->Executable = Parent->Executable;
	NewProcess->CurrentWorkingDirectory = Parent->CurrentWorkingDirectory;

	TCB *NewThread =
		TaskManager->CreateThread(NewProcess,
								  0,
								  nullptr,
								  nullptr,
								  std::vector<AuxiliaryVector>(),
								  Thread->Info.Architecture,
								  Thread->Info.Compatibility,
								  true);
	if (!NewThread)
	{
		error("Failed to create thread for fork");
		delete NewProcess;
		return -EAGAIN;
	}
	NewThread->Rename(Thread->Name);

	TaskManager->UpdateFrame();

	NewThread->FPU = Thread->FPU;
	NewThread->Stack->Fork(Thread->Stack);
	NewThread->Info.Architecture = Thread->Info.Architecture;
	NewThread->Info.Compatibility = Thread->Info.Compatibility;
	NewThread->Security.IsCritical = Thread->Security.IsCritical;
	NewThread->Registers = Thread->Registers;
#if defined(a64)
	NewThread->Registers.rip = (uintptr_t)linux_fork_return;
	/* For sysretq */
	NewThread->Registers.rdi = (uintptr_t)NewProcess->PageTable;
	NewThread->Registers.rcx = sf->ReturnAddress;
	NewThread->Registers.r8 = sf->StackPointer;
#else
#warning "sys_fork not implemented for other platforms"
#endif

#ifdef a86
	NewThread->GSBase = NewThread->ShadowGSBase;
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("ret addr: %#lx, stack: %#lx ip: %#lx", sf->ReturnAddress,
		  sf->StackPointer, (uintptr_t)linux_fork_return);
	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)",
		  Thread->Name, Thread->ID,
		  NewThread->Name, NewThread->ID);
	NewThread->SetState(Tasking::Ready);

	// Parent->GetContext()->Yield();
	return (int)NewProcess->ID;
}

/* https://man7.org/linux/man-pages/man2/execve.2.html */
__no_sanitize("undefined") static int linux_execve(SysFrm *sf, const char *pathname,
												   char *const argv[],
												   char *const envp[])
{
	/* FIXME: exec doesn't follow the UNIX standard
		The pid, open files, etc. should be preserved */
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pPathname = vma->UserCheckAndGetAddress(pathname, PAGE_SIZE);
	auto pArgv = vma->UserCheckAndGetAddress(argv, 1 /*MAX_ARG*/); /* MAX_ARG is too much? */
	auto pEnvp = vma->UserCheckAndGetAddress(envp, 1 /*MAX_ARG*/);
	if (pPathname == nullptr || pArgv == nullptr || pEnvp == nullptr)
		return -EFAULT;

	function("%s %#lx %#lx", pPathname, pArgv, pEnvp);

	int argvLen = 0;
	for (argvLen = 0; MAX_ARG; argvLen++)
	{
		if (vma->UserCheck(pArgv[argvLen]) < 0)
			break;

		auto arg = pcb->PageTable->Get(pArgv[argvLen]);
		if (arg == nullptr)
			break;
	}

	int envpLen = 0;
	for (envpLen = 0; MAX_ARG; envpLen++)
	{
		if (vma->UserCheck(pEnvp[envpLen]) < 0)
			break;

		auto arg = pcb->PageTable->Get(pEnvp[envpLen]);
		if (arg == nullptr)
			break;
	}

	char **safe_argv = (char **)pcb->vma->RequestPages(TO_PAGES(argvLen * sizeof(char *)));
	char **safe_envp = (char **)pcb->vma->RequestPages(TO_PAGES(envpLen * sizeof(char *)));

	const char *arg;
	char *n_arg;
	for (int i = 0; i < argvLen; i++)
	{
		arg = pcb->PageTable->Get(pArgv[i]);
		assert(arg != nullptr);
		size_t len = strlen(arg);
		debug("arg[%d]: %s", i, arg);

		n_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
		memcpy((void *)n_arg, arg, len);
		n_arg[len] = '\0';

		safe_argv[i] = n_arg;

		if (likely(i < MAX_ARG - 1))
			safe_argv[i + 1] = nullptr;
	}

	for (int i = 0; i < envpLen; i++)
	{
		arg = pcb->PageTable->Get(pEnvp[i]);
		assert(arg != nullptr);
		size_t len = strlen(arg);
		debug("env[%d]: %s", i, arg);

		n_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
		memcpy((void *)n_arg, arg, len);
		n_arg[len] = '\0';

		safe_envp[i] = n_arg;

		if (likely(i < MAX_ARG - 1))
			safe_envp[i + 1] = nullptr;
	}

	vfs::RefNode *File = fs->Open(pPathname,
								  pcb->CurrentWorkingDirectory);

	if (!File)
	{
		error("File not found");
		return -ENOENT;
	}

	char shebang_magic[2];
	File->read((uint8_t *)shebang_magic, 2);

	if (shebang_magic[0] == '#' && shebang_magic[1] == '!')
	{
		char *orig_path = (char *)pcb->vma->RequestPages(TO_PAGES(strlen(pPathname) + 1));
		memcpy(orig_path, pPathname, strlen(pPathname) + 1);

		char *shebang = (char *)pPathname;
		size_t shebang_len = 0;
		constexpr int shebang_len_max = 255;
		File->seek(2, SEEK_SET);
		off_t shebang_off = 2;
		while (true)
		{
			char c;
			if (File->node->read((uint8_t *)&c, 1, shebang_off) == 0)
				break;
			if (c == '\n' || shebang_len == shebang_len_max)
				break;
			shebang[shebang_len++] = c;
			shebang_off++;
		}
		shebang[shebang_len] = '\0';
		debug("Shebang: %s", shebang);

		char **c_safe_argv = (char **)pcb->vma->RequestPages(TO_PAGES(MAX_ARG));
		int i = 0;
		for (; safe_argv[i] != nullptr; i++)
		{
			size_t arg_len = strlen(safe_argv[i]);
			char *c_arg = (char *)pcb->vma->RequestPages(TO_PAGES(arg_len));
			memcpy((void *)c_arg, safe_argv[i], arg_len);
			c_arg[arg_len] = '\0';

			c_safe_argv[i] = c_arg;
			debug("c_safe_argv[%d]: %s", i, c_safe_argv[i]);
		}
		c_safe_argv[i] = nullptr;

		char *token = strtok(shebang, " ");
		i = 0;
		while (token != nullptr)
		{
			size_t len = strlen(token);
			char *t_arg = (char *)pcb->vma->RequestPages(TO_PAGES(len));
			memcpy((void *)t_arg, token, len);
			t_arg[len] = '\0';

			safe_argv[i++] = t_arg;
			token = strtok(nullptr, " ");
		}

		safe_argv[i++] = orig_path;
		for (int j = 1; c_safe_argv[j] != nullptr; j++)
		{
			safe_argv[i++] = c_safe_argv[j];
			debug("clone: safe_argv[%d]: %s",
				  i, safe_argv[i - 1]);
		}
		safe_argv[i] = nullptr;

		delete File;
		return linux_execve(sf, safe_argv[0],
							(char *const *)safe_argv,
							(char *const *)safe_envp);
	}

	int ret = Execute::Spawn((char *)pPathname,
							 (const char **)safe_argv,
							 (const char **)safe_envp,
							 pcb, true,
							 pcb->Info.Compatibility);

	if (ret < 0)
	{
		error("Failed to spawn");
		delete File;
		return ret;
	}

	const char *baseName;
	cwk_path_get_basename(pPathname, &baseName, nullptr);

	pcb->Rename(baseName);
	pcb->SetWorkingDirectory(File->node->Parent);
	pcb->SetExe(pPathname);

	delete File;
	Tasking::Task *ctx = pcb->GetContext();
	// ctx->Sleep(1000);
	// pcb->SetState(Tasking::Zombie);
	// pcb->SetExitCode(0); /* FIXME: get process exit code */
	while (true)
		ctx->Yield();
	__builtin_unreachable();
}

/* https://man7.org/linux/man-pages/man2/exit.2.html */
static __noreturn void linux_exit(SysFrm *, int status)
{
	TCB *t = thisThread;

	trace("Userspace thread %s(%d) exited with code %d (%#x)",
		  t->Name,
		  t->ID, status,
		  status < 0 ? -status : status);

	t->SetState(Tasking::Zombie);
	t->SetExitCode(status);
	while (true)
		t->GetContext()->Yield();
	__builtin_unreachable();
}

/* https://man7.org/linux/man-pages/man2/wait4.2.html */
static pid_t linux_wait4(SysFrm *, pid_t pid, int *wstatus,
						 int options, struct rusage *rusage)
{
	static_assert(sizeof(struct rusage) < PAGE_SIZE);
	/* FIXME: this function is very poorly implemented, the way
		it handles the zombie & coredump processes is very
		inefficient and should be rewritten */

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (pid == -1)
	{
		if (pcb->Children.empty())
		{
			debug("No children");
			return -ECHILD;
		}

		std::vector<PCB *> wChilds;
		for (auto child : pcb->Children)
		{
			if (child->State == Tasking::Zombie)
			{
				if (wstatus != nullptr)
				{
					int *pWstatus = pcb->PageTable->Get(wstatus);
					*pWstatus = 0;

					bool ProcessExited = true;
					int ExitStatus = child->ExitCode.load();

					debug("Process returned %d", ExitStatus);

					if (ProcessExited)
						*pWstatus |= ExitStatus << 8;
				}

				if (rusage != nullptr)
				{
					size_t kTime = child->Info.KernelTime;
					size_t uTime = child->Info.UserTime;
					size_t _maxrss = child->GetSize();

					struct rusage *pRusage = vma->UserCheckAndGetAddress(rusage);
					if (pRusage == nullptr)
						return -EFAULT;

					pRusage->ru_utime.tv_sec = uTime / 1000000000000000; /* Seconds */
					pRusage->ru_utime.tv_usec = uTime / 1000000000;		 /* Microseconds */

					pRusage->ru_stime.tv_sec = kTime / 1000000000000000; /* Seconds */
					pRusage->ru_stime.tv_usec = kTime / 1000000000;		 /* Microseconds */

					pRusage->ru_maxrss = _maxrss;
				}

				child->SetState(Tasking::Terminated);
				return child->ID;
			}

			if (child->State == Tasking::CoreDump)
			{
				if (wstatus != nullptr)
				{
					int *pWstatus = vma->UserCheckAndGetAddress(wstatus);
					if (pWstatus == nullptr)
						return -EFAULT;
					*pWstatus = 0;

					bool ProcessExited = true;
					int ExitStatus = child->ExitCode.load();
					bool ProcessSignaled = true;
					bool CoreDumped = true;
					int TermSignal = child->Signals->GetLastSignal();

					debug("Process returned %d", ExitStatus);

					if (ProcessExited)
						*pWstatus |= ExitStatus << 8;

					if (ProcessSignaled)
						*pWstatus |= TermSignal;

					if (CoreDumped)
						*pWstatus |= 0x80;
				}

				if (rusage != nullptr)
				{
					size_t kTime = child->Info.KernelTime;
					size_t uTime = child->Info.UserTime;
					size_t _maxrss = child->GetSize();

					struct rusage *pRusage = vma->UserCheckAndGetAddress(rusage);
					if (pRusage == nullptr)
						return -EFAULT;

					pRusage->ru_utime.tv_sec = uTime / 1000000000000000; /* Seconds */
					pRusage->ru_utime.tv_usec = uTime / 1000000000;		 /* Microseconds */

					pRusage->ru_stime.tv_sec = kTime / 1000000000000000; /* Seconds */
					pRusage->ru_stime.tv_usec = kTime / 1000000000;		 /* Microseconds */

					pRusage->ru_maxrss = _maxrss;
				}

				child->SetState(Tasking::Terminated);
				return child->ID;
			}

			if (child->State != Tasking::Terminated)
				wChilds.push_back(child);
		}

		if (wChilds.empty())
		{
			debug("No children");
			return -ECHILD;
		}

		fixme("Waiting for %d children", wChilds.size());
		pid = wChilds.front()->ID;
	}

	if (pid == 0)
	{
		fixme("Waiting for any child process whose process group ID is equal to that of the calling process");
		return -ENOSYS;
	}

	if (pid < -1)
	{
		fixme("Waiting for any child process whose process group ID is equal to the absolute value of pid");
		return -ENOSYS;
	}

	/* Wait for a child process, or any process? */
	PCB *tPcb = pcb->GetContext()->GetProcessByID(pid);
	if (!tPcb)
	{
		warn("Invalid PID %d", pid);
		return -ECHILD;
	}

	if (options)
	{
#define WCONTINUED 1
#define WNOHANG 2
#define WUNTRACED 4
#define WEXITED 8
#define WNOWAIT 16
#define WSTOPPED 32
		fixme("options=%#x", options);
	}

#ifdef DEBUG
	foreach (auto child in pcb->Children)
		debug("Child: %s(%d)", child->Name, child->ID);
#endif

	debug("Waiting for %d(%#lx) state %d", pid, tPcb, tPcb->State);
	while (tPcb->State != Tasking::Zombie &&
		   tPcb->State != Tasking::CoreDump &&
		   tPcb->State != Tasking::Terminated)
		pcb->GetContext()->Yield();
	debug("Waited for %d(%#lx) state %d", pid, tPcb, tPcb->State);

	if (wstatus != nullptr)
	{
		int *pWstatus = vma->UserCheckAndGetAddress(wstatus);
		if (pWstatus == nullptr)
			return -EFAULT;
		*pWstatus = 0;

		bool ProcessExited = true;
		int ExitStatus = tPcb->ExitCode.load();
		bool ProcessSignaled = false;
		bool CoreDumped = false;
		bool ProcessStopped = false;
		bool ProcessContinued = false;
		int TermSignal = 0;

		debug("Process returned %d", ExitStatus);

		if (ProcessExited)
			*pWstatus |= ExitStatus << 8;

		if (ProcessSignaled)
			*pWstatus |= TermSignal;

		if (CoreDumped)
			*pWstatus |= 0x80;

		/* FIXME: Untested */

		if (ProcessStopped)
			*pWstatus |= 0x7F;

		if (ProcessContinued)
			*pWstatus = 0xFFFF;

		debug("wstatus=%#x", *pWstatus);
	}

	if (rusage != nullptr)
	{
		size_t kTime = tPcb->Info.KernelTime;
		size_t uTime = tPcb->Info.UserTime;
		size_t _maxrss = tPcb->GetSize();

		struct rusage *pRusage = vma->UserCheckAndGetAddress(rusage);
		if (pRusage == nullptr)
			return -EFAULT;

		pRusage->ru_utime.tv_sec = uTime / 1000000000000000; /* Seconds */
		pRusage->ru_utime.tv_usec = uTime / 1000000000;		 /* Microseconds */

		pRusage->ru_stime.tv_sec = kTime / 1000000000000000; /* Seconds */
		pRusage->ru_stime.tv_usec = kTime / 1000000000;		 /* Microseconds */

		pRusage->ru_maxrss = _maxrss;
		/* TODO: The rest of the fields */
	}

	return pid;
}

/* https://man7.org/linux/man-pages/man2/kill.2.html */
static int linux_kill(SysFrm *, pid_t pid, int sig)
{
	PCB *target = thisProcess->GetContext()->GetProcessByID(pid);
	if (!target)
		return -ESRCH;

	/* TODO: Check permissions */

	if (sig == 0)
		return 0;

	if (pid == 0)
	{
		fixme("Sending signal %d to all processes", sig);
		return -ENOSYS;
	}

	if (pid == -1)
	{
		fixme("Sending signal %d to all processes except init", sig);
		return -ENOSYS;
	}

	if (pid < -1)
	{
		fixme("Sending signal %d to process group %d", sig, pid);
		return -ENOSYS;
	}

	return target->Signals->SendSignal(sig);
}

/* https://man7.org/linux/man-pages/man2/uname.2.html */
static int linux_uname(SysFrm *, struct utsname *buf)
{
	assert(sizeof(struct utsname) < PAGE_SIZE);

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pBuf = vma->UserCheckAndGetAddress(buf);
	if (pBuf == nullptr)
		return -EFAULT;

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

	vfs::RefNode *rn = fs->Open("/etc/cross/linux");
	if (rn)
	{
		Memory::SmartHeap sh(rn->Size);
		rn->read(sh, rn->Size);
		delete rn;

		ini_t *ini = ini_load(sh, NULL);
		int section = ini_find_section(ini, "uname", NULL);
		int sysIdx = ini_find_property(ini, section, "sysname", NULL);
		int nodIdx = ini_find_property(ini, section, "nodename", NULL);
		int relIdx = ini_find_property(ini, section, "release", NULL);
		int verIdx = ini_find_property(ini, section, "version", NULL);
		int macIdx = ini_find_property(ini, section, "machine", NULL);
		const char *uSys = ini_property_value(ini, section, sysIdx);
		const char *uNod = ini_property_value(ini, section, nodIdx);
		const char *uRel = ini_property_value(ini, section, relIdx);
		const char *uVer = ini_property_value(ini, section, verIdx);
		const char *uMac = ini_property_value(ini, section, macIdx);
		debug("sysname=%s", uSys);
		debug("nodename=%s", uNod);
		debug("release=%s", uRel);
		debug("version=%s", uVer);
		debug("machine=%s", uMac);

		if (uSys && strcmp(uSys, "auto") != 0)
			strncpy(uname.sysname, uSys, sizeof(uname.sysname));
		if (uNod && strcmp(uNod, "auto") != 0)
			strncpy(uname.nodename, uNod, sizeof(uname.nodename));
		if (uRel && strcmp(uRel, "auto") != 0)
			strncpy(uname.release, uRel, sizeof(uname.release));
		if (uVer && strcmp(uVer, "auto") != 0)
			strncpy(uname.version, uVer, sizeof(uname.version));
		if (uMac && strcmp(uMac, "auto") != 0)
			strncpy(uname.machine, uMac, sizeof(uname.machine));
		ini_destroy(ini);
	}
	else
		warn("Couldn't open /etc/cross/linux");

	memcpy(pBuf, &uname, sizeof(struct utsname));
	return 0;
}

/* https://man7.org/linux/man-pages/man2/fcntl.2.html */
static int linux_fcntl(SysFrm *, int fd, int cmd, void *arg)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;

	switch (cmd)
	{
	case F_DUPFD:
		return fdt->_dup2(fd, s_cst(int, (uintptr_t)arg));
	case F_GETFD:
		return fdt->GetFlags(fd);
	case F_SETFD:
		return fdt->SetFlags(fd, s_cst(int, (uintptr_t)arg));
	case F_GETFL:
	case F_SETFL:
	case F_SETOWN:
	case F_GETOWN:
	case F_SETSIG:
	case F_GETSIG:
	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
	case F_SETOWN_EX:
	case F_GETOWN_EX:
	{
		fixme("cmd %d not implemented", cmd);
		return -ENOSYS;
	}
	default:
	{
		debug("Invalid cmd %#x", cmd);
		return -EINVAL;
	}
	}
}

/* https://man7.org/linux/man-pages/man2/creat.2.html */
static int linux_creat(SysFrm *, const char *pathname, mode_t mode)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->_creat(pathname, mode);
}

/* https://man7.org/linux/man-pages/man2/mkdir.2.html */
static int linux_mkdir(SysFrm *, const char *pathname, mode_t mode)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;
	fixme("semi-stub");

	const char *pPathname = vma->UserCheckAndGetAddress(pathname);
	if (!pPathname)
		return -EFAULT;

	vfs::Node *n = fs->Create(pPathname, vfs::DIRECTORY, pcb->CurrentWorkingDirectory);
	if (!n)
		return -EEXIST;
	return 0;
}

/* https://man7.org/linux/man-pages/man2/readlink.2.html */
static ssize_t linux_readlink(SysFrm *, const char *pathname,
							  char *buf, size_t bufsiz)
{
	if (!pathname || !buf)
		return -EINVAL;

	if (bufsiz > PAGE_SIZE)
	{
		warn("bufsiz is too large: %ld", bufsiz);
		return -EINVAL;
	}

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const char *pPath = vma->UserCheckAndGetAddress(pathname);
	char *pBuf = vma->UserCheckAndGetAddress(buf);
	if (pPath == nullptr || pBuf == nullptr)
		return -EFAULT;

	function("%s %#lx %ld", pPath, buf, bufsiz);
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
	if (len > bufsiz)
		len = bufsiz;

	strncpy(pBuf, node->Symlink, len);
	return len;
}

/* https://man7.org/linux/man-pages/man2/getuid.2.html */
static uid_t linux_getuid(SysFrm *)
{
	return thisProcess->Security.Real.UserID;
}

/* https://man7.org/linux/man-pages/man2/getgid.2.html */
static gid_t linux_getgid(SysFrm *)
{
	return thisProcess->Security.Real.GroupID;
}

/* https://man7.org/linux/man-pages/man2/getuid.2.html */
static uid_t linux_geteuid(SysFrm *)
{
	return thisProcess->Security.Effective.UserID;
}

/* https://man7.org/linux/man-pages/man2/getgid.2.html */
static gid_t linux_getegid(SysFrm *)
{
	return thisProcess->Security.Effective.GroupID;
}

/* https://man7.org/linux/man-pages/man2/getpid.2.html */
static pid_t linux_getppid(SysFrm *)
{
	return thisProcess->Parent->ID;
}

/* https://man7.org/linux/man-pages/man2/arch_prctl.2.html */
static int linux_arch_prctl(SysFrm *, int code, unsigned long addr)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (vma->UserCheck(addr) < 0)
		return -EFAULT;

	switch (code)
	{
	case ARCH_SET_GS:
	{
#if defined(a64)
		CPU::x64::wrmsr(CPU::x64::MSRID::MSR_GS_BASE, addr);
#elif defined(a32)
		CPU::x32::wrmsr(CPU::x32::MSRID::MSR_GS_BASE, addr);
#endif
		return 0;
	}
	case ARCH_SET_FS:
	{
#if defined(a64)
		CPU::x64::wrmsr(CPU::x64::MSRID::MSR_FS_BASE, addr);
#elif defined(a32)
		CPU::x32::wrmsr(CPU::x32::MSRID::MSR_FS_BASE, addr);
#endif
		return 0;
	}
	case ARCH_GET_FS:
	{
#if defined(a64)
		*r_cst(uint64_t *, addr) =
			CPU::x64::rdmsr(CPU::x64::MSRID::MSR_FS_BASE);
#elif defined(a32)
		*r_cst(uint64_t *, addr) =
			CPU::x32::rdmsr(CPU::x32::MSRID::MSR_FS_BASE);
#endif
		return 0;
	}
	case ARCH_GET_GS:
	{
#if defined(a64)
		*r_cst(uint64_t *, addr) =
			CPU::x64::rdmsr(CPU::x64::MSRID::MSR_GS_BASE);
#elif defined(a32)
		*r_cst(uint64_t *, addr) =
			CPU::x32::rdmsr(CPU::x32::MSRID::MSR_GS_BASE);
#endif
		return 0;
	}
	case ARCH_GET_CPUID:
	case ARCH_SET_CPUID:
	case ARCH_GET_XCOMP_SUPP:
	case ARCH_GET_XCOMP_PERM:
	case ARCH_REQ_XCOMP_PERM:
	case ARCH_GET_XCOMP_GUEST_PERM:
	case ARCH_REQ_XCOMP_GUEST_PERM:
	case ARCH_XCOMP_TILECFG:
	case ARCH_XCOMP_TILEDATA:
	case ARCH_MAP_VDSO_X32:
	case ARCH_MAP_VDSO_32:
	case ARCH_MAP_VDSO_64:
	case ARCH_GET_UNTAG_MASK:
	case ARCH_ENABLE_TAGGED_ADDR:
	case ARCH_GET_MAX_TAG_BITS:
	case ARCH_FORCE_TAGGED_SVA:
	{
		fixme("Code %#lx not implemented", code);
		return -ENOSYS;
	}
	default:
	{
		debug("Invalid code %#lx", code);
		return -EINVAL;
	}
	}
}

/* https://man7.org/linux/man-pages/man2/reboot.2.html */
static int linux_reboot(SysFrm *, int magic, int magic2, int cmd, void *arg)
{
	if (magic != LINUX_REBOOT_MAGIC1 ||
		(magic2 != LINUX_REBOOT_MAGIC2 &&
		 magic2 != LINUX_REBOOT_MAGIC2A &&
		 magic2 != LINUX_REBOOT_MAGIC2B &&
		 magic2 != LINUX_REBOOT_MAGIC2C))
	{
		warn("Invalid magic %#x %#x", magic, magic2);
		return -EINVAL;
	}

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	debug("cmd=%#x arg=%#lx", cmd, arg);
	switch ((unsigned int)cmd)
	{
	case LINUX_REBOOT_CMD_RESTART:
	{
		KPrint("Restarting system.");

		Tasking::Task *ctx = pcb->GetContext();
		ctx->CreateThread(ctx->GetKernelProcess(),
						  Tasking::IP(KST_Reboot))
			->Rename("Restart");
		return 0;
	}
	case LINUX_REBOOT_CMD_HALT:
	{
		KPrint("System halted.");

		pcb->GetContext()->Panic();
		CPU::Stop();
	}
	case LINUX_REBOOT_CMD_POWER_OFF:
	{
		KPrint("Power down.");

		Tasking::Task *ctx = pcb->GetContext();
		ctx->CreateThread(ctx->GetKernelProcess(),
						  Tasking::IP(KST_Shutdown))
			->Rename("Shutdown");
		return 0;
	}
	case LINUX_REBOOT_CMD_RESTART2:
	{
		void *pArg = vma->__UserCheckAndGetAddress(arg, sizeof(void *));
		if (pArg == nullptr)
			return -EFAULT;

		KPrint("Restarting system with command '%s'",
			   (const char *)pArg);

		Tasking::Task *ctx = pcb->GetContext();
		ctx->CreateThread(ctx->GetKernelProcess(),
						  Tasking::IP(KST_Reboot))
			->Rename("Restart");
		break;
	}
	case LINUX_REBOOT_CMD_CAD_ON:
	case LINUX_REBOOT_CMD_CAD_OFF:
	case LINUX_REBOOT_CMD_SW_SUSPEND:
	case LINUX_REBOOT_CMD_KEXEC:
	{
		fixme("cmd %#x not implemented", cmd);
		return -ENOSYS;
	}
	default:
	{
		debug("Invalid cmd %#x", cmd);
		return -EINVAL;
	}
	}
	return 0;
}

/* https://man7.org/linux/man-pages/man2/sigaction.2.html */
static int linux_sigaction(SysFrm *, int signum,
						   const struct sigaction *act,
						   struct sigaction *oldact)
{
	if (signum == linux_SIGKILL || signum == linux_SIGSTOP)
	{
		debug("Invalid signal %d", signum);
		return -EINVAL;
	}

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	debug("signum=%d act=%#lx oldact=%#lx", signum, act, oldact);

	if (vma->UserCheck(act) < 0 && act != nullptr)
		return -EFAULT;
	if (vma->UserCheck(oldact) < 0 && oldact != nullptr)
		return -EFAULT;

	auto pAct = pcb->PageTable->Get(act);
	auto pOldact = pcb->PageTable->Get(oldact);
	int ret = 0;

	if (pOldact)
		ret = pcb->Signals->GetAction(signum, pOldact);

	if (unlikely(ret < 0))
		return ret;

	if (pAct)
		ret = pcb->Signals->SetAction(signum, *pAct);

	return ret;
}

/* https://man7.org/linux/man-pages/man2/sigprocmask.2.html */
static int linux_sigprocmask(SysFrm *, int how, const sigset_t *set,
							 sigset_t *oldset, size_t sigsetsize)
{
	static_assert(sizeof(sigset_t) < PAGE_SIZE);

	if (sigsetsize != sizeof(sigset_t))
	{
		warn("Unsupported sigsetsize %d!", sigsetsize);
		return -EINVAL;
	}

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (vma->UserCheck(set) < 0 && set != nullptr)
		return -EFAULT;
	if (vma->UserCheck(oldset) < 0 && oldset != nullptr)
		return -EFAULT;

	const sigset_t *pSet = (const sigset_t *)pcb->PageTable->Get((void *)set);
	sigset_t *pOldset = (sigset_t *)pcb->PageTable->Get(oldset);

	debug("how=%#x set=%#lx oldset=%#lx",
		  how, pSet ? *pSet : 0, pOldset ? *pOldset : 0);

	if (pOldset)
		*pOldset = pcb->Signals->GetMask();

	if (!pSet)
		return 0;

	switch (how)
	{
	case SIG_BLOCK:
		pcb->Signals->Block(*pSet);
		break;
	case SIG_UNBLOCK:
		pcb->Signals->Unblock(*pSet);
		break;
	case SIG_SETMASK:
		pcb->Signals->SetMask(*pSet);
		break;
	default:
		warn("Invalid how %#x", how);
		return -EINVAL;
	}
	return 0;
}

/* https://man7.org/linux/man-pages/man2/sigreturn.2.html */
static void linux_sigreturn(SysFrm *sf)
{
	thisProcess->Signals->RestoreHandleSignal(sf);
}

/* https://man7.org/linux/man-pages/man2/gettid.2.html */
static pid_t linux_gettid(SysFrm *)
{
	return thisThread->ID;
}

/* https://man7.org/linux/man-pages/man2/set_tid_address.2.html */
static pid_t linux_set_tid_address(SysFrm *, int *tidptr)
{
	if (tidptr == nullptr)
		return -EINVAL;

	Tasking::TCB *tcb = thisThread;

	tcb->Linux.clear_child_tid = tidptr;
	return tcb->ID;
}

/* https://man7.org/linux/man-pages/man2/getdents.2.html */
static ssize_t linux_getdents64(SysFrm *, int fd, struct linux_dirent64 *dirp,
								size_t count)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (count < sizeof(struct linux_dirent64))
	{
		debug("Invalid count %d", count);
		return -EINVAL;
	}

	vfs::FileDescriptorTable::Fildes &
		fildes = fdt->GetDescriptor(fd);
	if (!fildes.Handle)
	{
		debug("Invalid fd %d", fd);
		return -EBADF;
	}

	if (fildes.Handle->node->Type != vfs::NodeType::DIRECTORY)
	{
		debug("Invalid node type %d",
			  fildes.Handle->node->Type);
		return -ENOTDIR;
	}

	auto pDirp = vma->UserCheckAndGetAddress(dirp);
	if (pDirp == nullptr)
		return -EFAULT;

	UNUSED(pDirp);
	stub;
	return -ENOSYS;
}

/* https://man7.org/linux/man-pages/man3/clock_gettime.3.html */
static int linux_clock_gettime(SysFrm *, clockid_t clockid, struct timespec *tp)
{
	static_assert(sizeof(struct timespec) < PAGE_SIZE);

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	timespec *pTp = vma->UserCheckAndGetAddress(tp);
	if (pTp == nullptr)
		return -EFAULT;

	/* FIXME: This is not correct? */
	switch (clockid)
	{
	case CLOCK_REALTIME:
	{
		uint64_t time = TimeManager->GetCounter();
		pTp->tv_sec = time / Time::ConvertUnit(Time::Seconds);
		pTp->tv_nsec = time / Time::ConvertUnit(Time::Nanoseconds);
		debug("time=%ld sec=%ld nsec=%ld",
			  time, pTp->tv_sec, pTp->tv_nsec);
		break;
	}
	case CLOCK_MONOTONIC:
	{
		uint64_t time = TimeManager->GetCounter();
		pTp->tv_sec = time / Time::ConvertUnit(Time::Seconds);
		pTp->tv_nsec = time / Time::ConvertUnit(Time::Nanoseconds);
		debug("time=%ld sec=%ld nsec=%ld",
			  time, pTp->tv_sec, pTp->tv_nsec);
		break;
	}
	case CLOCK_PROCESS_CPUTIME_ID:
	case CLOCK_THREAD_CPUTIME_ID:
	case CLOCK_MONOTONIC_RAW:
	case CLOCK_REALTIME_COARSE:
	case CLOCK_MONOTONIC_COARSE:
	case CLOCK_BOOTTIME:
	case CLOCK_REALTIME_ALARM:
	case CLOCK_BOOTTIME_ALARM:
	case CLOCK_SGI_CYCLE:
	case CLOCK_TAI:
	{
		fixme("clockid %d is stub", clockid);
		return -ENOSYS;
	}
	default:
	{
		warn("Invalid clockid %#lx", clockid);
		return -EINVAL;
	}
	}
	return 0;
}

static int linux_clock_nanosleep(SysFrm *, clockid_t clockid, int flags,
								 const struct timespec *request,
								 struct timespec *remain)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const timespec *pRequest = vma->UserCheckAndGetAddress(request);
	timespec *pRemain = vma->UserCheckAndGetAddress(remain);
	if (pRequest == nullptr)
		return -EFAULT;

	UNUSED(pRemain);
	UNUSED(flags);

	switch (clockid)
	{
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC:
	{
		uint64_t time = TimeManager->GetCounter();
		uint64_t rqTime = pRequest->tv_sec * Time::ConvertUnit(Time::Seconds) +
						  pRequest->tv_nsec * Time::ConvertUnit(Time::Nanoseconds);

		debug("Sleeping for %ld", rqTime - time);
		if (rqTime > time)
			pcb->GetContext()->Sleep(rqTime - time);
		break;
	}
	case CLOCK_PROCESS_CPUTIME_ID:
	case CLOCK_THREAD_CPUTIME_ID:
	case CLOCK_MONOTONIC_RAW:
	case CLOCK_REALTIME_COARSE:
	case CLOCK_MONOTONIC_COARSE:
	case CLOCK_BOOTTIME:
	case CLOCK_REALTIME_ALARM:
	case CLOCK_BOOTTIME_ALARM:
	case CLOCK_SGI_CYCLE:
	case CLOCK_TAI:
	{
		fixme("clockid %d is stub", clockid);
		return -ENOSYS;
	}
	default:
	{
		warn("Invalid clockid %#lx", clockid);
		return -EINVAL;
	}
	}
	return 0;
}

/* https://man7.org/linux/man-pages/man2/exit_group.2.html */
static __noreturn void linux_exit_group(SysFrm *sf, int status)
{
	fixme("status=%d", status);
	linux_exit(sf, status);
}

/* https://man7.org/linux/man-pages/man2/tgkill.2.html */
static int linux_tgkill(SysFrm *sf, pid_t tgid, pid_t tid, int sig)
{
	Tasking::TCB *target = thisProcess->GetContext()->GetThreadByID(tid, thisProcess);
	if (!target)
		return -ESRCH;

	fixme("semi-stub: %d %d %d", tgid, tid, sig);
	return target->Parent->Signals->SendSignal(sig);
}

/* https://man7.org/linux/man-pages/man2/open.2.html */
static int linux_openat(SysFrm *, int dirfd, const char *pathname, int flags, mode_t mode)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const char *pPathname = vma->UserCheckAndGetAddress(pathname);
	if (pPathname == nullptr)
		return -EFAULT;

	debug("dirfd=%d pathname=%s flags=%#x mode=%#x",
		  dirfd, pPathname, flags, mode);

	if (dirfd == AT_FDCWD)
	{
		vfs::RefNode *absoluteNode = fs->Open(pPathname, pcb->CurrentWorkingDirectory);
		if (!absoluteNode)
			return -ENOENT;

		const char *absPath = new char[strlen(absoluteNode->node->FullPath) + 1];
		strcpy((char *)absPath, absoluteNode->node->FullPath);
		delete absoluteNode;
		return fdt->_open(absPath, flags, mode);
	}

	if (!fs->PathIsRelative(pPathname))
		return fdt->_open(pPathname, flags, mode);

	fixme("dirfd=%d is stub", dirfd);
	return -ENOSYS;
}

/* Undocumented? */
static long linux_newfstatat(SysFrm *, int dfd, const char *filename,
							 struct stat *statbuf, int flag)
{
	/* FIXME: This function is not working at all? */

	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (flag)
		fixme("flag %#x is stub", flag);

	if (dfd == AT_FDCWD)
	{
		fixme("dfd AT_FDCWD is stub");
		return -ENOSYS;
	}

	vfs::FileDescriptorTable::Fildes &
		fildes = fdt->GetDescriptor(dfd);
	if (!fildes.Handle)
	{
		debug("Invalid fd %d", dfd);
		return -EBADF;
	}

	const char *pFilename = vma->UserCheckAndGetAddress(filename);
	struct stat *pStatbuf = vma->UserCheckAndGetAddress(statbuf);
	if (pFilename == nullptr || pStatbuf == nullptr)
		return -EFAULT;

	debug("%s %#lx %#lx", pFilename, filename, statbuf);
	return fdt->_stat(pFilename, pStatbuf);
}

/* https://man7.org/linux/man-pages/man2/pipe2.2.html */
static int linux_pipe2(SysFrm *sf, int pipefd[2], int flags)
{
	if (flags == 0)
		return linux_pipe(sf, pipefd);

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	int *pPipefd = vma->UserCheckAndGetAddress(pipefd);
	if (pPipefd == nullptr)
		return -EFAULT;

	debug("pipefd=%#lx", pPipefd);
	fixme("pipefd=[%d, %d] flags=%#x", pPipefd[0], pPipefd[1], flags);
	return -ENOSYS;
}

/* https://man7.org/linux/man-pages/man2/getrlimit.2.html */
static int linux_prlimit64(SysFrm *, pid_t pid, int resource,
						   const struct rlimit *new_limit,
						   struct rlimit *old_limit)
{
	static_assert(sizeof(struct rlimit) < PAGE_SIZE);

	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pOldLimit = vma->UserCheckAndGetAddress(old_limit);
	auto pNewLimit = vma->UserCheckAndGetAddress(new_limit);
	if (pOldLimit == nullptr && old_limit != nullptr)
		return -EFAULT;

	if (pNewLimit == nullptr && new_limit != nullptr)
		return -EFAULT;

	UNUSED(pOldLimit);
	UNUSED(pNewLimit);

	switch (resource)
	{
	case RLIMIT_NOFILE:
	{
		fixme("Setting RLIMIT_NOFILE is stub");
		return 0;
	}
	case RLIMIT_STACK:
	{
		fixme("Setting RLIMIT_STACK is stub");
		return 0;
	}
	case RLIMIT_CPU:
	case RLIMIT_FSIZE:
	case RLIMIT_DATA:
	case RLIMIT_CORE:
	case RLIMIT_RSS:
	case RLIMIT_NPROC:
	case RLIMIT_MEMLOCK:
	case RLIMIT_AS:
	case RLIMIT_LOCKS:
	case RLIMIT_SIGPENDING:
	case RLIMIT_MSGQUEUE:
	case RLIMIT_NICE:
	case RLIMIT_RTPRIO:
	case RLIMIT_RTTIME:
	case RLIMIT_NLIMITS:
	{
		fixme("resource %d is stub", resource);
		return -ENOSYS;
	}
	default:
	{
		debug("Invalid resource %d", resource);
		return -EINVAL;
	}
	}

	return 0;
}

/* https://man7.org/linux/man-pages/man2/getrandom.2.html */
static ssize_t linux_getrandom(SysFrm *, void *buf,
							   size_t buflen, unsigned int flags)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	if (flags & GRND_NONBLOCK)
		fixme("GRND_NONBLOCK not implemented");

	if (flags & ~(GRND_NONBLOCK |
				  GRND_RANDOM |
				  GRND_INSECURE))
	{
		warn("Invalid flags %#x", flags);
		return -EINVAL;
	}

	auto pBuf = vma->UserCheckAndGetAddress(buf, buflen);
	if (pBuf == nullptr)
		return -EFAULT;

	if (flags & GRND_RANDOM)
	{
		uint16_t random;
		for (size_t i = 0; i < buflen; i++)
		{
			random = Random::rand16();
			{
				Memory::SwapPT swap(pcb->PageTable);
				((uint8_t *)pBuf)[i] = uint8_t(random & 0xFF);
			}
		}
		return buflen;
	}

	return 0;
}

static SyscallData LinuxSyscallsTableAMD64[] = {
	[__NR_amd64_read] = {"read", (void *)linux_read},
	[__NR_amd64_write] = {"write", (void *)linux_write},
	[__NR_amd64_open] = {"open", (void *)linux_open},
	[__NR_amd64_close] = {"close", (void *)linux_close},
	[__NR_amd64_stat] = {"stat", (void *)linux_stat},
	[__NR_amd64_fstat] = {"fstat", (void *)linux_fstat},
	[__NR_amd64_lstat] = {"lstat", (void *)linux_lstat},
	[__NR_amd64_poll] = {"poll", (void *)nullptr},
	[__NR_amd64_lseek] = {"lseek", (void *)linux_lseek},
	[__NR_amd64_mmap] = {"mmap", (void *)linux_mmap},
	[__NR_amd64_mprotect] = {"mprotect", (void *)linux_mprotect},
	[__NR_amd64_munmap] = {"munmap", (void *)linux_munmap},
	[__NR_amd64_brk] = {"brk", (void *)linux_brk},
	[__NR_amd64_rt_sigaction] = {"rt_sigaction", (void *)linux_sigaction},
	[__NR_amd64_rt_sigprocmask] = {"rt_sigprocmask", (void *)linux_sigprocmask},
	[__NR_amd64_rt_sigreturn] = {"rt_sigreturn", (void *)linux_sigreturn},
	[__NR_amd64_ioctl] = {"ioctl", (void *)linux_ioctl},
	[__NR_amd64_pread64] = {"pread64", (void *)linux_pread64},
	[__NR_amd64_pwrite64] = {"pwrite64", (void *)linux_pwrite64},
	[__NR_amd64_readv] = {"readv", (void *)linux_readv},
	[__NR_amd64_writev] = {"writev", (void *)linux_writev},
	[__NR_amd64_access] = {"access", (void *)linux_access},
	[__NR_amd64_pipe] = {"pipe", (void *)linux_pipe},
	[__NR_amd64_select] = {"select", (void *)nullptr},
	[__NR_amd64_sched_yield] = {"sched_yield", (void *)nullptr},
	[__NR_amd64_mremap] = {"mremap", (void *)nullptr},
	[__NR_amd64_msync] = {"msync", (void *)nullptr},
	[__NR_amd64_mincore] = {"mincore", (void *)nullptr},
	[__NR_amd64_madvise] = {"madvise", (void *)nullptr},
	[__NR_amd64_shmget] = {"shmget", (void *)nullptr},
	[__NR_amd64_shmat] = {"shmat", (void *)nullptr},
	[__NR_amd64_shmctl] = {"shmctl", (void *)nullptr},
	[__NR_amd64_dup] = {"dup", (void *)linux_dup},
	[__NR_amd64_dup2] = {"dup2", (void *)linux_dup2},
	[__NR_amd64_pause] = {"pause", (void *)linux_pause},
	[__NR_amd64_nanosleep] = {"nanosleep", (void *)linux_nanosleep},
	[__NR_amd64_getitimer] = {"getitimer", (void *)nullptr},
	[__NR_amd64_alarm] = {"alarm", (void *)nullptr},
	[__NR_amd64_setitimer] = {"setitimer", (void *)nullptr},
	[__NR_amd64_getpid] = {"getpid", (void *)linux_getpid},
	[__NR_amd64_sendfile] = {"sendfile", (void *)nullptr},
	[__NR_amd64_socket] = {"socket", (void *)nullptr},
	[__NR_amd64_connect] = {"connect", (void *)nullptr},
	[__NR_amd64_accept] = {"accept", (void *)nullptr},
	[__NR_amd64_sendto] = {"sendto", (void *)nullptr},
	[__NR_amd64_recvfrom] = {"recvfrom", (void *)nullptr},
	[__NR_amd64_sendmsg] = {"sendmsg", (void *)nullptr},
	[__NR_amd64_recvmsg] = {"recvmsg", (void *)nullptr},
	[__NR_amd64_shutdown] = {"shutdown", (void *)linux_shutdown},
	[__NR_amd64_bind] = {"bind", (void *)nullptr},
	[__NR_amd64_listen] = {"listen", (void *)nullptr},
	[__NR_amd64_getsockname] = {"getsockname", (void *)nullptr},
	[__NR_amd64_getpeername] = {"getpeername", (void *)nullptr},
	[__NR_amd64_socketpair] = {"socketpair", (void *)nullptr},
	[__NR_amd64_setsockopt] = {"setsockopt", (void *)nullptr},
	[__NR_amd64_getsockopt] = {"getsockopt", (void *)nullptr},
	[__NR_amd64_clone] = {"clone", (void *)nullptr},
	[__NR_amd64_fork] = {"fork", (void *)linux_fork},
	[__NR_amd64_vfork] = {"vfork", (void *)nullptr},
	[__NR_amd64_execve] = {"execve", (void *)linux_execve},
	[__NR_amd64_exit] = {"exit", (void *)linux_exit},
	[__NR_amd64_wait4] = {"wait4", (void *)linux_wait4},
	[__NR_amd64_kill] = {"kill", (void *)linux_kill},
	[__NR_amd64_uname] = {"uname", (void *)linux_uname},
	[__NR_amd64_semget] = {"semget", (void *)nullptr},
	[__NR_amd64_semop] = {"semop", (void *)nullptr},
	[__NR_amd64_semctl] = {"semctl", (void *)nullptr},
	[__NR_amd64_shmdt] = {"shmdt", (void *)nullptr},
	[__NR_amd64_msgget] = {"msgget", (void *)nullptr},
	[__NR_amd64_msgsnd] = {"msgsnd", (void *)nullptr},
	[__NR_amd64_msgrcv] = {"msgrcv", (void *)nullptr},
	[__NR_amd64_msgctl] = {"msgctl", (void *)nullptr},
	[__NR_amd64_fcntl] = {"fcntl", (void *)linux_fcntl},
	[__NR_amd64_flock] = {"flock", (void *)nullptr},
	[__NR_amd64_fsync] = {"fsync", (void *)nullptr},
	[__NR_amd64_fdatasync] = {"fdatasync", (void *)nullptr},
	[__NR_amd64_truncate] = {"truncate", (void *)nullptr},
	[__NR_amd64_ftruncate] = {"ftruncate", (void *)nullptr},
	[__NR_amd64_getdents] = {"getdents", (void *)nullptr},
	[__NR_amd64_getcwd] = {"getcwd", (void *)nullptr},
	[__NR_amd64_chdir] = {"chdir", (void *)nullptr},
	[__NR_amd64_fchdir] = {"fchdir", (void *)nullptr},
	[__NR_amd64_rename] = {"rename", (void *)nullptr},
	[__NR_amd64_mkdir] = {"mkdir", (void *)linux_mkdir},
	[__NR_amd64_rmdir] = {"rmdir", (void *)nullptr},
	[__NR_amd64_creat] = {"creat", (void *)linux_creat},
	[__NR_amd64_link] = {"link", (void *)nullptr},
	[__NR_amd64_unlink] = {"unlink", (void *)nullptr},
	[__NR_amd64_symlink] = {"symlink", (void *)nullptr},
	[__NR_amd64_readlink] = {"readlink", (void *)linux_readlink},
	[__NR_amd64_chmod] = {"chmod", (void *)nullptr},
	[__NR_amd64_fchmod] = {"fchmod", (void *)nullptr},
	[__NR_amd64_chown] = {"chown", (void *)nullptr},
	[__NR_amd64_fchown] = {"fchown", (void *)nullptr},
	[__NR_amd64_lchown] = {"lchown", (void *)nullptr},
	[__NR_amd64_umask] = {"umask", (void *)nullptr},
	[__NR_amd64_gettimeofday] = {"gettimeofday", (void *)nullptr},
	[__NR_amd64_getrlimit] = {"getrlimit", (void *)nullptr},
	[__NR_amd64_getrusage] = {"getrusage", (void *)nullptr},
	[__NR_amd64_sysinfo] = {"sysinfo", (void *)nullptr},
	[__NR_amd64_times] = {"times", (void *)nullptr},
	[__NR_amd64_ptrace] = {"ptrace", (void *)nullptr},
	[__NR_amd64_getuid] = {"getuid", (void *)linux_getuid},
	[__NR_amd64_syslog] = {"syslog", (void *)nullptr},
	[__NR_amd64_getgid] = {"getgid", (void *)linux_getgid},
	[__NR_amd64_setuid] = {"setuid", (void *)nullptr},
	[__NR_amd64_setgid] = {"setgid", (void *)nullptr},
	[__NR_amd64_geteuid] = {"geteuid", (void *)linux_geteuid},
	[__NR_amd64_getegid] = {"getegid", (void *)linux_getegid},
	[__NR_amd64_setpgid] = {"setpgid", (void *)nullptr},
	[__NR_amd64_getppid] = {"getppid", (void *)linux_getppid},
	[__NR_amd64_getpgrp] = {"getpgrp", (void *)nullptr},
	[__NR_amd64_setsid] = {"setsid", (void *)nullptr},
	[__NR_amd64_setreuid] = {"setreuid", (void *)nullptr},
	[__NR_amd64_setregid] = {"setregid", (void *)nullptr},
	[__NR_amd64_getgroups] = {"getgroups", (void *)nullptr},
	[__NR_amd64_setgroups] = {"setgroups", (void *)nullptr},
	[__NR_amd64_setresuid] = {"setresuid", (void *)nullptr},
	[__NR_amd64_getresuid] = {"getresuid", (void *)nullptr},
	[__NR_amd64_setresgid] = {"setresgid", (void *)nullptr},
	[__NR_amd64_getresgid] = {"getresgid", (void *)nullptr},
	[__NR_amd64_getpgid] = {"getpgid", (void *)nullptr},
	[__NR_amd64_setfsuid] = {"setfsuid", (void *)nullptr},
	[__NR_amd64_setfsgid] = {"setfsgid", (void *)nullptr},
	[__NR_amd64_getsid] = {"getsid", (void *)nullptr},
	[__NR_amd64_capget] = {"capget", (void *)nullptr},
	[__NR_amd64_capset] = {"capset", (void *)nullptr},
	[__NR_amd64_rt_sigpending] = {"rt_sigpending", (void *)nullptr},
	[__NR_amd64_rt_sigtimedwait] = {"rt_sigtimedwait", (void *)nullptr},
	[__NR_amd64_rt_sigqueueinfo] = {"rt_sigqueueinfo", (void *)nullptr},
	[__NR_amd64_rt_sigsuspend] = {"rt_sigsuspend", (void *)nullptr},
	[__NR_amd64_sigaltstack] = {"sigaltstack", (void *)nullptr},
	[__NR_amd64_utime] = {"utime", (void *)nullptr},
	[__NR_amd64_mknod] = {"mknod", (void *)nullptr},
	[__NR_amd64_uselib] = {"uselib", (void *)nullptr},
	[__NR_amd64_personality] = {"personality", (void *)nullptr},
	[__NR_amd64_ustat] = {"ustat", (void *)nullptr},
	[__NR_amd64_statfs] = {"statfs", (void *)nullptr},
	[__NR_amd64_fstatfs] = {"fstatfs", (void *)nullptr},
	[__NR_amd64_sysfs] = {"sysfs", (void *)nullptr},
	[__NR_amd64_getpriority] = {"getpriority", (void *)nullptr},
	[__NR_amd64_setpriority] = {"setpriority", (void *)nullptr},
	[__NR_amd64_sched_setparam] = {"sched_setparam", (void *)nullptr},
	[__NR_amd64_sched_getparam] = {"sched_getparam", (void *)nullptr},
	[__NR_amd64_sched_setscheduler] = {"sched_setscheduler", (void *)nullptr},
	[__NR_amd64_sched_getscheduler] = {"sched_getscheduler", (void *)nullptr},
	[__NR_amd64_sched_get_priority_max] = {"sched_get_priority_max", (void *)nullptr},
	[__NR_amd64_sched_get_priority_min] = {"sched_get_priority_min", (void *)nullptr},
	[__NR_amd64_sched_rr_get_interval] = {"sched_rr_get_interval", (void *)nullptr},
	[__NR_amd64_mlock] = {"mlock", (void *)nullptr},
	[__NR_amd64_munlock] = {"munlock", (void *)nullptr},
	[__NR_amd64_mlockall] = {"mlockall", (void *)nullptr},
	[__NR_amd64_munlockall] = {"munlockall", (void *)nullptr},
	[__NR_amd64_vhangup] = {"vhangup", (void *)nullptr},
	[__NR_amd64_modify_ldt] = {"modify_ldt", (void *)nullptr},
	[__NR_amd64_pivot_root] = {"pivot_root", (void *)nullptr},
	[__NR_amd64__sysctl] = {"_sysctl", (void *)nullptr},
	[__NR_amd64_prctl] = {"prctl", (void *)nullptr},
	[__NR_amd64_arch_prctl] = {"arch_prctl", (void *)linux_arch_prctl},
	[__NR_amd64_adjtimex] = {"adjtimex", (void *)nullptr},
	[__NR_amd64_setrlimit] = {"setrlimit", (void *)nullptr},
	[__NR_amd64_chroot] = {"chroot", (void *)nullptr},
	[__NR_amd64_sync] = {"sync", (void *)nullptr},
	[__NR_amd64_acct] = {"acct", (void *)nullptr},
	[__NR_amd64_settimeofday] = {"settimeofday", (void *)nullptr},
	[__NR_amd64_mount] = {"mount", (void *)nullptr},
	[__NR_amd64_umount2] = {"umount2", (void *)nullptr},
	[__NR_amd64_swapon] = {"swapon", (void *)nullptr},
	[__NR_amd64_swapoff] = {"swapoff", (void *)nullptr},
	[__NR_amd64_reboot] = {"reboot", (void *)linux_reboot},
	[__NR_amd64_sethostname] = {"sethostname", (void *)nullptr},
	[__NR_amd64_setdomainname] = {"setdomainname", (void *)nullptr},
	[__NR_amd64_iopl] = {"iopl", (void *)nullptr},
	[__NR_amd64_ioperm] = {"ioperm", (void *)nullptr},
	[__NR_amd64_create_module] = {"create_module", (void *)nullptr},
	[__NR_amd64_init_module] = {"init_module", (void *)nullptr},
	[__NR_amd64_delete_module] = {"delete_module", (void *)nullptr},
	[__NR_amd64_get_kernel_syms] = {"get_kernel_syms", (void *)nullptr},
	[__NR_amd64_query_module] = {"query_module", (void *)nullptr},
	[__NR_amd64_quotactl] = {"quotactl", (void *)nullptr},
	[__NR_amd64_nfsservctl] = {"nfsservctl", (void *)nullptr},
	[__NR_amd64_getpmsg] = {"getpmsg", (void *)nullptr},
	[__NR_amd64_putpmsg] = {"putpmsg", (void *)nullptr},
	[__NR_amd64_afs_syscall] = {"afs_syscall", (void *)nullptr},
	[__NR_amd64_tuxcall] = {"tuxcall", (void *)nullptr},
	[__NR_amd64_security] = {"security", (void *)nullptr},
	[__NR_amd64_gettid] = {"gettid", (void *)linux_gettid},
	[__NR_amd64_readahead] = {"readahead", (void *)nullptr},
	[__NR_amd64_setxattr] = {"setxattr", (void *)nullptr},
	[__NR_amd64_lsetxattr] = {"lsetxattr", (void *)nullptr},
	[__NR_amd64_fsetxattr] = {"fsetxattr", (void *)nullptr},
	[__NR_amd64_getxattr] = {"getxattr", (void *)nullptr},
	[__NR_amd64_lgetxattr] = {"lgetxattr", (void *)nullptr},
	[__NR_amd64_fgetxattr] = {"fgetxattr", (void *)nullptr},
	[__NR_amd64_listxattr] = {"listxattr", (void *)nullptr},
	[__NR_amd64_llistxattr] = {"llistxattr", (void *)nullptr},
	[__NR_amd64_flistxattr] = {"flistxattr", (void *)nullptr},
	[__NR_amd64_removexattr] = {"removexattr", (void *)nullptr},
	[__NR_amd64_lremovexattr] = {"lremovexattr", (void *)nullptr},
	[__NR_amd64_fremovexattr] = {"fremovexattr", (void *)nullptr},
	[__NR_amd64_tkill] = {"tkill", (void *)nullptr},
	[__NR_amd64_time] = {"time", (void *)nullptr},
	[__NR_amd64_futex] = {"futex", (void *)nullptr},
	[__NR_amd64_sched_setaffinity] = {"sched_setaffinity", (void *)nullptr},
	[__NR_amd64_sched_getaffinity] = {"sched_getaffinity", (void *)nullptr},
	[__NR_amd64_set_thread_area] = {"set_thread_area", (void *)nullptr},
	[__NR_amd64_io_setup] = {"io_setup", (void *)nullptr},
	[__NR_amd64_io_destroy] = {"io_destroy", (void *)nullptr},
	[__NR_amd64_io_getevents] = {"io_getevents", (void *)nullptr},
	[__NR_amd64_io_submit] = {"io_submit", (void *)nullptr},
	[__NR_amd64_io_cancel] = {"io_cancel", (void *)nullptr},
	[__NR_amd64_get_thread_area] = {"get_thread_area", (void *)nullptr},
	[__NR_amd64_lookup_dcookie] = {"lookup_dcookie", (void *)nullptr},
	[__NR_amd64_epoll_create] = {"epoll_create", (void *)nullptr},
	[__NR_amd64_epoll_ctl_old] = {"epoll_ctl_old", (void *)nullptr},
	[__NR_amd64_epoll_wait_old] = {"epoll_wait_old", (void *)nullptr},
	[__NR_amd64_remap_file_pages] = {"remap_file_pages", (void *)nullptr},
	[__NR_amd64_getdents64] = {"getdents64", (void *)linux_getdents64},
	[__NR_amd64_set_tid_address] = {"set_tid_address", (void *)linux_set_tid_address},
	[__NR_amd64_restart_syscall] = {"restart_syscall", (void *)nullptr},
	[__NR_amd64_semtimedop] = {"semtimedop", (void *)nullptr},
	[__NR_amd64_fadvise64] = {"fadvise64", (void *)nullptr},
	[__NR_amd64_timer_create] = {"timer_create", (void *)nullptr},
	[__NR_amd64_timer_settime] = {"timer_settime", (void *)nullptr},
	[__NR_amd64_timer_gettime] = {"timer_gettime", (void *)nullptr},
	[__NR_amd64_timer_getoverrun] = {"timer_getoverrun", (void *)nullptr},
	[__NR_amd64_timer_delete] = {"timer_delete", (void *)nullptr},
	[__NR_amd64_clock_settime] = {"clock_settime", (void *)nullptr},
	[__NR_amd64_clock_gettime] = {"clock_gettime", (void *)linux_clock_gettime},
	[__NR_amd64_clock_getres] = {"clock_getres", (void *)nullptr},
	[__NR_amd64_clock_nanosleep] = {"clock_nanosleep", (void *)linux_clock_nanosleep},
	[__NR_amd64_exit_group] = {"exit_group", (void *)linux_exit_group},
	[__NR_amd64_epoll_wait] = {"epoll_wait", (void *)nullptr},
	[__NR_amd64_epoll_ctl] = {"epoll_ctl", (void *)nullptr},
	[__NR_amd64_tgkill] = {"tgkill", (void *)linux_tgkill},
	[__NR_amd64_utimes] = {"utimes", (void *)nullptr},
	[__NR_amd64_vserver] = {"vserver", (void *)nullptr},
	[__NR_amd64_mbind] = {"mbind", (void *)nullptr},
	[__NR_amd64_set_mempolicy] = {"set_mempolicy", (void *)nullptr},
	[__NR_amd64_get_mempolicy] = {"get_mempolicy", (void *)nullptr},
	[__NR_amd64_mq_open] = {"mq_open", (void *)nullptr},
	[__NR_amd64_mq_unlink] = {"mq_unlink", (void *)nullptr},
	[__NR_amd64_mq_timedsend] = {"mq_timedsend", (void *)nullptr},
	[__NR_amd64_mq_timedreceive] = {"mq_timedreceive", (void *)nullptr},
	[__NR_amd64_mq_notify] = {"mq_notify", (void *)nullptr},
	[__NR_amd64_mq_getsetattr] = {"mq_getsetattr", (void *)nullptr},
	[__NR_amd64_kexec_load] = {"kexec_load", (void *)nullptr},
	[__NR_amd64_waitid] = {"waitid", (void *)nullptr},
	[__NR_amd64_add_key] = {"add_key", (void *)nullptr},
	[__NR_amd64_request_key] = {"request_key", (void *)nullptr},
	[__NR_amd64_keyctl] = {"keyctl", (void *)nullptr},
	[__NR_amd64_ioprio_set] = {"ioprio_set", (void *)nullptr},
	[__NR_amd64_ioprio_get] = {"ioprio_get", (void *)nullptr},
	[__NR_amd64_inotify_init] = {"inotify_init", (void *)nullptr},
	[__NR_amd64_inotify_add_watch] = {"inotify_add_watch", (void *)nullptr},
	[__NR_amd64_inotify_rm_watch] = {"inotify_rm_watch", (void *)nullptr},
	[__NR_amd64_migrate_pages] = {"migrate_pages", (void *)nullptr},
	[__NR_amd64_openat] = {"openat", (void *)linux_openat},
	[__NR_amd64_mkdirat] = {"mkdirat", (void *)nullptr},
	[__NR_amd64_mknodat] = {"mknodat", (void *)nullptr},
	[__NR_amd64_fchownat] = {"fchownat", (void *)nullptr},
	[__NR_amd64_futimesat] = {"futimesat", (void *)nullptr},
	[__NR_amd64_newfstatat] = {"newfstatat", (void *)linux_newfstatat},
	[__NR_amd64_unlinkat] = {"unlinkat", (void *)nullptr},
	[__NR_amd64_renameat] = {"renameat", (void *)nullptr},
	[__NR_amd64_linkat] = {"linkat", (void *)nullptr},
	[__NR_amd64_symlinkat] = {"symlinkat", (void *)nullptr},
	[__NR_amd64_readlinkat] = {"readlinkat", (void *)nullptr},
	[__NR_amd64_fchmodat] = {"fchmodat", (void *)nullptr},
	[__NR_amd64_faccessat] = {"faccessat", (void *)nullptr},
	[__NR_amd64_pselect6] = {"pselect6", (void *)nullptr},
	[__NR_amd64_ppoll] = {"ppoll", (void *)nullptr},
	[__NR_amd64_unshare] = {"unshare", (void *)nullptr},
	[__NR_amd64_set_robust_list] = {"set_robust_list", (void *)nullptr},
	[__NR_amd64_get_robust_list] = {"get_robust_list", (void *)nullptr},
	[__NR_amd64_splice] = {"splice", (void *)nullptr},
	[__NR_amd64_tee] = {"tee", (void *)nullptr},
	[__NR_amd64_sync_file_range] = {"sync_file_range", (void *)nullptr},
	[__NR_amd64_vmsplice] = {"vmsplice", (void *)nullptr},
	[__NR_amd64_move_pages] = {"move_pages", (void *)nullptr},
	[__NR_amd64_utimensat] = {"utimensat", (void *)nullptr},
	[__NR_amd64_epoll_pwait] = {"epoll_pwait", (void *)nullptr},
	[__NR_amd64_signalfd] = {"signalfd", (void *)nullptr},
	[__NR_amd64_timerfd_create] = {"timerfd_create", (void *)nullptr},
	[__NR_amd64_eventfd] = {"eventfd", (void *)nullptr},
	[__NR_amd64_fallocate] = {"fallocate", (void *)nullptr},
	[__NR_amd64_timerfd_settime] = {"timerfd_settime", (void *)nullptr},
	[__NR_amd64_timerfd_gettime] = {"timerfd_gettime", (void *)nullptr},
	[__NR_amd64_accept4] = {"accept4", (void *)nullptr},
	[__NR_amd64_signalfd4] = {"signalfd4", (void *)nullptr},
	[__NR_amd64_eventfd2] = {"eventfd2", (void *)nullptr},
	[__NR_amd64_epoll_create1] = {"epoll_create1", (void *)nullptr},
	[__NR_amd64_dup3] = {"dup3", (void *)nullptr},
	[__NR_amd64_pipe2] = {"pipe2", (void *)linux_pipe2},
	[__NR_amd64_inotify_init1] = {"inotify_init1", (void *)nullptr},
	[__NR_amd64_preadv] = {"preadv", (void *)nullptr},
	[__NR_amd64_pwritev] = {"pwritev", (void *)nullptr},
	[__NR_amd64_rt_tgsigqueueinfo] = {"rt_tgsigqueueinfo", (void *)nullptr},
	[__NR_amd64_perf_event_open] = {"perf_event_open", (void *)nullptr},
	[__NR_amd64_recvmmsg] = {"recvmmsg", (void *)nullptr},
	[__NR_amd64_fanotify_init] = {"fanotify_init", (void *)nullptr},
	[__NR_amd64_fanotify_mark] = {"fanotify_mark", (void *)nullptr},
	[__NR_amd64_prlimit64] = {"prlimit64", (void *)linux_prlimit64},
	[__NR_amd64_name_to_handle_at] = {"name_to_handle_at", (void *)nullptr},
	[__NR_amd64_open_by_handle_at] = {"open_by_handle_at", (void *)nullptr},
	[__NR_amd64_clock_adjtime] = {"clock_adjtime", (void *)nullptr},
	[__NR_amd64_syncfs] = {"syncfs", (void *)nullptr},
	[__NR_amd64_sendmmsg] = {"sendmmsg", (void *)nullptr},
	[__NR_amd64_setns] = {"setns", (void *)nullptr},
	[__NR_amd64_getcpu] = {"getcpu", (void *)nullptr},
	[__NR_amd64_process_vm_readv] = {"process_vm_readv", (void *)nullptr},
	[__NR_amd64_process_vm_writev] = {"process_vm_writev", (void *)nullptr},
	[__NR_amd64_kcmp] = {"kcmp", (void *)nullptr},
	[__NR_amd64_finit_module] = {"finit_module", (void *)nullptr},
	[__NR_amd64_sched_setattr] = {"sched_setattr", (void *)nullptr},
	[__NR_amd64_sched_getattr] = {"sched_getattr", (void *)nullptr},
	[__NR_amd64_renameat2] = {"renameat2", (void *)nullptr},
	[__NR_amd64_seccomp] = {"seccomp", (void *)nullptr},
	[__NR_amd64_getrandom] = {"getrandom", (void *)linux_getrandom},
	[__NR_amd64_memfd_create] = {"memfd_create", (void *)nullptr},
	[__NR_amd64_kexec_file_load] = {"kexec_file_load", (void *)nullptr},
	[__NR_amd64_bpf] = {"bpf", (void *)nullptr},
	[__NR_amd64_execveat] = {"execveat", (void *)nullptr},
	[__NR_amd64_userfaultfd] = {"userfaultfd", (void *)nullptr},
	[__NR_amd64_membarrier] = {"membarrier", (void *)nullptr},
	[__NR_amd64_mlock2] = {"mlock2", (void *)nullptr},
	[__NR_amd64_copy_file_range] = {"copy_file_range", (void *)nullptr},
	[__NR_amd64_preadv2] = {"preadv2", (void *)nullptr},
	[__NR_amd64_pwritev2] = {"pwritev2", (void *)nullptr},
	[__NR_amd64_pkey_mprotect] = {"pkey_mprotect", (void *)nullptr},
	[__NR_amd64_pkey_alloc] = {"pkey_alloc", (void *)nullptr},
	[__NR_amd64_pkey_free] = {"pkey_free", (void *)nullptr},
	[__NR_amd64_statx] = {"statx", (void *)nullptr},
	[__NR_amd64_io_pgetevents] = {"io_pgetevents", (void *)nullptr},
	[__NR_amd64_rseq] = {"rseq", (void *)nullptr},
	[335] = {"reserved", (void *)nullptr},
	[336] = {"reserved", (void *)nullptr},
	[337] = {"reserved", (void *)nullptr},
	[338] = {"reserved", (void *)nullptr},
	[339] = {"reserved", (void *)nullptr},
	[340] = {"reserved", (void *)nullptr},
	[341] = {"reserved", (void *)nullptr},
	[342] = {"reserved", (void *)nullptr},
	[343] = {"reserved", (void *)nullptr},
	[344] = {"reserved", (void *)nullptr},
	[345] = {"reserved", (void *)nullptr},
	[346] = {"reserved", (void *)nullptr},
	[347] = {"reserved", (void *)nullptr},
	[348] = {"reserved", (void *)nullptr},
	[349] = {"reserved", (void *)nullptr},
	[350] = {"reserved", (void *)nullptr},
	[351] = {"reserved", (void *)nullptr},
	[352] = {"reserved", (void *)nullptr},
	[353] = {"reserved", (void *)nullptr},
	[354] = {"reserved", (void *)nullptr},
	[355] = {"reserved", (void *)nullptr},
	[356] = {"reserved", (void *)nullptr},
	[357] = {"reserved", (void *)nullptr},
	[358] = {"reserved", (void *)nullptr},
	[359] = {"reserved", (void *)nullptr},
	[360] = {"reserved", (void *)nullptr},
	[361] = {"reserved", (void *)nullptr},
	[362] = {"reserved", (void *)nullptr},
	[363] = {"reserved", (void *)nullptr},
	[364] = {"reserved", (void *)nullptr},
	[365] = {"reserved", (void *)nullptr},
	[366] = {"reserved", (void *)nullptr},
	[367] = {"reserved", (void *)nullptr},
	[368] = {"reserved", (void *)nullptr},
	[369] = {"reserved", (void *)nullptr},
	[370] = {"reserved", (void *)nullptr},
	[371] = {"reserved", (void *)nullptr},
	[372] = {"reserved", (void *)nullptr},
	[373] = {"reserved", (void *)nullptr},
	[374] = {"reserved", (void *)nullptr},
	[375] = {"reserved", (void *)nullptr},
	[376] = {"reserved", (void *)nullptr},
	[377] = {"reserved", (void *)nullptr},
	[378] = {"reserved", (void *)nullptr},
	[379] = {"reserved", (void *)nullptr},
	[380] = {"reserved", (void *)nullptr},
	[381] = {"reserved", (void *)nullptr},
	[382] = {"reserved", (void *)nullptr},
	[383] = {"reserved", (void *)nullptr},
	[384] = {"reserved", (void *)nullptr},
	[385] = {"reserved", (void *)nullptr},
	[386] = {"reserved", (void *)nullptr},
	[387] = {"reserved", (void *)nullptr},
	[388] = {"reserved", (void *)nullptr},
	[389] = {"reserved", (void *)nullptr},
	[390] = {"reserved", (void *)nullptr},
	[391] = {"reserved", (void *)nullptr},
	[392] = {"reserved", (void *)nullptr},
	[393] = {"reserved", (void *)nullptr},
	[394] = {"reserved", (void *)nullptr},
	[395] = {"reserved", (void *)nullptr},
	[396] = {"reserved", (void *)nullptr},
	[397] = {"reserved", (void *)nullptr},
	[398] = {"reserved", (void *)nullptr},
	[399] = {"reserved", (void *)nullptr},
	[400] = {"reserved", (void *)nullptr},
	[401] = {"reserved", (void *)nullptr},
	[402] = {"reserved", (void *)nullptr},
	[403] = {"reserved", (void *)nullptr},
	[404] = {"reserved", (void *)nullptr},
	[405] = {"reserved", (void *)nullptr},
	[406] = {"reserved", (void *)nullptr},
	[407] = {"reserved", (void *)nullptr},
	[408] = {"reserved", (void *)nullptr},
	[409] = {"reserved", (void *)nullptr},
	[410] = {"reserved", (void *)nullptr},
	[411] = {"reserved", (void *)nullptr},
	[412] = {"reserved", (void *)nullptr},
	[413] = {"reserved", (void *)nullptr},
	[414] = {"reserved", (void *)nullptr},
	[415] = {"reserved", (void *)nullptr},
	[416] = {"reserved", (void *)nullptr},
	[417] = {"reserved", (void *)nullptr},
	[418] = {"reserved", (void *)nullptr},
	[419] = {"reserved", (void *)nullptr},
	[420] = {"reserved", (void *)nullptr},
	[421] = {"reserved", (void *)nullptr},
	[422] = {"reserved", (void *)nullptr},
	[423] = {"reserved", (void *)nullptr},
	[__NR_amd64_pidfd_send_signal] = {"pidfd_send_signal", (void *)nullptr},
	[__NR_amd64_io_uring_setup] = {"io_uring_setup", (void *)nullptr},
	[__NR_amd64_io_uring_enter] = {"io_uring_enter", (void *)nullptr},
	[__NR_amd64_io_uring_register] = {"io_uring_register", (void *)nullptr},
	[__NR_amd64_open_tree] = {"open_tree", (void *)nullptr},
	[__NR_amd64_move_mount] = {"move_mount", (void *)nullptr},
	[__NR_amd64_fsopen] = {"fsopen", (void *)nullptr},
	[__NR_amd64_fsconfig] = {"fsconfig", (void *)nullptr},
	[__NR_amd64_fsmount] = {"fsmount", (void *)nullptr},
	[__NR_amd64_fspick] = {"fspick", (void *)nullptr},
	[__NR_amd64_pidfd_open] = {"pidfd_open", (void *)nullptr},
	[__NR_amd64_clone3] = {"clone3", (void *)nullptr},
	[__NR_amd64_close_range] = {"close_range", (void *)nullptr},
	[__NR_amd64_openat2] = {"openat2", (void *)nullptr},
	[__NR_amd64_pidfd_getfd] = {"pidfd_getfd", (void *)nullptr},
	[__NR_amd64_faccessat2] = {"faccessat2", (void *)nullptr},
	[__NR_amd64_process_madvise] = {"process_madvise", (void *)nullptr},
	[__NR_amd64_epoll_pwait2] = {"epoll_pwait2", (void *)nullptr},
	[__NR_amd64_mount_setattr] = {"mount_setattr", (void *)nullptr},
	[443] = {"reserved", (void *)nullptr},
	[__NR_amd64_landlock_create_ruleset] = {"landlock_create_ruleset", (void *)nullptr},
	[__NR_amd64_landlock_add_rule] = {"landlock_add_rule", (void *)nullptr},
	[__NR_amd64_landlock_restrict_self] = {"landlock_restrict_self", (void *)nullptr},
};

static SyscallData LinuxSyscallsTableI386[] = {
	[__NR_i386_restart_syscall] = {"restart_syscall", (void *)nullptr},
	[__NR_i386_exit] = {"exit", (void *)linux_exit},
	[__NR_i386_fork] = {"fork", (void *)linux_fork},
	[__NR_i386_read] = {"read", (void *)linux_read},
	[__NR_i386_write] = {"write", (void *)linux_write},
	[__NR_i386_open] = {"open", (void *)linux_open},
	[__NR_i386_close] = {"close", (void *)linux_close},
	[__NR_i386_waitpid] = {"waitpid", (void *)nullptr},
	[__NR_i386_creat] = {"creat", (void *)linux_creat},
	[__NR_i386_link] = {"link", (void *)nullptr},
	[__NR_i386_unlink] = {"unlink", (void *)nullptr},
	[__NR_i386_execve] = {"execve", (void *)linux_execve},
	[__NR_i386_chdir] = {"chdir", (void *)nullptr},
	[__NR_i386_time] = {"time", (void *)nullptr},
	[__NR_i386_mknod] = {"mknod", (void *)nullptr},
	[__NR_i386_chmod] = {"chmod", (void *)nullptr},
	[__NR_i386_lchown] = {"lchown", (void *)nullptr},
	[__NR_i386_break] = {"break", (void *)nullptr},
	[__NR_i386_oldstat] = {"oldstat", (void *)nullptr},
	[__NR_i386_lseek] = {"lseek", (void *)linux_lseek},
	[__NR_i386_getpid] = {"getpid", (void *)linux_getpid},
	[__NR_i386_mount] = {"mount", (void *)nullptr},
	[__NR_i386_umount] = {"umount", (void *)nullptr},
	[__NR_i386_setuid] = {"setuid", (void *)nullptr},
	[__NR_i386_getuid] = {"getuid", (void *)nullptr},
	[__NR_i386_stime] = {"stime", (void *)nullptr},
	[__NR_i386_ptrace] = {"ptrace", (void *)nullptr},
	[__NR_i386_alarm] = {"alarm", (void *)nullptr},
	[__NR_i386_oldfstat] = {"oldfstat", (void *)nullptr},
	[__NR_i386_pause] = {"pause", (void *)nullptr},
	[__NR_i386_utime] = {"utime", (void *)nullptr},
	[__NR_i386_stty] = {"stty", (void *)nullptr},
	[__NR_i386_gtty] = {"gtty", (void *)nullptr},
	[__NR_i386_access] = {"access", (void *)linux_access},
	[__NR_i386_nice] = {"nice", (void *)nullptr},
	[__NR_i386_ftime] = {"ftime", (void *)nullptr},
	[__NR_i386_sync] = {"sync", (void *)nullptr},
	[__NR_i386_kill] = {"kill", (void *)linux_kill},
	[__NR_i386_rename] = {"rename", (void *)nullptr},
	[__NR_i386_mkdir] = {"mkdir", (void *)linux_mkdir},
	[__NR_i386_rmdir] = {"rmdir", (void *)nullptr},
	[__NR_i386_dup] = {"dup", (void *)linux_dup},
	[__NR_i386_pipe] = {"pipe", (void *)nullptr},
	[__NR_i386_times] = {"times", (void *)nullptr},
	[__NR_i386_prof] = {"prof", (void *)nullptr},
	[__NR_i386_brk] = {"brk", (void *)linux_brk},
	[__NR_i386_setgid] = {"setgid", (void *)nullptr},
	[__NR_i386_getgid] = {"getgid", (void *)linux_getgid},
	[__NR_i386_signal] = {"signal", (void *)nullptr},
	[__NR_i386_geteuid] = {"geteuid", (void *)linux_geteuid},
	[__NR_i386_getegid] = {"getegid", (void *)linux_getegid},
	[__NR_i386_acct] = {"acct", (void *)nullptr},
	[__NR_i386_umount2] = {"umount2", (void *)nullptr},
	[__NR_i386_lock] = {"lock", (void *)nullptr},
	[__NR_i386_ioctl] = {"ioctl", (void *)linux_ioctl},
	[__NR_i386_fcntl] = {"fcntl", (void *)linux_fcntl},
	[__NR_i386_mpx] = {"mpx", (void *)nullptr},
	[__NR_i386_setpgid] = {"setpgid", (void *)nullptr},
	[__NR_i386_ulimit] = {"ulimit", (void *)nullptr},
	[__NR_i386_oldolduname] = {"oldolduname", (void *)nullptr},
	[__NR_i386_umask] = {"umask", (void *)nullptr},
	[__NR_i386_chroot] = {"chroot", (void *)nullptr},
	[__NR_i386_ustat] = {"ustat", (void *)nullptr},
	[__NR_i386_dup2] = {"dup2", (void *)linux_dup2},
	[__NR_i386_getppid] = {"getppid", (void *)linux_getppid},
	[__NR_i386_getpgrp] = {"getpgrp", (void *)nullptr},
	[__NR_i386_setsid] = {"setsid", (void *)nullptr},
	[__NR_i386_sigaction] = {"sigaction", (void *)nullptr},
	[__NR_i386_sgetmask] = {"sgetmask", (void *)nullptr},
	[__NR_i386_ssetmask] = {"ssetmask", (void *)nullptr},
	[__NR_i386_setreuid] = {"setreuid", (void *)nullptr},
	[__NR_i386_setregid] = {"setregid", (void *)nullptr},
	[__NR_i386_sigsuspend] = {"sigsuspend", (void *)nullptr},
	[__NR_i386_sigpending] = {"sigpending", (void *)nullptr},
	[__NR_i386_sethostname] = {"sethostname", (void *)nullptr},
	[__NR_i386_setrlimit] = {"setrlimit", (void *)nullptr},
	[__NR_i386_getrlimit] = {"getrlimit", (void *)nullptr},
	[__NR_i386_getrusage] = {"getrusage", (void *)nullptr},
	[__NR_i386_gettimeofday_time32] = {"gettimeofday_time32", (void *)nullptr},
	[__NR_i386_settimeofday_time32] = {"settimeofday_time32", (void *)nullptr},
	[__NR_i386_getgroups] = {"getgroups", (void *)nullptr},
	[__NR_i386_setgroups] = {"setgroups", (void *)nullptr},
	[__NR_i386_select] = {"select", (void *)nullptr},
	[__NR_i386_symlink] = {"symlink", (void *)nullptr},
	[__NR_i386_oldlstat] = {"oldlstat", (void *)nullptr},
	[__NR_i386_readlink] = {"readlink", (void *)linux_readlink},
	[__NR_i386_uselib] = {"uselib", (void *)nullptr},
	[__NR_i386_swapon] = {"swapon", (void *)nullptr},
	[__NR_i386_reboot] = {"reboot", (void *)linux_reboot},
	[__NR_i386_readdir] = {"readdir", (void *)nullptr},
	[__NR_i386_mmap] = {"mmap", (void *)linux_mmap},
	[__NR_i386_munmap] = {"munmap", (void *)linux_munmap},
	[__NR_i386_truncate] = {"truncate", (void *)nullptr},
	[__NR_i386_ftruncate] = {"ftruncate", (void *)nullptr},
	[__NR_i386_fchmod] = {"fchmod", (void *)nullptr},
	[__NR_i386_fchown] = {"fchown", (void *)nullptr},
	[__NR_i386_getpriority] = {"getpriority", (void *)nullptr},
	[__NR_i386_setpriority] = {"setpriority", (void *)nullptr},
	[__NR_i386_profil] = {"profil", (void *)nullptr},
	[__NR_i386_statfs] = {"statfs", (void *)nullptr},
	[__NR_i386_fstatfs] = {"fstatfs", (void *)nullptr},
	[__NR_i386_ioperm] = {"ioperm", (void *)nullptr},
	[__NR_i386_socketcall] = {"socketcall", (void *)nullptr},
	[__NR_i386_syslog] = {"syslog", (void *)nullptr},
	[__NR_i386_setitimer] = {"setitimer", (void *)nullptr},
	[__NR_i386_getitimer] = {"getitimer", (void *)nullptr},
	[__NR_i386_stat] = {"stat", (void *)linux_stat},
	[__NR_i386_lstat] = {"lstat", (void *)linux_lstat},
	[__NR_i386_fstat] = {"fstat", (void *)linux_fstat},
	[__NR_i386_olduname] = {"olduname", (void *)nullptr},
	[__NR_i386_iopl] = {"iopl", (void *)nullptr},
	[__NR_i386_vhangup] = {"vhangup", (void *)nullptr},
	[__NR_i386_idle] = {"idle", (void *)nullptr},
	[__NR_i386_vm86old] = {"vm86old", (void *)nullptr},
	[__NR_i386_wait4] = {"wait4", (void *)linux_wait4},
	[__NR_i386_swapoff] = {"swapoff", (void *)nullptr},
	[__NR_i386_sysinfo] = {"sysinfo", (void *)nullptr},
	[__NR_i386_ipc] = {"ipc", (void *)nullptr},
	[__NR_i386_fsync] = {"fsync", (void *)nullptr},
	[__NR_i386_sigreturn] = {"sigreturn", (void *)nullptr},
	[__NR_i386_clone] = {"clone", (void *)nullptr},
	[__NR_i386_setdomainname] = {"setdomainname", (void *)nullptr},
	[__NR_i386_uname] = {"uname", (void *)linux_uname},
	[__NR_i386_modify_ldt] = {"modify_ldt", (void *)nullptr},
	[__NR_i386_adjtimex] = {"adjtimex", (void *)nullptr},
	[__NR_i386_mprotect] = {"mprotect", (void *)linux_mprotect},
	[__NR_i386_sigprocmask] = {"sigprocmask", (void *)nullptr},
	[__NR_i386_create_module] = {"create_module", (void *)nullptr},
	[__NR_i386_init_module] = {"init_module", (void *)nullptr},
	[__NR_i386_delete_module] = {"delete_module", (void *)nullptr},
	[__NR_i386_get_kernel_syms] = {"get_kernel_syms", (void *)nullptr},
	[__NR_i386_quotactl] = {"quotactl", (void *)nullptr},
	[__NR_i386_getpgid] = {"getpgid", (void *)nullptr},
	[__NR_i386_fchdir] = {"fchdir", (void *)nullptr},
	[__NR_i386_bdflush] = {"bdflush", (void *)nullptr},
	[__NR_i386_sysfs] = {"sysfs", (void *)nullptr},
	[__NR_i386_personality] = {"personality", (void *)nullptr},
	[__NR_i386_afs_syscall] = {"afs_syscall", (void *)nullptr},
	[__NR_i386_setfsuid] = {"setfsuid", (void *)nullptr},
	[__NR_i386_setfsgid] = {"setfsgid", (void *)nullptr},
	[__NR_i386__llseek] = {"_llseek", (void *)nullptr},
	[__NR_i386_getdents] = {"getdents", (void *)nullptr},
	[__NR_i386__newselect] = {"_newselect", (void *)nullptr},
	[__NR_i386_flock] = {"flock", (void *)nullptr},
	[__NR_i386_msync] = {"msync", (void *)nullptr},
	[__NR_i386_readv] = {"readv", (void *)linux_readv},
	[__NR_i386_writev] = {"writev", (void *)linux_writev},
	[__NR_i386_getsid] = {"getsid", (void *)nullptr},
	[__NR_i386_fdatasync] = {"fdatasync", (void *)nullptr},
	[__NR_i386__sysctl] = {"_sysctl", (void *)nullptr},
	[__NR_i386_mlock] = {"mlock", (void *)nullptr},
	[__NR_i386_munlock] = {"munlock", (void *)nullptr},
	[__NR_i386_mlockall] = {"mlockall", (void *)nullptr},
	[__NR_i386_munlockall] = {"munlockall", (void *)nullptr},
	[__NR_i386_sched_setparam] = {"sched_setparam", (void *)nullptr},
	[__NR_i386_sched_getparam] = {"sched_getparam", (void *)nullptr},
	[__NR_i386_sched_setscheduler] = {"sched_setscheduler", (void *)nullptr},
	[__NR_i386_sched_getscheduler] = {"sched_getscheduler", (void *)nullptr},
	[__NR_i386_sched_yield] = {"sched_yield", (void *)nullptr},
	[__NR_i386_sched_get_priority_max] = {"sched_get_priority_max", (void *)nullptr},
	[__NR_i386_sched_get_priority_min] = {"sched_get_priority_min", (void *)nullptr},
	[__NR_i386_sched_rr_get_interval] = {"sched_rr_get_interval", (void *)nullptr},
	[__NR_i386_nanosleep] = {"nanosleep", (void *)nullptr},
	[__NR_i386_mremap] = {"mremap", (void *)nullptr},
	[__NR_i386_setresuid] = {"setresuid", (void *)nullptr},
	[__NR_i386_getresuid] = {"getresuid", (void *)nullptr},
	[__NR_i386_vm86] = {"vm86", (void *)nullptr},
	[__NR_i386_query_module] = {"query_module", (void *)nullptr},
	[__NR_i386_poll] = {"poll", (void *)nullptr},
	[__NR_i386_nfsservctl] = {"nfsservctl", (void *)nullptr},
	[__NR_i386_setresgid] = {"setresgid", (void *)nullptr},
	[__NR_i386_getresgid] = {"getresgid", (void *)nullptr},
	[__NR_i386_prctl] = {"prctl", (void *)nullptr},
	[__NR_i386_rt_sigreturn] = {"rt_sigreturn", (void *)linux_sigreturn},
	[__NR_i386_rt_sigaction] = {"rt_sigaction", (void *)nullptr},
	[__NR_i386_rt_sigprocmask] = {"rt_sigprocmask", (void *)linux_sigprocmask},
	[__NR_i386_rt_sigpending] = {"rt_sigpending", (void *)nullptr},
	[__NR_i386_rt_sigtimedwait] = {"rt_sigtimedwait", (void *)nullptr},
	[__NR_i386_rt_sigqueueinfo] = {"rt_sigqueueinfo", (void *)nullptr},
	[__NR_i386_rt_sigsuspend] = {"rt_sigsuspend", (void *)nullptr},
	[__NR_i386_pread64] = {"pread64", (void *)linux_pread64},
	[__NR_i386_pwrite64] = {"pwrite64", (void *)linux_pwrite64},
	[__NR_i386_chown] = {"chown", (void *)nullptr},
	[__NR_i386_getcwd] = {"getcwd", (void *)nullptr},
	[__NR_i386_capget] = {"capget", (void *)nullptr},
	[__NR_i386_capset] = {"capset", (void *)nullptr},
	[__NR_i386_sigaltstack] = {"sigaltstack", (void *)nullptr},
	[__NR_i386_sendfile] = {"sendfile", (void *)nullptr},
	[__NR_i386_getpmsg] = {"getpmsg", (void *)nullptr},
	[__NR_i386_putpmsg] = {"putpmsg", (void *)nullptr},
	[__NR_i386_vfork] = {"vfork", (void *)nullptr},
	[__NR_i386_ugetrlimit] = {"ugetrlimit", (void *)nullptr},
	[__NR_i386_mmap2] = {"mmap2", (void *)nullptr},
	[__NR_i386_truncate64] = {"truncate64", (void *)nullptr},
	[__NR_i386_ftruncate64] = {"ftruncate64", (void *)nullptr},
	[__NR_i386_stat64] = {"stat64", (void *)nullptr},
	[__NR_i386_lstat64] = {"lstat64", (void *)nullptr},
	[__NR_i386_fstat64] = {"fstat64", (void *)nullptr},
	[__NR_i386_lchown32] = {"lchown32", (void *)nullptr},
	[__NR_i386_getuid32] = {"getuid32", (void *)nullptr},
	[__NR_i386_getgid32] = {"getgid32", (void *)nullptr},
	[__NR_i386_geteuid32] = {"geteuid32", (void *)nullptr},
	[__NR_i386_getegid32] = {"getegid32", (void *)nullptr},
	[__NR_i386_setreuid32] = {"setreuid32", (void *)nullptr},
	[__NR_i386_setregid32] = {"setregid32", (void *)nullptr},
	[__NR_i386_getgroups32] = {"getgroups32", (void *)nullptr},
	[__NR_i386_setgroups32] = {"setgroups32", (void *)nullptr},
	[__NR_i386_fchown32] = {"fchown32", (void *)nullptr},
	[__NR_i386_setresuid32] = {"setresuid32", (void *)nullptr},
	[__NR_i386_getresuid32] = {"getresuid32", (void *)nullptr},
	[__NR_i386_setresgid32] = {"setresgid32", (void *)nullptr},
	[__NR_i386_getresgid32] = {"getresgid32", (void *)nullptr},
	[__NR_i386_chown32] = {"chown32", (void *)nullptr},
	[__NR_i386_setuid32] = {"setuid32", (void *)nullptr},
	[__NR_i386_setgid32] = {"setgid32", (void *)nullptr},
	[__NR_i386_setfsuid32] = {"setfsuid32", (void *)nullptr},
	[__NR_i386_setfsgid32] = {"setfsgid32", (void *)nullptr},
	[__NR_i386_pivot_root] = {"pivot_root", (void *)nullptr},
	[__NR_i386_mincore] = {"mincore", (void *)nullptr},
	[__NR_i386_madvise] = {"madvise", (void *)nullptr},
	[__NR_i386_getdents64] = {"getdents64", (void *)linux_getdents64},
	[__NR_i386_fcntl64] = {"fcntl64", (void *)nullptr},
	[222] = {"reserved", (void *)nullptr},
	[223] = {"reserved", (void *)nullptr},
	[__NR_i386_gettid] = {"gettid", (void *)linux_gettid},
	[__NR_i386_readahead] = {"readahead", (void *)nullptr},
	[__NR_i386_setxattr] = {"setxattr", (void *)nullptr},
	[__NR_i386_lsetxattr] = {"lsetxattr", (void *)nullptr},
	[__NR_i386_fsetxattr] = {"fsetxattr", (void *)nullptr},
	[__NR_i386_getxattr] = {"getxattr", (void *)nullptr},
	[__NR_i386_lgetxattr] = {"lgetxattr", (void *)nullptr},
	[__NR_i386_fgetxattr] = {"fgetxattr", (void *)nullptr},
	[__NR_i386_listxattr] = {"listxattr", (void *)nullptr},
	[__NR_i386_llistxattr] = {"llistxattr", (void *)nullptr},
	[__NR_i386_flistxattr] = {"flistxattr", (void *)nullptr},
	[__NR_i386_removexattr] = {"removexattr", (void *)nullptr},
	[__NR_i386_lremovexattr] = {"lremovexattr", (void *)nullptr},
	[__NR_i386_fremovexattr] = {"fremovexattr", (void *)nullptr},
	[__NR_i386_tkill] = {"tkill", (void *)nullptr},
	[__NR_i386_sendfile64] = {"sendfile64", (void *)nullptr},
	[__NR_i386_futex] = {"futex", (void *)nullptr},
	[__NR_i386_sched_setaffinity] = {"sched_setaffinity", (void *)nullptr},
	[__NR_i386_sched_getaffinity] = {"sched_getaffinity", (void *)nullptr},
	[__NR_i386_set_thread_area] = {"set_thread_area", (void *)nullptr},
	[__NR_i386_get_thread_area] = {"get_thread_area", (void *)nullptr},
	[__NR_i386_io_setup] = {"io_setup", (void *)nullptr},
	[__NR_i386_io_destroy] = {"io_destroy", (void *)nullptr},
	[__NR_i386_io_getevents] = {"io_getevents", (void *)nullptr},
	[__NR_i386_io_submit] = {"io_submit", (void *)nullptr},
	[__NR_i386_io_cancel] = {"io_cancel", (void *)nullptr},
	[__NR_i386_fadvise64] = {"fadvise64", (void *)nullptr},
	[__NR_i386_set_zone_reclaim] = {"set_zone_reclaim", (void *)nullptr},
	[__NR_i386_exit_group] = {"exit_group", (void *)linux_exit_group},
	[__NR_i386_lookup_dcookie] = {"lookup_dcookie", (void *)nullptr},
	[__NR_i386_epoll_create] = {"epoll_create", (void *)nullptr},
	[__NR_i386_epoll_ctl] = {"epoll_ctl", (void *)nullptr},
	[__NR_i386_epoll_wait] = {"epoll_wait", (void *)nullptr},
	[__NR_i386_remap_file_pages] = {"remap_file_pages", (void *)nullptr},
	[__NR_i386_set_tid_address] = {"set_tid_address", (void *)linux_set_tid_address},
	[__NR_i386_timer_create] = {"timer_create", (void *)nullptr},
	[__NR_i386_timer_settime32] = {"timer_settime32", (void *)nullptr},
	[__NR_i386_timer_gettime32] = {"timer_gettime32", (void *)nullptr},
	[__NR_i386_timer_getoverrun] = {"timer_getoverrun", (void *)nullptr},
	[__NR_i386_timer_delete] = {"timer_delete", (void *)nullptr},
	[__NR_i386_clock_settime32] = {"clock_settime32", (void *)nullptr},
	[__NR_i386_clock_gettime32] = {"clock_gettime32", (void *)nullptr},
	[__NR_i386_clock_getres_time32] = {"clock_getres_time32", (void *)nullptr},
	[__NR_i386_clock_nanosleep_time32] = {"clock_nanosleep_time32", (void *)nullptr},
	[__NR_i386_statfs64] = {"statfs64", (void *)nullptr},
	[__NR_i386_fstatfs64] = {"fstatfs64", (void *)nullptr},
	[__NR_i386_tgkill] = {"tgkill", (void *)linux_tgkill},
	[__NR_i386_utimes] = {"utimes", (void *)nullptr},
	[__NR_i386_fadvise64_64] = {"fadvise64_64", (void *)nullptr},
	[__NR_i386_vserver] = {"vserver", (void *)nullptr},
	[__NR_i386_mbind] = {"mbind", (void *)nullptr},
	[__NR_i386_get_mempolicy] = {"get_mempolicy", (void *)nullptr},
	[__NR_i386_set_mempolicy] = {"set_mempolicy", (void *)nullptr},
	[__NR_i386_mq_open] = {"mq_open", (void *)nullptr},
	[__NR_i386_mq_unlink] = {"mq_unlink", (void *)nullptr},
	[__NR_i386_mq_timedsend] = {"mq_timedsend", (void *)nullptr},
	[__NR_i386_mq_timedreceive] = {"mq_timedreceive", (void *)nullptr},
	[__NR_i386_mq_notify] = {"mq_notify", (void *)nullptr},
	[__NR_i386_mq_getsetattr] = {"mq_getsetattr", (void *)nullptr},
	[__NR_i386_kexec_load] = {"kexec_load", (void *)nullptr},
	[__NR_i386_waitid] = {"waitid", (void *)nullptr},
	[__NR_i386_sys_setaltroot] = {"sys_setaltroot", (void *)nullptr},
	[__NR_i386_add_key] = {"add_key", (void *)nullptr},
	[__NR_i386_request_key] = {"request_key", (void *)nullptr},
	[__NR_i386_keyctl] = {"keyctl", (void *)nullptr},
	[__NR_i386_ioprio_set] = {"ioprio_set", (void *)nullptr},
	[__NR_i386_ioprio_get] = {"ioprio_get", (void *)nullptr},
	[__NR_i386_inotify_init] = {"inotify_init", (void *)nullptr},
	[__NR_i386_inotify_add_watch] = {"inotify_add_watch", (void *)nullptr},
	[__NR_i386_inotify_rm_watch] = {"inotify_rm_watch", (void *)nullptr},
	[__NR_i386_migrate_pages] = {"migrate_pages", (void *)nullptr},
	[__NR_i386_openat] = {"openat", (void *)linux_openat},
	[__NR_i386_mkdirat] = {"mkdirat", (void *)nullptr},
	[__NR_i386_mknodat] = {"mknodat", (void *)nullptr},
	[__NR_i386_fchownat] = {"fchownat", (void *)nullptr},
	[__NR_i386_futimesat] = {"futimesat", (void *)nullptr},
	[__NR_i386_fstatat64] = {"fstatat64", (void *)nullptr},
	[__NR_i386_unlinkat] = {"unlinkat", (void *)nullptr},
	[__NR_i386_renameat] = {"renameat", (void *)nullptr},
	[__NR_i386_linkat] = {"linkat", (void *)nullptr},
	[__NR_i386_symlinkat] = {"symlinkat", (void *)nullptr},
	[__NR_i386_readlinkat] = {"readlinkat", (void *)nullptr},
	[__NR_i386_fchmodat] = {"fchmodat", (void *)nullptr},
	[__NR_i386_faccessat] = {"faccessat", (void *)nullptr},
	[__NR_i386_pselect6] = {"pselect6", (void *)nullptr},
	[__NR_i386_ppoll] = {"ppoll", (void *)nullptr},
	[__NR_i386_unshare] = {"unshare", (void *)nullptr},
	[__NR_i386_set_robust_list] = {"set_robust_list", (void *)nullptr},
	[__NR_i386_get_robust_list] = {"get_robust_list", (void *)nullptr},
	[__NR_i386_splice] = {"splice", (void *)nullptr},
	[__NR_i386_sync_file_range] = {"sync_file_range", (void *)nullptr},
	[__NR_i386_tee] = {"tee", (void *)nullptr},
	[__NR_i386_vmsplice] = {"vmsplice", (void *)nullptr},
	[__NR_i386_move_pages] = {"move_pages", (void *)nullptr},
	[__NR_i386_getcpu] = {"getcpu", (void *)nullptr},
	[__NR_i386_epoll_pwait] = {"epoll_pwait", (void *)nullptr},
	[__NR_i386_utimensat] = {"utimensat", (void *)nullptr},
	[__NR_i386_signalfd] = {"signalfd", (void *)nullptr},
	[__NR_i386_timerfd_create] = {"timerfd_create", (void *)nullptr},
	[__NR_i386_eventfd] = {"eventfd", (void *)nullptr},
	[__NR_i386_fallocate] = {"fallocate", (void *)nullptr},
	[__NR_i386_timerfd_settime32] = {"timerfd_settime32", (void *)nullptr},
	[__NR_i386_timerfd_gettime32] = {"timerfd_gettime32", (void *)nullptr},
	[__NR_i386_signalfd4] = {"signalfd4", (void *)nullptr},
	[__NR_i386_eventfd2] = {"eventfd2", (void *)nullptr},
	[__NR_i386_epoll_create1] = {"epoll_create1", (void *)nullptr},
	[__NR_i386_dup3] = {"dup3", (void *)nullptr},
	[__NR_i386_pipe2] = {"pipe2", (void *)nullptr},
	[__NR_i386_inotify_init1] = {"inotify_init1", (void *)nullptr},
	[__NR_i386_preadv] = {"preadv", (void *)nullptr},
	[__NR_i386_pwritev] = {"pwritev", (void *)nullptr},
	[__NR_i386_rt_tgsigqueueinfo] = {"rt_tgsigqueueinfo", (void *)nullptr},
	[__NR_i386_perf_event_open] = {"perf_event_open", (void *)nullptr},
	[__NR_i386_recvmmsg] = {"recvmmsg", (void *)nullptr},
	[__NR_i386_fanotify_init] = {"fanotify_init", (void *)nullptr},
	[__NR_i386_fanotify_mark] = {"fanotify_mark", (void *)nullptr},
	[__NR_i386_prlimit64] = {"prlimit64", (void *)nullptr},
	[__NR_i386_name_to_handle_at] = {"name_to_handle_at", (void *)nullptr},
	[__NR_i386_open_by_handle_at] = {"open_by_handle_at", (void *)nullptr},
	[__NR_i386_clock_adjtime] = {"clock_adjtime", (void *)nullptr},
	[__NR_i386_syncfs] = {"syncfs", (void *)nullptr},
	[__NR_i386_sendmmsg] = {"sendmmsg", (void *)nullptr},
	[__NR_i386_setns] = {"setns", (void *)nullptr},
	[__NR_i386_process_vm_readv] = {"process_vm_readv", (void *)nullptr},
	[__NR_i386_process_vm_writev] = {"process_vm_writev", (void *)nullptr},
	[__NR_i386_kcmp] = {"kcmp", (void *)nullptr},
	[__NR_i386_finit_module] = {"finit_module", (void *)nullptr},
	[__NR_i386_sched_setattr] = {"sched_setattr", (void *)nullptr},
	[__NR_i386_sched_getattr] = {"sched_getattr", (void *)nullptr},
	[__NR_i386_renameat2] = {"renameat2", (void *)nullptr},
	[__NR_i386_seccomp] = {"seccomp", (void *)nullptr},
	[__NR_i386_getrandom] = {"getrandom", (void *)linux_getrandom},
	[__NR_i386_memfd_create] = {"memfd_create", (void *)nullptr},
	[__NR_i386_bpf] = {"bpf", (void *)nullptr},
	[__NR_i386_execveat] = {"execveat", (void *)nullptr},
	[__NR_i386_socket] = {"socket", (void *)nullptr},
	[__NR_i386_socketpair] = {"socketpair", (void *)nullptr},
	[__NR_i386_bind] = {"bind", (void *)nullptr},
	[__NR_i386_connect] = {"connect", (void *)nullptr},
	[__NR_i386_listen] = {"listen", (void *)nullptr},
	[__NR_i386_accept4] = {"accept4", (void *)nullptr},
	[__NR_i386_getsockopt] = {"getsockopt", (void *)nullptr},
	[__NR_i386_setsockopt] = {"setsockopt", (void *)nullptr},
	[__NR_i386_getsockname] = {"getsockname", (void *)nullptr},
	[__NR_i386_getpeername] = {"getpeername", (void *)nullptr},
	[__NR_i386_sendto] = {"sendto", (void *)nullptr},
	[__NR_i386_sendmsg] = {"sendmsg", (void *)nullptr},
	[__NR_i386_recvfrom] = {"recvfrom", (void *)nullptr},
	[__NR_i386_recvmsg] = {"recvmsg", (void *)nullptr},
	[__NR_i386_shutdown] = {"shutdown", (void *)linux_shutdown},
	[__NR_i386_userfaultfd] = {"userfaultfd", (void *)nullptr},
	[__NR_i386_membarrier] = {"membarrier", (void *)nullptr},
	[__NR_i386_mlock2] = {"mlock2", (void *)nullptr},
	[__NR_i386_copy_file_range] = {"copy_file_range", (void *)nullptr},
	[__NR_i386_preadv2] = {"preadv2", (void *)nullptr},
	[__NR_i386_pwritev2] = {"pwritev2", (void *)nullptr},
	[__NR_i386_pkey_mprotect] = {"pkey_mprotect", (void *)nullptr},
	[__NR_i386_pkey_alloc] = {"pkey_alloc", (void *)nullptr},
	[__NR_i386_pkey_free] = {"pkey_free", (void *)nullptr},
	[__NR_i386_statx] = {"statx", (void *)nullptr},
	[__NR_i386_arch_prctl] = {"arch_prctl", (void *)linux_arch_prctl},
	[__NR_i386_io_pgetevents] = {"io_pgetevents", (void *)nullptr},
	[__NR_i386_rseq] = {"rseq", (void *)nullptr},
	[387] = {"reserved", (void *)nullptr},
	[388] = {"reserved", (void *)nullptr},
	[389] = {"reserved", (void *)nullptr},
	[390] = {"reserved", (void *)nullptr},
	[391] = {"reserved", (void *)nullptr},
	[392] = {"reserved", (void *)nullptr},
	[__NR_i386_semget] = {"semget", (void *)nullptr},
	[__NR_i386_semctl] = {"semctl", (void *)nullptr},
	[__NR_i386_shmget] = {"shmget", (void *)nullptr},
	[__NR_i386_shmctl] = {"shmctl", (void *)nullptr},
	[__NR_i386_shmat] = {"shmat", (void *)nullptr},
	[__NR_i386_shmdt] = {"shmdt", (void *)nullptr},
	[__NR_i386_msgget] = {"msgget", (void *)nullptr},
	[__NR_i386_msgsnd] = {"msgsnd", (void *)nullptr},
	[__NR_i386_msgrcv] = {"msgrcv", (void *)nullptr},
	[__NR_i386_msgctl] = {"msgctl", (void *)nullptr},
	[__NR_i386_clock_gettime64] = {"clock_gettime64", (void *)nullptr},
	[__NR_i386_clock_settime64] = {"clock_settime64", (void *)nullptr},
	[__NR_i386_clock_adjtime64] = {"clock_adjtime64", (void *)nullptr},
	[__NR_i386_clock_getres_time64] = {"clock_getres_time64", (void *)nullptr},
	[__NR_i386_clock_nanosleep_time64] = {"clock_nanosleep_time64", (void *)nullptr},
	[__NR_i386_timer_gettime64] = {"timer_gettime64", (void *)nullptr},
	[__NR_i386_timer_settime64] = {"timer_settime64", (void *)nullptr},
	[__NR_i386_timerfd_gettime64] = {"timerfd_gettime64", (void *)nullptr},
	[__NR_i386_timerfd_settime64] = {"timerfd_settime64", (void *)nullptr},
	[__NR_i386_utimensat_time64] = {"utimensat_time64", (void *)nullptr},
	[__NR_i386_pselect6_time64] = {"pselect6_time64", (void *)nullptr},
	[__NR_i386_ppoll_time64] = {"ppoll_time64", (void *)nullptr},
	[415] = {"reserved", (void *)nullptr},
	[__NR_i386_io_pgetevents_time64] = {"io_pgetevents_time64", (void *)nullptr},
	[__NR_i386_recvmmsg_time64] = {"recvmmsg_time64", (void *)nullptr},
	[__NR_i386_mq_timedsend_time64] = {"mq_timedsend_time64", (void *)nullptr},
	[__NR_i386_mq_timedreceive_time64] = {"mq_timedreceive_time64", (void *)nullptr},
	[__NR_i386_semtimedop_time64] = {"semtimedop_time64", (void *)nullptr},
	[__NR_i386_rt_sigtimedwait_time64] = {"rt_sigtimedwait_time64", (void *)nullptr},
	[__NR_i386_futex_time64] = {"futex_time64", (void *)nullptr},
	[__NR_i386_sched_rr_get_interval_time64] = {"sched_rr_get_interval_time64", (void *)nullptr},
	[__NR_i386_pidfd_send_signal] = {"pidfd_send_signal", (void *)nullptr},
	[__NR_i386_io_uring_setup] = {"io_uring_setup", (void *)nullptr},
	[__NR_i386_io_uring_enter] = {"io_uring_enter", (void *)nullptr},
	[__NR_i386_io_uring_register] = {"io_uring_register", (void *)nullptr},
	[__NR_i386_open_tree] = {"open_tree", (void *)nullptr},
	[__NR_i386_move_mount] = {"move_mount", (void *)nullptr},
	[__NR_i386_fsopen] = {"fsopen", (void *)nullptr},
	[__NR_i386_fsconfig] = {"fsconfig", (void *)nullptr},
	[__NR_i386_fsmount] = {"fsmount", (void *)nullptr},
	[__NR_i386_fspick] = {"fspick", (void *)nullptr},
	[__NR_i386_pidfd_open] = {"pidfd_open", (void *)nullptr},
	[__NR_i386_clone3] = {"clone3", (void *)nullptr},
	[__NR_i386_close_range] = {"close_range", (void *)nullptr},
	[__NR_i386_openat2] = {"openat2", (void *)nullptr},
	[__NR_i386_pidfd_getfd] = {"pidfd_getfd", (void *)nullptr},
	[__NR_i386_faccessat2] = {"faccessat2", (void *)nullptr},
	[__NR_i386_process_madvise] = {"process_madvise", (void *)nullptr},
	[__NR_i386_epoll_pwait2] = {"epoll_pwait2", (void *)nullptr},
	[__NR_i386_mount_setattr] = {"mount_setattr", (void *)nullptr},
	[443] = {"reserved", (void *)nullptr},
	[__NR_i386_landlock_create_ruleset] = {"landlock_create_ruleset", (void *)nullptr},
	[__NR_i386_landlock_add_rule] = {"landlock_add_rule", (void *)nullptr},
	[__NR_i386_landlock_restrict_self] = {"landlock_restrict_self", (void *)nullptr},
};

uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame)
{
#if defined(a64)
	if (Frame->rax > sizeof(LinuxSyscallsTableAMD64) / sizeof(SyscallData))
	{
		fixme("Syscall %d not implemented",
			  Frame->rax);
		return -ENOSYS;
	}

	SyscallData Syscall = LinuxSyscallsTableAMD64[Frame->rax];

	long (*call)(SysFrm *, long, ...) = r_cst(long (*)(SysFrm *, long, ...),
											  Syscall.Handler);

	if (unlikely(!call))
	{
		fixme("Syscall %s(%d) not implemented.",
			  Syscall.Name, Frame->rax);
		return -ENOSYS;
	}

	debug("> [%d:\"%s\"]( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax, Syscall.Name,
		  Frame->rdi, Frame->rsi, Frame->rdx,
		  Frame->r10, Frame->r8, Frame->r9);

	long sc_ret = call(Frame,
					   Frame->rdi, Frame->rsi, Frame->rdx,
					   Frame->r10, Frame->r8, Frame->r9);

	debug("< [%d:\"%s\"] = %d", Frame->rax, Syscall.Name, sc_ret);
	return sc_ret;
#elif defined(a32)
	if (Frame->eax > sizeof(LinuxSyscallsTableI386) / sizeof(SyscallData))
	{
		fixme("Syscall %d not implemented",
			  Frame->eax);
		return -ENOSYS;
	}

	SyscallData Syscall = LinuxSyscallsTableI386[Frame->eax];

	long (*call)(SysFrm *, long, ...) = r_cst(long (*)(SysFrm *, long, ...),
											  Syscall.Handler);

	if (unlikely(!call))
	{
		fixme("Syscall %s(%d) not implemented.",
			  Syscall.Name, Frame->eax);
		return -ENOSYS;
	}

	debug("> [%d:\"%s\"]( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->eax, Syscall.Name,
		  Frame->ebx, Frame->ecx, Frame->edx,
		  Frame->esi, Frame->edi, Frame->ebp);

	int sc_ret = call(Frame,
					  Frame->ebx, Frame->ecx, Frame->edx,
					  Frame->esi, Frame->edi, Frame->ebp);

	debug("< [%d:\"%s\"] = %d", Frame->eax, Syscall.Name, sc_ret);
	return sc_ret;
#elif defined(aa64)
	return -ENOSYS;
#endif

#if defined(a64)
	UNUSED(LinuxSyscallsTableI386);
#elif defined(a32)
	UNUSED(LinuxSyscallsTableAMD64);
#endif
}
