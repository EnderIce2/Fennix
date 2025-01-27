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

#ifndef _FCNTL_H
#define _FCNTL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>

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

#define F_DUPFD
#define F_DUPFD_CLOEXEC
#define F_DUPFD_CLOFORK
#define F_GETFD
#define F_SETFD
#define F_GETFL
#define F_SETFL
#define F_GETLK
#define F_SETLK
#define F_SETLKW
#define F_OFD_GETLK
#define F_OFD_SETLK
#define F_OFD_SETLKW
#define F_GETOWN
#define F_GETOWN_EX
#define F_SETOWN
#define F_SETOWN_EX
#define FD_CLOEXEC
#define FD_CLOFORK
#define F_RDLCK
#define F_UNLCK
#define F_WRLCK
#define F_OWNER_PID
#define F_OWNER_PGRP
#define O_CLOEXEC 02000000
#define O_CLOFORK
#define O_CREAT 0100
#define O_DIRECTORY
#define O_EXCL 0200
#define O_NOCTTY
#define O_NOFOLLOW 0400000
#define O_TRUNC 01000
#define O_TTY_INIT
#define O_APPEND 02000
#define O_DSYNC
#define O_NONBLOCK
#define O_RSYNC
#define O_SYNC
#define O_ACCMODE
#define O_EXEC
#define O_RDONLY 00
#define O_RDWR 02
#define O_SEARCH
#define O_WRONLY 01
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

	int creat(const char *, mode_t);
	int fcntl(int, int, ...);
	int open(const char *, int, ...);
	int openat(int, const char *, int, ...);
	int posix_fadvise(int, off_t, off_t, int);
	int posix_fallocate(int, off_t, off_t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_FCNTL_H
