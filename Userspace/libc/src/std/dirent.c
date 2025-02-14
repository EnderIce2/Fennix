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
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

export int alphasort(const struct dirent **d1, const struct dirent **d2)
{
	return strcoll((*d1)->d_name, (*d2)->d_name);
}

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

export DIR *fdopendir(int fd)
{
	DIR *dirp = malloc(sizeof(DIR));
	if (dirp == NULL)
		return NULL;

	dirp->__fd = fd;
	return dirp;
}

export DIR *opendir(const char *dirname)
{
	DIR *dirp = malloc(sizeof(DIR));
	if (dirp == NULL)
		return NULL;

	dirp->__fd = open(dirname, O_RDONLY);
	if (dirp->__fd == -1)
	{
		free(dirp);
		return NULL;
	}

	return dirp;
}

export ssize_t posix_getdents(int fildes, void *buf, size_t nbyte, int flags)
{
	printf("posix_getdents() is unimplemented\n");
	return __check_errno(-ENOSYS, -1);
}

export struct dirent *readdir(DIR *dirp)
{
	if (!dirp)
	{
		errno = EBADF;
		return NULL;
	}

	struct dirent *entry = malloc(sizeof(struct dirent));
	if (entry == NULL)
		return NULL;

	ssize_t bytes = posix_getdents(dirp->__fd, entry, sizeof(struct dirent), 0);
	if (bytes == -1)
	{
		free(entry);
		return NULL;
	}

	return entry;
}

export int readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
export void rewinddir(DIR *);

export int scandir(const char *dir, struct dirent ***namelist, int (*sel)(const struct dirent *), int (*compar)(const struct dirent **, const struct dirent **))
{
	struct dirent **entry_list = NULL;
	struct dirent *entry;
	DIR *dp;
	int count = 0;
	int capacity = 10;

	dp = opendir(dir);
	if (dp == NULL)
		return -1;

	entry_list = malloc(capacity * sizeof(struct dirent *));
	if (entry_list == NULL)
	{
		closedir(dp);
		return -1;
	}

	while ((entry = readdir(dp)) != NULL)
	{
		if (sel == NULL || sel(entry))
		{
			if (count >= capacity)
			{
				capacity *= 2;
				struct dirent **new_list = realloc(entry_list, capacity * sizeof(struct dirent *));
				if (new_list == NULL)
				{
					for (int i = 0; i < count; i++)
						free(entry_list[i]);
					free(entry_list);
					closedir(dp);
					return -1;
				}
				entry_list = new_list;
			}
			entry_list[count] = malloc(sizeof(struct dirent));
			if (entry_list[count] == NULL)
			{
				for (int i = 0; i < count; i++)
					free(entry_list[i]);
				free(entry_list);
				closedir(dp);
				return -1;
			}
			memcpy(entry_list[count], entry, sizeof(struct dirent));
			count++;
		}
	}

	closedir(dp);

	if (count > 0)
		qsort(entry_list, count, sizeof(struct dirent *), (int (*)(const void *, const void *))compar);

	*namelist = entry_list;
	return count;
}

export void seekdir(DIR *, long);
export long telldir(DIR *);
