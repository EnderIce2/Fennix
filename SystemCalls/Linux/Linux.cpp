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

#include <debug.h>
#include <cpu.hpp>
#include <errno.h>

#include "../../kernel.h"
#include "linux_syscalls.hpp"

struct SyscallData
{
	const char *Name;
	void *Handler;
};

using InterProcessCommunication::IPC;
using InterProcessCommunication::IPCID;
using Tasking::TaskStatus::Ready;
using Tasking::TaskStatus::Terminated;

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

#define ARCH_GET_CPUID 0x1011
#define ARCH_SET_CPUID 0x1012

#define ARCH_GET_XCOMP_SUPP 0x1021
#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define ARCH_GET_XCOMP_GUEST_PERM 0x1024
#define ARCH_REQ_XCOMP_GUEST_PERM 0x1025

#define ARCH_XCOMP_TILECFG 17
#define ARCH_XCOMP_TILEDATA 18

#define ARCH_MAP_VDSO_X32 0x2001
#define ARCH_MAP_VDSO_32 0x2002
#define ARCH_MAP_VDSO_64 0x2003

#define ARCH_GET_UNTAG_MASK 0x4001
#define ARCH_ENABLE_TAGGED_ADDR 0x4002
#define ARCH_GET_MAX_TAG_BITS 0x4003
#define ARCH_FORCE_TAGGED_SVA 0x4004

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP 0x02000000

#define MAP_TYPE 0x0f

#define MAP_FILE 0
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_SHARED_VALIDATE 0x03
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_NORESERVE 0x4000
#define MAP_GROWSDOWN 0x0100
#define MAP_DENYWRITE 0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED 0x2000
#define MAP_POPULATE 0x8000
#define MAP_NONBLOCK 0x10000
#define MAP_STACK 0x20000
#define MAP_HUGETLB 0x40000
#define MAP_SYNC 0x80000
#define MAP_FIXED_NOREPLACE 0x100000

typedef int pid_t;

struct iovec
{
	void *iov_base;
	size_t iov_len;
};

#if defined(a64) || defined(aa64)
static ssize_t ConvertErrno(ssize_t r)
{
	if (r >= 0)
		return r;
	return -errno;
}
#endif

static int ConvertErrno(int r)
{
	if (r >= 0)
		return r;
	return -errno;
}

/* https://man7.org/linux/man-pages/man2/read.2.html */
static ssize_t sys_read(int fd, void *buf, size_t count)
{
	debug("Reading %d bytes from fd %d", count, fd);
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check(buf, Memory::US))
	{
		warn("Invalid address %#lx", buf);
		return -EFAULT;
	}
	auto pBuf = pcb->PageTable->Get(buf);

	ssize_t ret = ConvertErrno(fdt->_read(fd, pBuf, count));
	debug("Read %d bytes from fd %d, got %d", count, fd, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/write.2.html */
static ssize_t sys_write(int fd, const void *buf, size_t count)
{
	debug("Writing %d bytes to fd %d", count, fd);
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)buf, Memory::US))
	{
		warn("Invalid address %#lx", buf);
		return -EFAULT;
	}
	auto pBuf = pcb->PageTable->Get(buf);

	ssize_t ret = ConvertErrno(fdt->_write(fd, pBuf, count));
	debug("Wrote %d bytes to fd %d, got %d", count, fd, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/open.2.html */
static int sys_open(const char *pathname, int flags, mode_t mode)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)pathname, Memory::US))
	{
		warn("Invalid address %#lx", pathname);
		return -EFAULT;
	}
	auto pPathname = pcb->PageTable->Get(pathname);

	int ret = ConvertErrno(fdt->_open(pPathname, flags, mode));
	debug("Opened %s with flags %d and mode %d, got fd %d",
		  pPathname, flags, mode, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man2/close.2.html */
static int sys_close(int fd)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	int ret = ConvertErrno(fdt->_close(fd));
	debug("Closed fd %d, got %d", fd, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man3/stat.3p.html */
static int sys_stat(const char *pathname, struct stat *statbuf)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)pathname, Memory::US))
	{
		warn("Invalid address %#lx", pathname);
		return -EFAULT;
	}
	auto pPathname = pcb->PageTable->Get(pathname);

	return ConvertErrno(fdt->_stat(pPathname, statbuf));
}

/* https://man7.org/linux/man-pages/man3/fstat.3p.html */
static int sys_fstat(int fd, struct stat *statbuf)
{
#undef fstat
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)statbuf, Memory::US))
	{
		warn("Invalid address %#lx", statbuf);
		return -EFAULT;
	}
	auto pStatbuf = pcb->PageTable->Get(statbuf);

	return ConvertErrno(fdt->_fstat(fd, pStatbuf));
}

/* https://man7.org/linux/man-pages/man2/lstat.2.html */
static int sys_lstat(const char *pathname, struct stat *statbuf)
{
#undef lstat
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)pathname, Memory::US))
	{
		warn("Invalid address %#lx", pathname);
		return -EFAULT;
	}

	if (!vmm.Check((void *)statbuf, Memory::US))
	{
		warn("Invalid address %#lx", statbuf);
		return -EFAULT;
	}

	auto pPathname = pcb->PageTable->Get(pathname);
	auto pStatbuf = pcb->PageTable->Get(statbuf);

	return ConvertErrno(fdt->_lstat(pPathname, pStatbuf));
}

