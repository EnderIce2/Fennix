/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#include <bits/syscalls.h>
#include <bits/libc.h>
#include <string.h>

void sysdep(Exit)(int Status)
{
	syscall1(sys_exit, Status);
}

int sysdep(Accept)(int Socket, struct sockaddr *restrict Address, socklen_t *restrict AddressLength)
{
	return syscall3(sys_accept, Socket, Address, AddressLength);
}

int sysdep(Bind)(int Socket, const struct sockaddr *Address, socklen_t AddressLength)
{
	return syscall3(sys_bind, Socket, Address, AddressLength);
}

int sysdep(Connect)(int Socket, const struct sockaddr *Address, socklen_t AddressLength)
{
	return syscall3(sys_connect, Socket, Address, AddressLength);
}

int sysdep(Listen)(int Socket, int Backlog)
{
	return syscall2(sys_listen, Socket, Backlog);
}

int sysdep(Socket)(int Domain, int Type, int Protocol)
{
	return syscall3(sys_socket, Domain, Type, Protocol);
}

int sysdep(UnixName)(struct utsname *Name)
{
	struct kutsname kname;
	int result = syscall1(sys_uname, &kname);
	if (result == 0)
	{
		strcpy(Name->sysname, kname.sysname);
		strcpy(Name->release, kname.release);
		strcpy(Name->version, kname.version);
		strcpy(Name->machine, kname.machine);
		return 0;
	}

	return result;
}

int sysdep(WaitProcessID)(pid_t ProcessID, int *Status, int Options)
{
	return syscall3(sys_wait4, ProcessID, (scarg)Status, Options);
}

int sysdep(IOControl)(int Descriptor, unsigned long Operation, void *Argument)
{
	return syscall3(sys_ioctl, Descriptor, Operation, (scarg)Argument);
}

void *sysdep(MemoryMap)(void *Address, size_t Length, int Protection, int Flags, int Descriptor, off_t Offset)
{
	return syscall6(sys_mmap, (scarg)Address, Length, Protection, Flags, Descriptor, Offset);
}

int sysdep(MemoryUnmap)(void *Address, size_t Length)
{
	return syscall2(sys_munmap, (scarg)Address, Length);
}

int sysdep(MemoryProtect)(void *Address, size_t Length, int Protection)
{
	return syscall3(sys_mprotect, (scarg)Address, Length, Protection);
}

int sysdep(Fork)(void)
{
	return syscall0(sys_fork);
}

int sysdep(Read)(int Descriptor, void *Buffer, size_t Size)
{
	return syscall3(sys_read, Descriptor, (scarg)Buffer, Size);
}

int sysdep(Write)(int Descriptor, const void *Buffer, size_t Size)
{
	return syscall3(sys_write, Descriptor, (scarg)Buffer, Size);
}

int sysdep(PRead)(int Descriptor, void *Buffer, size_t Size, off_t Offset)
{
	return syscall4(sys_pread64, Descriptor, (scarg)Buffer, Size, Offset);
}

int sysdep(PWrite)(int Descriptor, const void *Buffer, size_t Size, off_t Offset)
{
	return syscall4(sys_pwrite64, Descriptor, (scarg)Buffer, Size, Offset);
}

int sysdep(Open)(const char *Pathname, int Flags, mode_t Mode)
{
	return syscall3(sys_open, (scarg)Pathname, Flags, Mode);
}

int sysdep(Close)(int Descriptor)
{
	return syscall1(sys_close, Descriptor);
}

int sysdep(Access)(const char *Pathname, int Mode)
{
	return syscall2(sys_access, (scarg)Pathname, Mode);
}

int sysdep(Tell)(int Descriptor)
{
#undef SEEK_CUR
#define SEEK_CUR 1
	return syscall3(sys_lseek, Descriptor, 0, SEEK_CUR);
}

int sysdep(Seek)(int Descriptor, off_t Offset, int Whence)
{
	return syscall3(sys_lseek, Descriptor, Offset, Whence);
}

pid_t sysdep(GetProcessID)(void)
{
	return syscall0(sys_getpid);
}

pid_t sysdep(GetParentProcessID)(void)
{
	return syscall0(sys_getppid);
}

int sysdep(Execve)(const char *Pathname, char *const *Argv, char *const *Envp)
{
	return syscall3(sys_execve, (scarg)Pathname, (scarg)Argv, (scarg)Envp);
}

int sysdep(Kill)(pid_t ProcessID, int Signal)
{
	return syscall2(sys_kill, ProcessID, Signal);
}

int sysdep(Stat)(const char *Pathname, struct stat *Statbuf)
{
	return syscall2(sys_stat, (scarg)Pathname, (scarg)Statbuf);
}

int sysdep(FStat)(int Descriptor, struct stat *Statbuf)
{
	return syscall2(sys_fstat, Descriptor, (scarg)Statbuf);
}

int sysdep(LStat)(const char *Pathname, struct stat *Statbuf)
{
	return syscall2(sys_lstat, (scarg)Pathname, (scarg)Statbuf);
}

int sysdep(Truncate)(const char *Pathname, off_t Length)
{
	return syscall2(sys_truncate, (scarg)Pathname, Length);
}

int sysdep(MakeDirectory)(const char *Pathname, mode_t Mode)
{
	return syscall2(sys_mkdir, (scarg)Pathname, Mode);
}

int sysdep(ProcessControl)(unsigned long Option, unsigned long Arg1, unsigned long Arg2, unsigned long Arg3, unsigned long Arg4)
{
	return syscall5(sys_prctl, Option, Arg1, Arg2, Arg3, Arg4);
}
