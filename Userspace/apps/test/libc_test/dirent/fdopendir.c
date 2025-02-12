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

#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>

/* https://pubs.opengroup.org/onlinepubs/9799919799/functions/fdopendir.html */

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
