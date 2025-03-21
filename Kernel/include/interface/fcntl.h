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

#ifndef __FENNIX_API_FCNTL_H__
#define __FENNIX_API_FCNTL_H__

#ifdef __kernel__
#include <types.h>
#endif

/* cmd */
#define F_DUPFD 0x1
#define F_DUPFD_CLOEXEC 0x101
#define F_DUPFD_CLOFORK 0x201
#define F_GETFD 0x2
#define F_SETFD 0x3
#define F_GETFL 0x4
#define F_SETFL 0x5
#define F_GETLK 0x6
#define F_SETLK 0x7
#define F_SETLKW 0x8
#define F_OFD_GETLK 0x9
#define F_OFD_SETLK 0xA
#define F_OFD_SETLKW 0xB
#define F_GETOWN 0xC
#define F_GETOWN_EX 0xD
#define F_SETOWN 0xE
#define F_SETOWN_EX 0xF

#define FD_CLOEXEC 0x1
#define FD_CLOFORK 0x2

/* l_type */
#define F_RDLCK 0x1
#define F_UNLCK 0x2
#define F_WRLCK 0x3

/* type */
#define F_OWNER_PID 0
#define F_OWNER_PGRP 1

/* oflag */
#define O_CLOEXEC 02000000
#define O_CLOFORK 04000000
#define O_CREAT 0x8
#define O_DIRECTORY 0200000
#define O_EXCL 0x20
#define O_NOCTTY 0x40
#define O_NOFOLLOW 0400000
#define O_TRUNC 0x400
#define O_TTY_INIT 0x800

#define O_APPEND 0x4
#define O_DSYNC 0x10
#define O_NONBLOCK 0x80
#define O_RSYNC 0x100
#define O_SYNC 0x200

#define O_ACCMODE 0x3

#define O_EXEC 0x4
#define O_RDONLY 0x1
#define O_RDWR 0x3
#define O_SEARCH 0x10
#define O_WRONLY 0x2

#define AT_FDCWD
#define AT_EACCESS
#define AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_FOLLOW
#define AT_REMOVEDIR

#define POSIX_FADV_DONTNEED
#define POSIX_FADV_NOREUSE
#define POSIX_FADV_NORMAL
#define POSIX_FADV_RANDOM
#define POSIX_FADV_SEQUENTIAL
#define POSIX_FADV_WILLNEED

typedef struct f_owner_ex
{
	int type;  /* Discriminator for pid. */
	pid_t pid; /* Process ID or process group ID. */
} f_owner_ex;

typedef struct flock
{
	short l_type;	/* Type of lock; F_RDLCK, F_WRLCK, F_UNLCK. */
	short l_whence; /* Flag for starting offset. */
	off_t l_start;	/* Relative offset in bytes. */
	off_t l_len;	/* Size; if 0 then until EOF. */
	pid_t l_pid;	/* For a process-owned file lock, ignored on input or the process ID of the owning process on output; for an OFD-owned file lock, zero on input or (pid_t)-1 on output. */
} flock;

#endif // !__FENNIX_API_FCNTL_H__
