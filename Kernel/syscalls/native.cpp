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

struct SyscallData
{
	const char *Name;
	void *Handler;
};

using Tasking::PCB;
using Tasking::TCB;

static int sys_api_version(SysFrm *Frame, int version) { return 0; }
static int sys_dummy(SysFrm *Frame) { return 0; }

static ssize_t sys_read(SysFrm *Frame, int fildes, void *buf, size_t nbyte)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	void *pBuf = vma->UserCheckAndGetAddress(buf, nbyte);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t ret = fdt->usr_read(fildes, pBuf, nbyte);
	if (ret >= 0)
		fdt->usr_lseek(fildes, ret, SEEK_CUR);
	return ret;
}

static ssize_t sys_pread(SysFrm *Frame, int fildes, void *buf, size_t nbyte, off_t offset)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	void *pBuf = vma->UserCheckAndGetAddress(buf, nbyte);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->usr_pread(fildes, pBuf, nbyte, offset);
}

static ssize_t sys_write(SysFrm *Frame, int fildes, const void *buf, size_t nbyte)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const void *pBuf = vma->UserCheckAndGetAddress(buf, nbyte);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	ssize_t ret = fdt->usr_write(fildes, pBuf, nbyte);
	if (ret)
		fdt->usr_lseek(fildes, ret, SEEK_CUR);
	return ret;
}

static ssize_t sys_pwrite(SysFrm *Frame, int fildes, const void *buf, size_t nbyte, off_t offset)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const void *pBuf = vma->UserCheckAndGetAddress(buf, nbyte);
	if (pBuf == nullptr)
		return -EFAULT;

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->usr_pwrite(fildes, pBuf, nbyte, offset);
}

static int sys_open(SysFrm *Frame, const char *pathname, int flags, mode_t mode)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	const char *pPathname = vma->UserCheckAndGetAddress(pathname, PAGE_SIZE);
	if (pPathname == nullptr)
		return -EFAULT;

	debug("%s, %d, %d", pPathname, flags, mode);

	if (flags & 0200000 /* O_DIRECTORY */)
	{
		FileNode *node = fs->GetByPath(pPathname, pcb->CWD);
		if (node == nullptr)
		{
			debug("Couldn't find %s", pPathname);
			return -ENOENT;
		}

		if (!node->IsDirectory())
		{
			debug("%s is not a directory", pPathname);
			return -ENOTDIR;
		}
	}

	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->usr_open(pPathname, flags, mode);
}

static int sys_close(SysFrm *Frame, int fd)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return fdt->usr_close(fd);
}

static int sys_ioctl(SysFrm *Frame, int fd, unsigned long request, void *argp) { return -ENOSYS; }
static int sys_stat(SysFrm *Frame, const char *pathname, struct stat *statbuf) { return -ENOSYS; }
static int sys_fstat(SysFrm *Frame, int fd, struct stat *statbuf) { return -ENOSYS; }
static int sys_lstat(SysFrm *Frame, const char *pathname, struct stat *statbuf) { return -ENOSYS; }
static int sys_access(SysFrm *Frame, const char *pathname, int mode)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	auto pPathname = vma->UserCheckAndGetAddress(pathname);
	if (pPathname == nullptr)
		return -EFAULT;

	debug("access(%s, %d)", (char *)pPathname, mode);

	if (!fs->PathExists(pPathname, pcb->CWD))
		return -ENOENT;

	stub;
	return 0;
}

static int sys_truncate(SysFrm *Frame, const char *pathname, off_t length) { return -ENOSYS; }
static int sys_ftruncate(SysFrm *Frame, int fd, off_t length) { return -ENOSYS; }

static int sys_tell(SysFrm *Frame, int fd)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;

	return fdt->usr_lseek(fd, 0, SEEK_CUR);
}

static off_t sys_seek(SysFrm *Frame, int fd, off_t offset, int whence)
{
	PCB *pcb = thisProcess;
	vfs::FileDescriptorTable *fdt = pcb->FileDescriptors;

	return fdt->usr_lseek(fd, offset, whence);
}

__noreturn void sys_exit(SysFrm *Frame, int status);
pid_t sys_fork(SysFrm *Frame);
int sys_execve(SysFrm *Frame, const char *pathname, char *const argv[], char *const envp[]);
pid_t sys_getpid(SysFrm *Frame);
pid_t sys_getppid(SysFrm *Frame);
pid_t sys_waitpid(pid_t pid, int *wstatus, int options);
int sys_kill(SysFrm *Frame, pid_t pid, int sig);
int sys_prctl(SysFrm *Frame, prctl_options_t option, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);