/* https://man7.org/linux/man-pages/man2/lseek.2.html */
static off_t sys_lseek(int fd, off_t offset, int whence)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;

	off_t ret = ConvertErrno(fdt->_lseek(fd, offset, whence));
	debug("(%d, %d, %d) = %d", fd, offset, whence, ret);
	return ret;
}

/* https://man7.org/linux/man-pages/man3/mmap.3p.html */
static void *sys_mmap(void *addr, size_t length, int prot,
					  int flags, int fildes, off_t offset)
{
	UNUSED(offset); /* FIXME */
	Tasking::PCB *pcb = thisProcess;
	Memory::MemMgr *mm = pcb->Memory;

	void *newPages = mm->RequestPages(TO_PAGES(length));
	if (newPages == nullptr)
		return (void *)-ENOMEM;

	bool MustUseAddr = (flags & MAP_FIXED) != 0;
	if (addr == NULL && !MustUseAddr)
		addr = newPages;

	if (MustUseAddr)
	{
		debug("Using fixed address %#lx", addr);
	}

	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	uint64_t MapFlags = Memory::P;
	if (prot & PROT_READ)
		MapFlags |= Memory::US;
	if (prot & PROT_WRITE)
		MapFlags |= Memory::RW;
	if (prot & PROT_EXEC)
		debug("PROT_EXEC ignored"); /* MapFlags |= Memory::XD; */

	switch (flags & MAP_TYPE)
	{
	case MAP_FILE:
		debug("MAP_FILE ignored");
		[[fallthrough]];
	case MAP_SHARED:
		fixme("MAP_SHARED not implemented");
		[[fallthrough]];
	case MAP_SHARED_VALIDATE:
		fixme("MAP_SHARED_VALIDATE not implemented");
		[[fallthrough]];
	case MAP_PRIVATE:
		debug("MAP_PRIVATE ignored");
		[[fallthrough]];
	case MAP_ANONYMOUS:
		fixme("MAP_ANONYMOUS not implemented");
		[[fallthrough]];
	default:
	{
		debug("mmap flags %#x", flags);
		break;
	}
	}

	vmm.Map(addr, newPages, length, MapFlags, Memory::Virtual::FourKiB);
	debug("Mapped %#lx to %#lx (%d pages)", addr, newPages, TO_PAGES(length));

	if (fildes != -1)
	{
		fixme("File mapping not implemented");
		mm->FreePages(newPages, TO_PAGES(length));
		return (void *)-ENOSYS;
	}

	return addr;
}

/* https://man7.org/linux/man-pages/man3/mprotect.3p.html */
static int sys_mprotect(void *addr, size_t len, int prot)
{
	Tasking::PCB *pcb = thisProcess;

	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);
	vmm.Map(addr, addr, len, Memory::P, Memory::Virtual::FourKiB);

	if (prot & PROT_READ)
		vmm.Map(addr, addr, len, Memory::P | Memory::US, Memory::Virtual::FourKiB);
	if (prot & PROT_WRITE)
		vmm.Map(addr, addr, len, Memory::RW, Memory::Virtual::FourKiB);
	if (prot & PROT_EXEC)
		debug("PROT_EXEC ignored"); /* MapFlags |= Memory::XD; */

	return 0;
}

/* https://man7.org/linux/man-pages/man3/munmap.3p.html */
static int sys_munmap(void *addr, size_t length)
{
	Tasking::PCB *pcb = thisProcess;
	Memory::MemMgr *mm = pcb->Memory;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	mm->FreePages(addr, TO_PAGES(length));
	vmm.Unmap(addr, length, Memory::Virtual::FourKiB);

	return 0;
}

/* https://man7.org/linux/man-pages/man2/brk.2.html */
static void *sys_brk(void *addr)
{
	trace("Ignoring brk syscall...");
	return (void *)-ENOSYS;
	// Tasking::PCB *pcb = thisProcess;
	// void *ret = pcb->ProgramBreak->brk(addr);
	// debug("brk(%#lx) = %#lx", addr, ret);
	// return ret;
}

/* https://man7.org/linux/man-pages/man2/ioctl.2.html */
static int sys_ioctl(int fd, unsigned long request, void *argp)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)argp, Memory::US))
	{
		warn("Invalid address %#lx", argp);
		return -EFAULT;
	}
	auto pArgp = pcb->PageTable->Get(argp);

	return ConvertErrno(fdt->_ioctl(fd, request, pArgp));
}

/* https://man7.org/linux/man-pages/man3/readv.3p.html */
static ssize_t sys_readv(int fildes, const struct iovec *iov, int iovcnt)
{
	ssize_t Total = 0;
	for (int i = 0; i < iovcnt; i++)
	{
		debug("%d: iov[%d]: %p %d", fildes, i, iov[i].iov_base, iov[i].iov_len);
		ssize_t n = sys_read(fildes, iov[i].iov_base, iov[i].iov_len);
		if (n < 0)
			return n;
		debug("n: %d", n);

		Total += n;
		if (n < (ssize_t)iov[i].iov_len)
		{
			debug("break");
			break;
		}
	}
	debug("readv: %d", Total);
	return Total;
}

