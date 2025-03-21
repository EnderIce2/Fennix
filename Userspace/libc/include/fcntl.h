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
#include <bits/fcntl.h>

	int creat(const char *path, mode_t mode);
	int fcntl(int fildes, int cmd, ...);
	int open(const char *path, int oflag, ...);
	int openat(int fd, const char *path, int oflag, ...);
	int posix_fadvise(int fd, off_t offset, off_t len, int advice);
	int posix_fallocate(int fd, off_t offset, off_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_FCNTL_H
