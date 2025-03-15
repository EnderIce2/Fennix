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

#include <bits/libc.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

export int chmod(const char *, mode_t);
export int fchmod(int, mode_t);
export int fchmodat(int, const char *, mode_t, int);

export int fstat(int fildes, struct stat *buf)
{
	if (fildes < 0)
	{
		errno = EBADF;
		return -1;
	}

	return __check_errno(sysdep(FStat)(fildes, buf), -1);
}

export int fstatat(int fd, const char *restrict path, struct stat *restrict buf, int flag)
{
	if (fd < 0 || path == NULL || buf == NULL)
	{
		errno = EBADF;
		return -1;
	}

	/* FIXME: fstatat is not implemented in kernel */

	return __check_errno(ENOSYS, -1);
	// return__check_errno(call_fstatat(fd, path, buf, flag), -1);
}

export int futimens(int, const struct timespec[2]);

export int lstat(const char *restrict path, struct stat *restrict buf)
{
	if (path == NULL || buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	return __check_errno(sysdep(LStat)(path, buf), -1);
}

export int mkdir(const char *path, mode_t mode)
{
	return __check_errno(sysdep(MakeDirectory)(path, mode), -1);
}

export int mkdirat(int fd, const char *path, mode_t mode)
{
	printf("mkdirat() is unimplemented\n");
	return __check_errno(-ENOSYS, -1);
}

export int mkfifo(const char *, mode_t);
export int mkfifoat(int, const char *, mode_t);
export int mknod(const char *, mode_t, dev_t);
export int mknodat(int, const char *, mode_t, dev_t);

export int stat(const char *restrict path, struct stat *restrict buf)
{
	if (path == NULL || buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	return __check_errno(sysdep(Stat)(path, buf), -1);
}

export mode_t umask(mode_t);
export int utimensat(int, const char *, const struct timespec[2], int);