/* https://man7.org/linux/man-pages/man3/writev.3p.html */
static ssize_t sys_writev(int fildes, const struct iovec *iov, int iovcnt)
{
	ssize_t Total = 0;
	for (int i = 0; i < iovcnt; i++)
	{
		debug("%d: iov[%d]: %p %d", fildes, i, iov[i].iov_base, iov[i].iov_len);
		ssize_t n = sys_write(fildes, iov[i].iov_base, iov[i].iov_len);
		if (n < 0)
			return n;
		debug("n: %d", n);

		Total += n;
		if (n < (ssize_t)iov[i].iov_len)
		{
			debug("break");
			break;
		}
	}
	debug("writev: %d", Total);
	return Total;
}

/* https://man7.org/linux/man-pages/man2/dup.2.html */
static int sys_dup(int oldfd)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_dup(oldfd));
}

/* https://man7.org/linux/man-pages/man2/dup.2.html */
static int sys_dup2(int oldfd, int newfd)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_dup2(oldfd, newfd));
}

/* https://man7.org/linux/man-pages/man3/exit.3.html */
static __noreturn void sys_exit(int status)
{
	trace("Userspace thread %s(%d) exited with code %d (%#x)",
		  thisThread->Name,
		  thisThread->ID, status,
		  status < 0 ? -status : status);

	thisThread->ExitCode = status;
	thisThread->Status = Terminated;
	TaskManager->Yield();
	__builtin_unreachable();
}

/* https://man7.org/linux/man-pages/man3/creat.3p.html */
static int sys_creat(const char *pathname, mode_t mode)
{
	Tasking::PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_creat(pathname, mode));
}

/* https://man7.org/linux/man-pages/man2/arch_prctl.2.html */
static int sys_arch_prctl(int code, unsigned long addr)
{
	Tasking::PCB *pcb = thisProcess;
	Memory::Virtual vmm = Memory::Virtual(pcb->PageTable);

	if (!vmm.Check((void *)addr))
	{
		warn("Invalid address %#lx", addr);
		return -EFAULT;
	}

	if (!vmm.Check((void *)addr, Memory::US))
	{
		warn("Address %#lx is not user accessible", addr);
		return -EPERM;
	}

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
		warn("Invalid code %#lx", code);
		return -EINVAL;
	}
	}
}

/* https://man7.org/linux/man-pages/man2/set_tid_address.2.html */
static pid_t sys_set_tid_address(int *tidptr)
{
	if (tidptr == nullptr)
		return -EINVAL;

	Tasking::TCB *tcb = thisThread;

	tcb->Linux.clear_child_tid = tidptr;
	return tcb->ID;
}

/* https://man7.org/linux/man-pages/man2/exit_group.2.html */
static __noreturn void sys_exit_group(int status)
{
	fixme("status=%d", status);
	sys_exit(status);
}

