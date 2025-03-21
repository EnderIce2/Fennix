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

#ifndef FENNIX_BITS_LIBC_H
#define FENNIX_BITS_LIBC_H

#include <bits/socket.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __kernel__
#error "Kernel code should not include this header"
#endif // __kernel__

#ifndef export
#define export __attribute__((__visibility__("default")))
#endif // export

#define sysdep(name) \
	__libc_##name

void sysdep(Exit)(int Status);
int sysdep(Accept)(int Socket, struct sockaddr *restrict Address, socklen_t *restrict AddressLength);
int sysdep(Bind)(int Socket, const struct sockaddr *Address, socklen_t AddressLength);
int sysdep(Connect)(int Socket, const struct sockaddr *Address, socklen_t AddressLength);
int sysdep(Listen)(int Socket, int Backlog);
int sysdep(Socket)(int Domain, int Type, int Protocol);
int sysdep(UnixName)(struct utsname *Name);
int sysdep(WaitProcessID)(pid_t ProcessID, int *Status, int Options);
int sysdep(IOControl)(int Descriptor, unsigned long Operation, void *Argument);
void *sysdep(MemoryMap)(void *Address, size_t Length, int Protection, int Flags, int Descriptor, off_t Offset);
int sysdep(MemoryUnmap)(void *Address, size_t Length);
int sysdep(MemoryProtect)(void *Address, size_t Length, int Protection);
int sysdep(Fork)(void);
int sysdep(Read)(int Descriptor, void *Buffer, size_t Size);
int sysdep(Write)(int Descriptor, const void *Buffer, size_t Size);
int sysdep(PRead)(int Descriptor, void *Buffer, size_t Size, off_t Offset);
int sysdep(PWrite)(int Descriptor, const void *Buffer, size_t Size, off_t Offset);
int sysdep(Open)(const char *Pathname, int Flags, mode_t Mode);
int sysdep(Close)(int Descriptor);
int sysdep(Access)(const char *Pathname, int Mode);
int sysdep(Tell)(int Descriptor);
int sysdep(Seek)(int Descriptor, off_t Offset, int Whence);
pid_t sysdep(GetProcessID)(void);
pid_t sysdep(GetParentProcessID)(void);
int sysdep(Execve)(const char *Pathname, char *const *Argv, char *const *Envp);
int sysdep(Kill)(pid_t ProcessID, int Signal);
int sysdep(Stat)(const char *Pathname, struct stat *Statbuf);
int sysdep(FStat)(int Descriptor, struct stat *Statbuf);
int sysdep(LStat)(const char *Pathname, struct stat *Statbuf);
int sysdep(Truncate)(const char *Pathname, off_t Length);
int sysdep(MakeDirectory)(const char *Pathname, mode_t Mode);
int sysdep(ProcessControl)(unsigned long Option, unsigned long Arg1, unsigned long Arg2, unsigned long Arg3, unsigned long Arg4);
int sysdep(ChangeDirectory)(const char *Pathname);
char *sysdep(GetWorkingDirectory)(char *Buffer, size_t Size);
int sysdep(Brk)(void *Address);
int sysdep(FileControl)(int Descriptor, int Command, void *Arg);

#endif // FENNIX_BITS_LIBC_H