int sys_brk(SysFrm *Frame, void *end_data);
void *sys_mmap(SysFrm *Frame, void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int sys_munmap(SysFrm *Frame, void *addr, size_t length);
int sys_mprotect(SysFrm *Frame, void *addr, size_t length, int prot);
int sys_madvise(SysFrm *Frame, void *addr, size_t length, int advice);

static int sys_pipe(SysFrm *Frame, int pipefd[2]) { return -ENOSYS; }
static int sys_dup(SysFrm *Frame, int oldfd) { return -ENOSYS; }
static int sys_dup2(SysFrm *Frame, int oldfd, int newfd) { return -ENOSYS; }
static int sys_socket(SysFrm *Frame, int domain, int type, int protocol) { return -ENOSYS; }
static int sys_bind(SysFrm *Frame, int sockfd, const struct sockaddr *addr, __SYS_socklen_t addrlen) { return -ENOSYS; }
static int sys_connect(SysFrm *Frame, int sockfd, const struct sockaddr *addr, __SYS_socklen_t addrlen) { return -ENOSYS; }
static int sys_listen(SysFrm *Frame, int sockfd, int backlog) { return -ENOSYS; }
static int sys_accept(SysFrm *Frame, int sockfd, struct sockaddr *addr, __SYS_socklen_t *addrlen) { return -ENOSYS; }
static ssize_t sys_send(SysFrm *Frame, int sockfd, const void *buf, size_t len, int flags) { return -ENOSYS; }
static ssize_t sys_recv(SysFrm *Frame, int sockfd, void *buf, size_t len, int flags) { return -ENOSYS; }
static int sys_shutdown(SysFrm *Frame, int sockfd, int how) { return -ENOSYS; }
static time_t sys_time(SysFrm *Frame, time_t *t) { return -ENOSYS; }
static int sys_clock_gettime(SysFrm *Frame, __SYS_clockid_t clockid, struct timespec *tp) { return -ENOSYS; }
static int sys_clock_settime(SysFrm *Frame, __SYS_clockid_t clockid, const struct timespec *tp) { return -ENOSYS; }
static int sys_nanosleep(SysFrm *Frame, const struct timespec *req, struct timespec *rem) { return -ENOSYS; }
static char *sys_getcwd(SysFrm *Frame, char *buf, size_t size) { return (char *)-ENOSYS; }
static int sys_chdir(SysFrm *Frame, const char *path) { return -ENOSYS; }
static int sys_mkdir(SysFrm *Frame, const char *path, mode_t mode) { return -ENOSYS; }
static int sys_rmdir(SysFrm *Frame, const char *path) { return -ENOSYS; }
static int sys_unlink(SysFrm *Frame, const char *pathname) { return -ENOSYS; }
static int sys_rename(SysFrm *Frame, const char *oldpath, const char *newpath) { return -ENOSYS; }

static SyscallData scTbl[SYS_MAX] = {};
__constructor void __init_native_syscalls(void)
{
	/* Initialization */
	scTbl[SYS_API_VERSION] = {"SYS_API_VERSION", (void *)sys_api_version};
	scTbl[1] = {"dummy", (void *)sys_dummy};

	/* I/O */
	scTbl[SYS_READ] = {"SYS_READ", (void *)sys_read};
	scTbl[SYS_PREAD] = {"SYS_PREAD", (void *)sys_pread};
	scTbl[SYS_WRITE] = {"SYS_WRITE", (void *)sys_write};
	scTbl[SYS_PWRITE] = {"SYS_PWRITE", (void *)sys_pwrite};
	scTbl[SYS_OPEN] = {"SYS_OPEN", (void *)sys_open};
	scTbl[SYS_CLOSE] = {"SYS_CLOSE", (void *)sys_close};
	scTbl[SYS_IOCTL] = {"SYS_IOCTL", (void *)sys_ioctl};

	/* File Status */
	scTbl[SYS_STAT] = {"SYS_STAT", (void *)sys_stat};
	scTbl[SYS_FSTAT] = {"SYS_FSTAT", (void *)sys_fstat};
	scTbl[SYS_LSTAT] = {"SYS_LSTAT", (void *)sys_lstat};
	scTbl[SYS_ACCESS] = {"SYS_ACCESS", (void *)sys_access};
	scTbl[SYS_TRUNCATE] = {"SYS_TRUNCATE", (void *)sys_truncate};
	scTbl[SYS_FTRUNCATE] = {"SYS_FTRUNCATE", (void *)sys_ftruncate};
	scTbl[SYS_TELL] = {"SYS_TELL", (void *)sys_tell};
	scTbl[SYS_SEEK] = {"SYS_SEEK", (void *)sys_seek};

	/* Process Control */
	scTbl[SYS_EXIT] = {"SYS_EXIT", (void *)sys_exit};
	scTbl[SYS_FORK] = {"SYS_FORK", (void *)sys_fork};
	scTbl[SYS_EXECVE] = {"SYS_EXECVE", (void *)sys_execve};
	scTbl[SYS_GETPID] = {"SYS_GETPID", (void *)sys_getpid};
	scTbl[SYS_GETPPID] = {"SYS_GETPPID", (void *)sys_getppid};
	scTbl[SYS_WAITPID] = {"SYS_WAITPID", (void *)sys_waitpid};
	scTbl[SYS_KILL] = {"SYS_KILL", (void *)sys_kill};
	scTbl[SYS_PRCTL] = {"SYS_PRCTL", (void *)sys_prctl};

	/* Memory */
	scTbl[SYS_BRK] = {"SYS_BRK", (void *)sys_brk};
	scTbl[SYS_MMAP] = {"SYS_MMAP", (void *)sys_mmap};
	scTbl[SYS_MUNMAP] = {"SYS_MUNMAP", (void *)sys_munmap};
	scTbl[SYS_MPROTECT] = {"SYS_MPROTECT", (void *)sys_mprotect};
	scTbl[SYS_MADVISE] = {"SYS_MADVISE", (void *)sys_madvise};

	/* Communication */
	scTbl[SYS_PIPE] = {"SYS_PIPE", (void *)sys_pipe};
	scTbl[SYS_DUP] = {"SYS_DUP", (void *)sys_dup};
	scTbl[SYS_DUP2] = {"SYS_DUP2", (void *)sys_dup2};
	scTbl[SYS_SOCKET] = {"SYS_SOCKET", (void *)sys_socket};
	scTbl[SYS_BIND] = {"SYS_BIND", (void *)sys_bind};
	scTbl[SYS_CONNECT] = {"SYS_CONNECT", (void *)sys_connect};
	scTbl[SYS_LISTEN] = {"SYS_LISTEN", (void *)sys_listen};
	scTbl[SYS_ACCEPT] = {"SYS_ACCEPT", (void *)sys_accept};
	scTbl[SYS_SEND] = {"SYS_SEND", (void *)sys_send};
	scTbl[SYS_RECV] = {"SYS_RECV", (void *)sys_recv};
	scTbl[SYS_SHUTDOWN] = {"SYS_SHUTDOWN", (void *)sys_shutdown};

	/* Time */
	scTbl[SYS_TIME] = {"SYS_TIME", (void *)sys_time};
	scTbl[SYS_CLOCK_GETTIME] = {"SYS_CLOCK_GETTIME", (void *)sys_clock_gettime};
	scTbl[SYS_CLOCK_SETTIME] = {"SYS_CLOCK_SETTIME", (void *)sys_clock_settime};
	scTbl[SYS_NANOSLEEP] = {"SYS_NANOSLEEP", (void *)sys_nanosleep};

	/* Miscellaneous */
	scTbl[SYS_GETCWD] = {"SYS_GETCWD", (void *)sys_getcwd};
	scTbl[SYS_CHDIR] = {"SYS_CHDIR", (void *)sys_chdir};
	scTbl[SYS_MKDIR] = {"SYS_MKDIR", (void *)sys_mkdir};
	scTbl[SYS_RMDIR] = {"SYS_RMDIR", (void *)sys_rmdir};
	scTbl[SYS_UNLINK] = {"SYS_UNLINK", (void *)sys_unlink};
	scTbl[SYS_RENAME] = {"SYS_RENAME", (void *)sys_rename};
}

uintptr_t HandleNativeSyscalls(SysFrm *Frame)
{
	if (unlikely(Frame->ReturnValue() > SYS_MAX))
	{
		fixme("Syscall %ld not implemented.", Frame->ReturnValue());
		return -ENOSYS;
	}

	SyscallData sc = scTbl[Frame->ReturnValue()];
	uintptr_t (*call)(SysFrm *, uintptr_t, ...) = r_cst(uintptr_t (*)(SysFrm *, uintptr_t, ...), sc.Handler);

	if (unlikely(!call))
	{
		error("Syscall %s(%d) not implemented.",
			  sc.Name, Frame->ReturnValue());
		return -ENOSYS;
	}

	uintptr_t arg0 = Frame->Arg0();
	uintptr_t arg1 = Frame->Arg1();
	uintptr_t arg2 = Frame->Arg2();
	uintptr_t arg3 = Frame->Arg3();
	uintptr_t arg4 = Frame->Arg4();
	uintptr_t arg5 = Frame->Arg5();

	debug("> [%d:\"%s\"]( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )", Frame->ReturnValue(), sc.Name, arg0, arg1, arg2, arg3, arg4, arg5);
	uintptr_t result = call(Frame, arg0, arg1, arg2, arg3, arg4, arg5);
	debug("< [%d:\"%s\"] = %ld", Frame->ReturnValue(), sc.Name, result);
	return result;
}