static SyscallData LinuxSyscallsTable[] = {
	[__NR_read] = {"read", (void *)sys_read},
	[__NR_write] = {"write", (void *)sys_write},
	[__NR_open] = {"open", (void *)sys_open},
	[__NR_close] = {"close", (void *)sys_close},
	[__NR_stat] = {"stat", (void *)sys_stat},
	[__NR_fstat] = {"fstat", (void *)sys_fstat},
	[__NR_lstat] = {"lstat", (void *)sys_lstat},
	[__NR_poll] = {"poll", (void *)nullptr},
	[__NR_lseek] = {"lseek", (void *)sys_lseek},
	[__NR_mmap] = {"mmap", (void *)sys_mmap},
	[__NR_mprotect] = {"mprotect", (void *)sys_mprotect},
	[__NR_munmap] = {"munmap", (void *)sys_munmap},
	[__NR_brk] = {"brk", (void *)sys_brk},
	[__NR_rt_sigaction] = {"rt_sigaction", (void *)nullptr},
	[__NR_rt_sigprocmask] = {"rt_sigprocmask", (void *)nullptr},
	[__NR_rt_sigreturn] = {"rt_sigreturn", (void *)nullptr},
	[__NR_ioctl] = {"ioctl", (void *)sys_ioctl},
	[__NR_pread64] = {"pread64", (void *)nullptr},
	[__NR_pwrite64] = {"pwrite64", (void *)nullptr},
	[__NR_readv] = {"readv", (void *)sys_readv},
	[__NR_writev] = {"writev", (void *)sys_writev},
	[__NR_access] = {"access", (void *)nullptr},
	[__NR_pipe] = {"pipe", (void *)nullptr},
	[__NR_select] = {"select", (void *)nullptr},
	[__NR_sched_yield] = {"sched_yield", (void *)nullptr},
	[__NR_mremap] = {"mremap", (void *)nullptr},
	[__NR_msync] = {"msync", (void *)nullptr},
	[__NR_mincore] = {"mincore", (void *)nullptr},
	[__NR_madvise] = {"madvise", (void *)nullptr},
	[__NR_shmget] = {"shmget", (void *)nullptr},
	[__NR_shmat] = {"shmat", (void *)nullptr},
	[__NR_shmctl] = {"shmctl", (void *)nullptr},
	[__NR_dup] = {"dup", (void *)sys_dup},
	[__NR_dup2] = {"dup2", (void *)sys_dup2},
	[__NR_pause] = {"pause", (void *)nullptr},
	[__NR_nanosleep] = {"nanosleep", (void *)nullptr},
	[__NR_getitimer] = {"getitimer", (void *)nullptr},
	[__NR_alarm] = {"alarm", (void *)nullptr},
	[__NR_setitimer] = {"setitimer", (void *)nullptr},
	[__NR_getpid] = {"getpid", (void *)nullptr},
	[__NR_sendfile] = {"sendfile", (void *)nullptr},
	[__NR_socket] = {"socket", (void *)nullptr},
	[__NR_connect] = {"connect", (void *)nullptr},
	[__NR_accept] = {"accept", (void *)nullptr},
	[__NR_sendto] = {"sendto", (void *)nullptr},
	[__NR_recvfrom] = {"recvfrom", (void *)nullptr},
	[__NR_sendmsg] = {"sendmsg", (void *)nullptr},
	[__NR_recvmsg] = {"recvmsg", (void *)nullptr},
	[__NR_shutdown] = {"shutdown", (void *)nullptr},
	[__NR_bind] = {"bind", (void *)nullptr},
	[__NR_listen] = {"listen", (void *)nullptr},
	[__NR_getsockname] = {"getsockname", (void *)nullptr},
	[__NR_getpeername] = {"getpeername", (void *)nullptr},
	[__NR_socketpair] = {"socketpair", (void *)nullptr},
	[__NR_setsockopt] = {"setsockopt", (void *)nullptr},
	[__NR_getsockopt] = {"getsockopt", (void *)nullptr},
	[__NR_clone] = {"clone", (void *)nullptr},
	[__NR_fork] = {"fork", (void *)nullptr},
	[__NR_vfork] = {"vfork", (void *)nullptr},
	[__NR_execve] = {"execve", (void *)nullptr},
	[__NR_exit] = {"exit", (void *)sys_exit},
	[__NR_wait4] = {"wait4", (void *)nullptr},
	[__NR_kill] = {"kill", (void *)nullptr},
	[__NR_uname] = {"uname", (void *)nullptr},
	[__NR_semget] = {"semget", (void *)nullptr},
	[__NR_semop] = {"semop", (void *)nullptr},
	[__NR_semctl] = {"semctl", (void *)nullptr},
	[__NR_shmdt] = {"shmdt", (void *)nullptr},
	[__NR_msgget] = {"msgget", (void *)nullptr},
	[__NR_msgsnd] = {"msgsnd", (void *)nullptr},
	[__NR_msgrcv] = {"msgrcv", (void *)nullptr},
	[__NR_msgctl] = {"msgctl", (void *)nullptr},
	[__NR_fcntl] = {"fcntl", (void *)nullptr},
	[__NR_flock] = {"flock", (void *)nullptr},
	[__NR_fsync] = {"fsync", (void *)nullptr},
	[__NR_fdatasync] = {"fdatasync", (void *)nullptr},
	[__NR_truncate] = {"truncate", (void *)nullptr},
	[__NR_ftruncate] = {"ftruncate", (void *)nullptr},
	[__NR_getdents] = {"getdents", (void *)nullptr},
	[__NR_getcwd] = {"getcwd", (void *)nullptr},
	[__NR_chdir] = {"chdir", (void *)nullptr},
	[__NR_fchdir] = {"fchdir", (void *)nullptr},
	[__NR_rename] = {"rename", (void *)nullptr},
	[__NR_mkdir] = {"mkdir", (void *)nullptr},
	[__NR_rmdir] = {"rmdir", (void *)nullptr},
	[__NR_creat] = {"creat", (void *)sys_creat},
	[__NR_link] = {"link", (void *)nullptr},
	[__NR_unlink] = {"unlink", (void *)nullptr},
	[__NR_symlink] = {"symlink", (void *)nullptr},
	[__NR_readlink] = {"readlink", (void *)nullptr},
	[__NR_chmod] = {"chmod", (void *)nullptr},
	[__NR_fchmod] = {"fchmod", (void *)nullptr},
	[__NR_chown] = {"chown", (void *)nullptr},
	[__NR_fchown] = {"fchown", (void *)nullptr},
	[__NR_lchown] = {"lchown", (void *)nullptr},
	[__NR_umask] = {"umask", (void *)nullptr},
	[__NR_gettimeofday] = {"gettimeofday", (void *)nullptr},
	[__NR_getrlimit] = {"getrlimit", (void *)nullptr},
	[__NR_getrusage] = {"getrusage", (void *)nullptr},
	[__NR_sysinfo] = {"sysinfo", (void *)nullptr},
	[__NR_times] = {"times", (void *)nullptr},
	[__NR_ptrace] = {"ptrace", (void *)nullptr},
	[__NR_getuid] = {"getuid", (void *)nullptr},
	[__NR_syslog] = {"syslog", (void *)nullptr},
	[__NR_getgid] = {"getgid", (void *)nullptr},
	[__NR_setuid] = {"setuid", (void *)nullptr},
	[__NR_setgid] = {"setgid", (void *)nullptr},
	[__NR_geteuid] = {"geteuid", (void *)nullptr},
	[__NR_getegid] = {"getegid", (void *)nullptr},
	[__NR_setpgid] = {"setpgid", (void *)nullptr},
	[__NR_getppid] = {"getppid", (void *)nullptr},
	[__NR_getpgrp] = {"getpgrp", (void *)nullptr},
	[__NR_setsid] = {"setsid", (void *)nullptr},
	[__NR_setreuid] = {"setreuid", (void *)nullptr},
	[__NR_setregid] = {"setregid", (void *)nullptr},
	[__NR_getgroups] = {"getgroups", (void *)nullptr},
	[__NR_setgroups] = {"setgroups", (void *)nullptr},
	[__NR_setresuid] = {"setresuid", (void *)nullptr},
	[__NR_getresuid] = {"getresuid", (void *)nullptr},
	[__NR_setresgid] = {"setresgid", (void *)nullptr},
	[__NR_getresgid] = {"getresgid", (void *)nullptr},
	[__NR_getpgid] = {"getpgid", (void *)nullptr},
	[__NR_setfsuid] = {"setfsuid", (void *)nullptr},
	[__NR_setfsgid] = {"setfsgid", (void *)nullptr},
	[__NR_getsid] = {"getsid", (void *)nullptr},
	[__NR_capget] = {"capget", (void *)nullptr},
	[__NR_capset] = {"capset", (void *)nullptr},
	[__NR_rt_sigpending] = {"rt_sigpending", (void *)nullptr},
	[__NR_rt_sigtimedwait] = {"rt_sigtimedwait", (void *)nullptr},
	[__NR_rt_sigqueueinfo] = {"rt_sigqueueinfo", (void *)nullptr},
	[__NR_rt_sigsuspend] = {"rt_sigsuspend", (void *)nullptr},
	[__NR_sigaltstack] = {"sigaltstack", (void *)nullptr},
	[__NR_utime] = {"utime", (void *)nullptr},
	[__NR_mknod] = {"mknod", (void *)nullptr},
	[__NR_uselib] = {"uselib", (void *)nullptr},
	[__NR_personality] = {"personality", (void *)nullptr},
	[__NR_ustat] = {"ustat", (void *)nullptr},
	[__NR_statfs] = {"statfs", (void *)nullptr},
	[__NR_fstatfs] = {"fstatfs", (void *)nullptr},
	[__NR_sysfs] = {"sysfs", (void *)nullptr},
	[__NR_getpriority] = {"getpriority", (void *)nullptr},
	[__NR_setpriority] = {"setpriority", (void *)nullptr},
	[__NR_sched_setparam] = {"sched_setparam", (void *)nullptr},
	[__NR_sched_getparam] = {"sched_getparam", (void *)nullptr},
	[__NR_sched_setscheduler] = {"sched_setscheduler", (void *)nullptr},
	[__NR_sched_getscheduler] = {"sched_getscheduler", (void *)nullptr},
	[__NR_sched_get_priority_max] = {"sched_get_priority_max", (void *)nullptr},
	[__NR_sched_get_priority_min] = {"sched_get_priority_min", (void *)nullptr},
	[__NR_sched_rr_get_interval] = {"sched_rr_get_interval", (void *)nullptr},
	[__NR_mlock] = {"mlock", (void *)nullptr},
	[__NR_munlock] = {"munlock", (void *)nullptr},
	[__NR_mlockall] = {"mlockall", (void *)nullptr},
	[__NR_munlockall] = {"munlockall", (void *)nullptr},
	[__NR_vhangup] = {"vhangup", (void *)nullptr},
	[__NR_modify_ldt] = {"modify_ldt", (void *)nullptr},
	[__NR_pivot_root] = {"pivot_root", (void *)nullptr},
	[__NR__sysctl] = {"_sysctl", (void *)nullptr},
	[__NR_prctl] = {"prctl", (void *)nullptr},
	[__NR_arch_prctl] = {"arch_prctl", (void *)sys_arch_prctl},
	[__NR_adjtimex] = {"adjtimex", (void *)nullptr},
	[__NR_setrlimit] = {"setrlimit", (void *)nullptr},
	[__NR_chroot] = {"chroot", (void *)nullptr},
	[__NR_sync] = {"sync", (void *)nullptr},
	[__NR_acct] = {"acct", (void *)nullptr},
	[__NR_settimeofday] = {"settimeofday", (void *)nullptr},
	[__NR_mount] = {"mount", (void *)nullptr},
	[__NR_umount2] = {"umount2", (void *)nullptr},
	[__NR_swapon] = {"swapon", (void *)nullptr},
	[__NR_swapoff] = {"swapoff", (void *)nullptr},
	[__NR_reboot] = {"reboot", (void *)nullptr},
	[__NR_sethostname] = {"sethostname", (void *)nullptr},
	[__NR_setdomainname] = {"setdomainname", (void *)nullptr},
	[__NR_iopl] = {"iopl", (void *)nullptr},
	[__NR_ioperm] = {"ioperm", (void *)nullptr},
	[__NR_create_module] = {"create_module", (void *)nullptr},
	[__NR_init_module] = {"init_module", (void *)nullptr},
	[__NR_delete_module] = {"delete_module", (void *)nullptr},
	[__NR_get_kernel_syms] = {"get_kernel_syms", (void *)nullptr},
	[__NR_query_module] = {"query_module", (void *)nullptr},
	[__NR_quotactl] = {"quotactl", (void *)nullptr},
	[__NR_nfsservctl] = {"nfsservctl", (void *)nullptr},
	[__NR_getpmsg] = {"getpmsg", (void *)nullptr},
	[__NR_putpmsg] = {"putpmsg", (void *)nullptr},
	[__NR_afs_syscall] = {"afs_syscall", (void *)nullptr},
	[__NR_tuxcall] = {"tuxcall", (void *)nullptr},
	[__NR_security] = {"security", (void *)nullptr},
	[__NR_gettid] = {"gettid", (void *)nullptr},
	[__NR_readahead] = {"readahead", (void *)nullptr},
	[__NR_setxattr] = {"setxattr", (void *)nullptr},
	[__NR_lsetxattr] = {"lsetxattr", (void *)nullptr},
	[__NR_fsetxattr] = {"fsetxattr", (void *)nullptr},
	[__NR_getxattr] = {"getxattr", (void *)nullptr},
	[__NR_lgetxattr] = {"lgetxattr", (void *)nullptr},
	[__NR_fgetxattr] = {"fgetxattr", (void *)nullptr},
	[__NR_listxattr] = {"listxattr", (void *)nullptr},
	[__NR_llistxattr] = {"llistxattr", (void *)nullptr},
	[__NR_flistxattr] = {"flistxattr", (void *)nullptr},
	[__NR_removexattr] = {"removexattr", (void *)nullptr},
	[__NR_lremovexattr] = {"lremovexattr", (void *)nullptr},
	[__NR_fremovexattr] = {"fremovexattr", (void *)nullptr},
	[__NR_tkill] = {"tkill", (void *)nullptr},
	[__NR_time] = {"time", (void *)nullptr},
	[__NR_futex] = {"futex", (void *)nullptr},
	[__NR_sched_setaffinity] = {"sched_setaffinity", (void *)nullptr},
	[__NR_sched_getaffinity] = {"sched_getaffinity", (void *)nullptr},
	[__NR_set_thread_area] = {"set_thread_area", (void *)nullptr},
	[__NR_io_setup] = {"io_setup", (void *)nullptr},
	[__NR_io_destroy] = {"io_destroy", (void *)nullptr},
	[__NR_io_getevents] = {"io_getevents", (void *)nullptr},
	[__NR_io_submit] = {"io_submit", (void *)nullptr},
	[__NR_io_cancel] = {"io_cancel", (void *)nullptr},
	[__NR_get_thread_area] = {"get_thread_area", (void *)nullptr},
	[__NR_lookup_dcookie] = {"lookup_dcookie", (void *)nullptr},
	[__NR_epoll_create] = {"epoll_create", (void *)nullptr},
	[__NR_epoll_ctl_old] = {"epoll_ctl_old", (void *)nullptr},
	[__NR_epoll_wait_old] = {"epoll_wait_old", (void *)nullptr},
	[__NR_remap_file_pages] = {"remap_file_pages", (void *)nullptr},
	[__NR_getdents64] = {"getdents64", (void *)nullptr},
	[__NR_set_tid_address] = {"set_tid_address", (void *)sys_set_tid_address},
	[__NR_restart_syscall] = {"restart_syscall", (void *)nullptr},
	[__NR_semtimedop] = {"semtimedop", (void *)nullptr},
	[__NR_fadvise64] = {"fadvise64", (void *)nullptr},
	[__NR_timer_create] = {"timer_create", (void *)nullptr},
	[__NR_timer_settime] = {"timer_settime", (void *)nullptr},
	[__NR_timer_gettime] = {"timer_gettime", (void *)nullptr},
	[__NR_timer_getoverrun] = {"timer_getoverrun", (void *)nullptr},
	[__NR_timer_delete] = {"timer_delete", (void *)nullptr},
	[__NR_clock_settime] = {"clock_settime", (void *)nullptr},
	[__NR_clock_gettime] = {"clock_gettime", (void *)nullptr},
	[__NR_clock_getres] = {"clock_getres", (void *)nullptr},
	[__NR_clock_nanosleep] = {"clock_nanosleep", (void *)nullptr},
	[__NR_exit_group] = {"exit_group", (void *)sys_exit_group},
	[__NR_epoll_wait] = {"epoll_wait", (void *)nullptr},
	[__NR_epoll_ctl] = {"epoll_ctl", (void *)nullptr},
	[__NR_tgkill] = {"tgkill", (void *)nullptr},
	[__NR_utimes] = {"utimes", (void *)nullptr},
	[__NR_vserver] = {"vserver", (void *)nullptr},
	[__NR_mbind] = {"mbind", (void *)nullptr},
	[__NR_set_mempolicy] = {"set_mempolicy", (void *)nullptr},
	[__NR_get_mempolicy] = {"get_mempolicy", (void *)nullptr},
	[__NR_mq_open] = {"mq_open", (void *)nullptr},
	[__NR_mq_unlink] = {"mq_unlink", (void *)nullptr},
	[__NR_mq_timedsend] = {"mq_timedsend", (void *)nullptr},
	[__NR_mq_timedreceive] = {"mq_timedreceive", (void *)nullptr},
	[__NR_mq_notify] = {"mq_notify", (void *)nullptr},
	[__NR_mq_getsetattr] = {"mq_getsetattr", (void *)nullptr},
	[__NR_kexec_load] = {"kexec_load", (void *)nullptr},
	[__NR_waitid] = {"waitid", (void *)nullptr},
	[__NR_add_key] = {"add_key", (void *)nullptr},
	[__NR_request_key] = {"request_key", (void *)nullptr},
	[__NR_keyctl] = {"keyctl", (void *)nullptr},
	[__NR_ioprio_set] = {"ioprio_set", (void *)nullptr},
	[__NR_ioprio_get] = {"ioprio_get", (void *)nullptr},
	[__NR_inotify_init] = {"inotify_init", (void *)nullptr},
	[__NR_inotify_add_watch] = {"inotify_add_watch", (void *)nullptr},
	[__NR_inotify_rm_watch] = {"inotify_rm_watch", (void *)nullptr},
	[__NR_migrate_pages] = {"migrate_pages", (void *)nullptr},
	[__NR_openat] = {"openat", (void *)nullptr},
	[__NR_mkdirat] = {"mkdirat", (void *)nullptr},
	[__NR_mknodat] = {"mknodat", (void *)nullptr},
	[__NR_fchownat] = {"fchownat", (void *)nullptr},
	[__NR_futimesat] = {"futimesat", (void *)nullptr},
	[__NR_newfstatat] = {"newfstatat", (void *)nullptr},
	[__NR_unlinkat] = {"unlinkat", (void *)nullptr},
	[__NR_renameat] = {"renameat", (void *)nullptr},
	[__NR_linkat] = {"linkat", (void *)nullptr},
	[__NR_symlinkat] = {"symlinkat", (void *)nullptr},
	[__NR_readlinkat] = {"readlinkat", (void *)nullptr},
	[__NR_fchmodat] = {"fchmodat", (void *)nullptr},
	[__NR_faccessat] = {"faccessat", (void *)nullptr},
	[__NR_pselect6] = {"pselect6", (void *)nullptr},
	[__NR_ppoll] = {"ppoll", (void *)nullptr},
	[__NR_unshare] = {"unshare", (void *)nullptr},
	[__NR_set_robust_list] = {"set_robust_list", (void *)nullptr},
	[__NR_get_robust_list] = {"get_robust_list", (void *)nullptr},
	[__NR_splice] = {"splice", (void *)nullptr},
	[__NR_tee] = {"tee", (void *)nullptr},
	[__NR_sync_file_range] = {"sync_file_range", (void *)nullptr},
	[__NR_vmsplice] = {"vmsplice", (void *)nullptr},
	[__NR_move_pages] = {"move_pages", (void *)nullptr},
	[__NR_utimensat] = {"utimensat", (void *)nullptr},
	[__NR_epoll_pwait] = {"epoll_pwait", (void *)nullptr},
	[__NR_signalfd] = {"signalfd", (void *)nullptr},
	[__NR_timerfd_create] = {"timerfd_create", (void *)nullptr},
	[__NR_eventfd] = {"eventfd", (void *)nullptr},
	[__NR_fallocate] = {"fallocate", (void *)nullptr},
	[__NR_timerfd_settime] = {"timerfd_settime", (void *)nullptr},
	[__NR_timerfd_gettime] = {"timerfd_gettime", (void *)nullptr},
	[__NR_accept4] = {"accept4", (void *)nullptr},
	[__NR_signalfd4] = {"signalfd4", (void *)nullptr},
	[__NR_eventfd2] = {"eventfd2", (void *)nullptr},
	[__NR_epoll_create1] = {"epoll_create1", (void *)nullptr},
	[__NR_dup3] = {"dup3", (void *)nullptr},
	[__NR_pipe2] = {"pipe2", (void *)nullptr},
	[__NR_inotify_init1] = {"inotify_init1", (void *)nullptr},
	[__NR_preadv] = {"preadv", (void *)nullptr},
	[__NR_pwritev] = {"pwritev", (void *)nullptr},
	[__NR_rt_tgsigqueueinfo] = {"rt_tgsigqueueinfo", (void *)nullptr},
	[__NR_perf_event_open] = {"perf_event_open", (void *)nullptr},
	[__NR_recvmmsg] = {"recvmmsg", (void *)nullptr},
	[__NR_fanotify_init] = {"fanotify_init", (void *)nullptr},
	[__NR_fanotify_mark] = {"fanotify_mark", (void *)nullptr},
	[__NR_prlimit64] = {"prlimit64", (void *)nullptr},
	[__NR_name_to_handle_at] = {"name_to_handle_at", (void *)nullptr},
	[__NR_open_by_handle_at] = {"open_by_handle_at", (void *)nullptr},
	[__NR_clock_adjtime] = {"clock_adjtime", (void *)nullptr},
	[__NR_syncfs] = {"syncfs", (void *)nullptr},
	[__NR_sendmmsg] = {"sendmmsg", (void *)nullptr},
	[__NR_setns] = {"setns", (void *)nullptr},
	[__NR_getcpu] = {"getcpu", (void *)nullptr},
	[__NR_process_vm_readv] = {"process_vm_readv", (void *)nullptr},
	[__NR_process_vm_writev] = {"process_vm_writev", (void *)nullptr},
	[__NR_kcmp] = {"kcmp", (void *)nullptr},
	[__NR_finit_module] = {"finit_module", (void *)nullptr},
	[__NR_sched_setattr] = {"sched_setattr", (void *)nullptr},
	[__NR_sched_getattr] = {"sched_getattr", (void *)nullptr},
	[__NR_renameat2] = {"renameat2", (void *)nullptr},
	[__NR_seccomp] = {"seccomp", (void *)nullptr},
	[__NR_getrandom] = {"getrandom", (void *)nullptr},
	[__NR_memfd_create] = {"memfd_create", (void *)nullptr},
	[__NR_kexec_file_load] = {"kexec_file_load", (void *)nullptr},
	[__NR_bpf] = {"bpf", (void *)nullptr},
	[__NR_execveat] = {"execveat", (void *)nullptr},
	[__NR_userfaultfd] = {"userfaultfd", (void *)nullptr},
	[__NR_membarrier] = {"membarrier", (void *)nullptr},
	[__NR_mlock2] = {"mlock2", (void *)nullptr},
	[__NR_copy_file_range] = {"copy_file_range", (void *)nullptr},
	[__NR_preadv2] = {"preadv2", (void *)nullptr},
	[__NR_pwritev2] = {"pwritev2", (void *)nullptr},
	[__NR_pkey_mprotect] = {"pkey_mprotect", (void *)nullptr},
	[__NR_pkey_alloc] = {"pkey_alloc", (void *)nullptr},
	[__NR_pkey_free] = {"pkey_free", (void *)nullptr},
	[__NR_statx] = {"statx", (void *)nullptr},
	[__NR_io_pgetevents] = {"io_pgetevents", (void *)nullptr},
	[__NR_rseq] = {"rseq", (void *)nullptr},
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
	[__NR_pidfd_send_signal] = {"pidfd_send_signal", (void *)nullptr},
	[__NR_io_uring_setup] = {"io_uring_setup", (void *)nullptr},
	[__NR_io_uring_enter] = {"io_uring_enter", (void *)nullptr},
	[__NR_io_uring_register] = {"io_uring_register", (void *)nullptr},
	[__NR_open_tree] = {"open_tree", (void *)nullptr},
	[__NR_move_mount] = {"move_mount", (void *)nullptr},
	[__NR_fsopen] = {"fsopen", (void *)nullptr},
	[__NR_fsconfig] = {"fsconfig", (void *)nullptr},
	[__NR_fsmount] = {"fsmount", (void *)nullptr},
	[__NR_fspick] = {"fspick", (void *)nullptr},
	[__NR_pidfd_open] = {"pidfd_open", (void *)nullptr},
	[__NR_clone3] = {"clone3", (void *)nullptr},
	[__NR_close_range] = {"close_range", (void *)nullptr},
	[__NR_openat2] = {"openat2", (void *)nullptr},
	[__NR_pidfd_getfd] = {"pidfd_getfd", (void *)nullptr},
	[__NR_faccessat2] = {"faccessat2", (void *)nullptr},
	[__NR_process_madvise] = {"process_madvise", (void *)nullptr},
	[__NR_epoll_pwait2] = {"epoll_pwait2", (void *)nullptr},
	[__NR_mount_setattr] = {"mount_setattr", (void *)nullptr},
	[443] = {"reserved", (void *)nullptr},
	[__NR_landlock_create_ruleset] = {"landlock_create_ruleset", (void *)nullptr},
	[__NR_landlock_add_rule] = {"landlock_add_rule", (void *)nullptr},
	[__NR_landlock_restrict_self] = {"landlock_restrict_self", (void *)nullptr},
};

uintptr_t HandleLinuxSyscalls(SyscallsFrame *Frame)
{
#if defined(a64)
	if (Frame->rax > sizeof(LinuxSyscallsTable) / sizeof(SyscallData))
	{
		fixme("Syscall %d not implemented",
			  Frame->rax);
		return -ENOSYS;
	}

	SyscallData Syscall = LinuxSyscallsTable[Frame->rax];

	long (*call)(long, ...) = r_cst(long (*)(long, ...),
									Syscall.Handler);

	if (unlikely(!call))
	{
		fixme("Syscall %s(%d) not implemented.",
			  Syscall.Name, Frame->rax);
		return -ENOSYS;
	}

	debug("[%d:\"%s\"]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax, Syscall.Name,
		  Frame->rdi, Frame->rsi, Frame->rdx,
		  Frame->r10, Frame->r8, Frame->r9);

	return call(Frame->rdi, Frame->rsi, Frame->rdx,
				Frame->r10, Frame->r8, Frame->r9);
#elif defined(a32)
	return 0;
#elif defined(aa64)
	return 0;
#endif
}
