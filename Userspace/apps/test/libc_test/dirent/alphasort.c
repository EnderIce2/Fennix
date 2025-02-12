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

/* https://pubs.opengroup.org/onlinepubs/9799919799/functions/alphasort.html */

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
