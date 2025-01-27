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

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

export int alphasort(const struct dirent **, const struct dirent **);

export int closedir(DIR *dirp)
{
	if (!dirp)
	{
		errno = EBADF;
		return -1;
	}

	int fd = dirfd(dirp);
	if (fd == -1)
	{
		errno = EBADF;
		return -1;
	}

	int result = close(fd);
	if (result == -1)
		return -1;

	free(dirp);
	return 0;
}

export int dirfd(DIR *dirp)
{
	printf("dirfd() is unimplemented\n");
	return __check_errno(-ENOSYS, -1);
}

export DIR *fdopendir(int);
export DIR *opendir(const char *);
export ssize_t posix_getdents(int, void *, size_t, int);
export struct dirent *readdir(DIR *);
export int readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
export void rewinddir(DIR *);
export int scandir(const char *, struct dirent ***, int (*)(const struct dirent *), int (*)(const struct dirent **, const struct dirent **));
export void seekdir(DIR *, long);
export long telldir(DIR *);
