/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

#ifdef __linux__
#define _DEFAULT_SOURCE 1 /* for alphasort */
#endif

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int test_alphasort(void)
{
	struct dirent **namelist;
	int n = scandir(".", &namelist, NULL, alphasort);
	if (n < 0)
	{
		perror("scandir");
		return n;
	}

	for (int i = 0; i < n; i++)
	{
		// printf("%s\n", namelist[i]->d_name);
		free(namelist[i]);
	}

	free(namelist);
	return 0;
}

int test_closedir(void)
{
	DIR *dir = opendir(".");
	if (closedir(dir) != 0)
		return 0x101;

	if (closedir(NULL) != -1)
		return 0x102;

	// if (closedir(dir) != -1) /* yeah... this will result in a core dump */
	// 	return 0x103;
	return 0;
}

int test_dirfd(void)
{
	DIR *dir = opendir(".");

	if (dirfd(dir) == -1)
		return 0x101;

	// if (dirfd(NULL) != -1)
	// 	return 0x102;

	return 0;
}

int test_fdopendir(void)
{
	DIR *dir;
	struct dirent *dp;

	int fd = open(".", O_RDONLY);

	dir = fdopendir(fd);
	if (dir == NULL)
		return 0x101;

	dp = readdir(dir);
	if (dp == NULL)
		return 0x102;

	if (closedir(dir) != 0)
		return 0x103;

	return 0;
}

int test_opendir()
{
	DIR *dir = opendir(".");
	if (dir == NULL)
		return 0x101;

	if (closedir(dir) != 0)
		return 0x102;

	return 0;
}

int test_posix_getdents()
{
	return 2;
}

int test_readdir_r()
{
	return 2;
}

int test_readdir()
{
	return 2;
}

int test_rewinddir()
{
	return 2;
}

int test_scandir()
{
	return 2;
}

int test_seekdir()
{
	return 2;
}

int test_telldir()
{
	return 2;
}
