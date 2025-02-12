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

/* https://pubs.opengroup.org/onlinepubs/9799919799/functions/dirfd.html */

int test_dirfd(void)
{
	DIR *dir = opendir(".");

	if (dirfd(dir) == -1)
		return 0x101;

	// if (dirfd(NULL) != -1)
	// 	return 0x102;

	return 0;
}
