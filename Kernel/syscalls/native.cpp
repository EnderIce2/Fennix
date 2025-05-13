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
static int sys_debug_report(SysFrm *Frame) { return 0; }

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
		Node node = fs->Lookup(pcb->CWD, pPathname);
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
static int sys_fcntl(SysFrm *Frame, int fd, int cmd, void *arg) { return -ENOSYS; }
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

	if (!fs->Lookup(pcb->CWD, pPathname))
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

static int sys_uname(SysFrm *Frame, struct kutsname *buf)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	struct kutsname *pBuf = vma->UserCheckAndGetAddress(buf, sizeof(struct kutsname));
	if (pBuf == nullptr)
		return -EFAULT;

	strncpy(pBuf->sysname, KERNEL_NAME, sizeof(pBuf->sysname));

	char release[sizeof(pBuf->release)];
	sprintf(release, "%s", KERNEL_VERSION);
	strncpy(pBuf->release, release, sizeof(pBuf->release));

	char version[sizeof(pBuf->version)];

	bool isDebug = false;
#ifdef DEBUG
	isDebug = true;
#endif

	sprintf(version, "FNX-v%s-%s %s %s %s %s",
			KERNEL_VERSION, GIT_COMMIT_SHORT,
			isDebug ? "DEBUG" : "RELEASE",
			__DATE__, __TIME__, __VERSION__);
	strncpy(pBuf->version, version, sizeof(pBuf->version));

#if defined(__amd64__)
	const char *osarch = "x86_64";
#elif defined(__i386__)
	const char *osarch = "i386";
#elif defined(__aarch64__)
	const char *osarch = "aarch64";
#elif defined(__arm__)
	const char *osarch = "arm";
#else
	const char *osarch = "unknown";
#endif

	strncpy(pBuf->machine, osarch, sizeof(pBuf->machine));

	debug("%s %s %s %s", pBuf->sysname, pBuf->release,
		  pBuf->version, pBuf->machine);

	return 0;
}

static SyscallData scTbl[SYS_MAX] = {};
__constructor void __init_native_syscalls(void)
{
#define init_syscall(name, func) \
	scTbl[name] = {#name, (void *)func}

	/* Initialization */
	init_syscall(SYS_API_VERSION, sys_api_version);
	init_syscall(SYS_DEBUG_REPORT, sys_debug_report);

	/* I/O */
	init_syscall(SYS_READ, sys_read);
	init_syscall(SYS_PREAD, sys_pread);
	init_syscall(SYS_WRITE, sys_write);
	init_syscall(SYS_PWRITE, sys_pwrite);
	init_syscall(SYS_OPEN, sys_open);
	init_syscall(SYS_CLOSE, sys_close);
	init_syscall(SYS_IOCTL, sys_ioctl);
	init_syscall(SYS_FCNTL, sys_fcntl);

	/* File Status */
	init_syscall(SYS_STAT, sys_stat);
	init_syscall(SYS_FSTAT, sys_fstat);
	init_syscall(SYS_LSTAT, sys_lstat);
	init_syscall(SYS_ACCESS, sys_access);
	init_syscall(SYS_TRUNCATE, sys_truncate);
	init_syscall(SYS_FTRUNCATE, sys_ftruncate);
	init_syscall(SYS_TELL, sys_tell);
	init_syscall(SYS_SEEK, sys_seek);

	/* Process Control */
	init_syscall(SYS_EXIT, sys_exit);
	init_syscall(SYS_FORK, sys_fork);
	init_syscall(SYS_EXECVE, sys_execve);
	init_syscall(SYS_GETPID, sys_getpid);
	init_syscall(SYS_GETPPID, sys_getppid);
	init_syscall(SYS_WAITPID, sys_waitpid);
	init_syscall(SYS_KILL, sys_kill);
	init_syscall(SYS_PRCTL, sys_prctl);

	/* Memory */
	init_syscall(SYS_BRK, sys_brk);
	init_syscall(SYS_MMAP, sys_mmap);
	init_syscall(SYS_MUNMAP, sys_munmap);
	init_syscall(SYS_MPROTECT, sys_mprotect);
	init_syscall(SYS_MADVISE, sys_madvise);

	/* Communication */
	init_syscall(SYS_PIPE, sys_pipe);
	init_syscall(SYS_DUP, sys_dup);
	init_syscall(SYS_DUP2, sys_dup2);
	init_syscall(SYS_SOCKET, sys_socket);
	init_syscall(SYS_BIND, sys_bind);
	init_syscall(SYS_CONNECT, sys_connect);
	init_syscall(SYS_LISTEN, sys_listen);
	init_syscall(SYS_ACCEPT, sys_accept);
	init_syscall(SYS_SEND, sys_send);
	init_syscall(SYS_RECV, sys_recv);
	init_syscall(SYS_SHUTDOWN, sys_shutdown);

	/* Time */
	init_syscall(SYS_TIME, sys_time);
	init_syscall(SYS_CLOCK_GETTIME, sys_clock_gettime);
	init_syscall(SYS_CLOCK_SETTIME, sys_clock_settime);
	init_syscall(SYS_NANOSLEEP, sys_nanosleep);

	/* Miscellaneous */
	init_syscall(SYS_GETCWD, sys_getcwd);
	init_syscall(SYS_CHDIR, sys_chdir);
	init_syscall(SYS_MKDIR, sys_mkdir);
	init_syscall(SYS_RMDIR, sys_rmdir);
	init_syscall(SYS_UNLINK, sys_unlink);
	init_syscall(SYS_RENAME, sys_rename);
	init_syscall(SYS_UNAME, sys_uname);
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
