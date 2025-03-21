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
	call_exit(Status);
}

int sysdep(Accept)(int Socket, struct sockaddr *restrict Address, socklen_t *restrict AddressLength)
{
	return call_accept(Socket, Address, AddressLength);
}

int sysdep(Bind)(int Socket, const struct sockaddr *Address, socklen_t AddressLength)
{
	return call_bind(Socket, Address, AddressLength);
}

int sysdep(Connect)(int Socket, const struct sockaddr *Address, socklen_t AddressLength)
{
	return call_connect(Socket, Address, AddressLength);
}

int sysdep(Listen)(int Socket, int Backlog)
{
	return call_listen(Socket, Backlog);
}

int sysdep(Socket)(int Domain, int Type, int Protocol)
{
	return call_socket(Domain, Type, Protocol);
}

int sysdep(UnixName)(struct utsname *Name)
{
	struct kutsname kname;
	int result = call_uname(&kname);
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
	return call_waitpid(ProcessID, Status, Options);
}

int sysdep(IOControl)(int Descriptor, unsigned long Operation, void *Argument)
{
	return call_ioctl(Descriptor, Operation, Argument);
}

void *sysdep(MemoryMap)(void *Address, size_t Length, int Protection, int Flags, int Descriptor, off_t Offset)
{
	return (void *)call_mmap(Address, Length, Protection, Flags, Descriptor, Offset);
}

int sysdep(MemoryUnmap)(void *Address, size_t Length)
{
	return call_munmap(Address, Length);
}

int sysdep(MemoryProtect)(void *Address, size_t Length, int Protection)
{
	return call_mprotect(Address, Length, Protection);
}

int sysdep(Fork)(void)
{
	return call_fork();
}

int sysdep(Read)(int Descriptor, void *Buffer, size_t Size)
{
	return call_read(Descriptor, Buffer, Size);
}

int sysdep(Write)(int Descriptor, const void *Buffer, size_t Size)
{
	return call_write(Descriptor, Buffer, Size);
}

int sysdep(PRead)(int Descriptor, void *Buffer, size_t Size, off_t Offset)
{
	return call_pread(Descriptor, Buffer, Size, Offset);
}

int sysdep(PWrite)(int Descriptor, const void *Buffer, size_t Size, off_t Offset)
{
	return call_pwrite(Descriptor, Buffer, Size, Offset);
}

int sysdep(Open)(const char *Pathname, int Flags, mode_t Mode)
{
	return call_open(Pathname, Flags, Mode);
}

int sysdep(Close)(int Descriptor)
{
	return call_close(Descriptor);
}

int sysdep(Access)(const char *Pathname, int Mode)
{
	return call_access(Pathname, Mode);
}

int sysdep(Tell)(int Descriptor)
{
	return call_tell(Descriptor);
}

int sysdep(Seek)(int Descriptor, off_t Offset, int Whence)
{
	return call_seek(Descriptor, Offset, Whence);
}

pid_t sysdep(GetProcessID)(void)
{
	return call_getpid();
}

pid_t sysdep(GetParentProcessID)(void)
{
	return call_getppid();
}

int sysdep(Execve)(const char *Pathname, char *const *Argv, char *const *Envp)
{
	return call_execve(Pathname, Argv, Envp);
}

int sysdep(Kill)(pid_t ProcessID, int Signal)
{
	return call_kill(ProcessID, Signal);
}

int sysdep(Stat)(const char *Pathname, struct stat *Statbuf)
{
	return call_stat(Pathname, Statbuf);
}

int sysdep(FStat)(int Descriptor, struct stat *Statbuf)
{
	return call_fstat(Descriptor, Statbuf);
}

int sysdep(LStat)(const char *Pathname, struct stat *Statbuf)
{
	return call_lstat(Pathname, Statbuf);
}

int sysdep(Truncate)(const char *Pathname, off_t Length)
{
	return call_truncate(Pathname, Length);
}

int sysdep(MakeDirectory)(const char *Pathname, mode_t Mode)
{
	return call_mkdir(Pathname, Mode);
}

int sysdep(ProcessControl)(unsigned long Option, unsigned long Arg1, unsigned long Arg2, unsigned long Arg3, unsigned long Arg4)
{
	return call_prctl(Option, Arg1, Arg2, Arg3, Arg4);
}

int sysdep(ChangeDirectory)(const char *Pathname)
{
	return call_chdir(Pathname);
}

char *sysdep(GetWorkingDirectory)(char *Buffer, size_t Size)
{
	return (char *)call_getcwd(Buffer, Size);
}

int sysdep(Brk)(void *Address)
{
	return call_brk(Address);
}

int sysdep(FileControl)(int Descriptor, int Command, void *Arg)
{
	return call_fcntl(Descriptor, Command, Arg);
}
