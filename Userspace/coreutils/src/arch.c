/*
	This file is part of Fennix Core Utilities.

	Fennix Core Utilities is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Core Utilities is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Core Utilities. If not, see <https://www.gnu.org/licenses/>.
*/

#include <coreutils.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/utsname.h>

void PrintUsage()
{
	printf("Usage: arch [OPTION]...\n");
	printf("Display the machine hardware architecture name.\n\n");
	printf("      --help               show this help message and exit\n");
	printf("      --version            output version information and exit\n");
}

int main(int argc, char *argv[])
{
	struct utsname buffer;
	if (uname(&buffer) != 0)
	{
		perror("uname");
		exit(EXIT_FAILURE);
	}

	if (argc == 1)
		printf("%s\n", buffer.machine);
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "--help") == 0)
			{
				PrintUsage();
				exit(EXIT_SUCCESS);
			}
			else if (strcmp(argv[1], "--version") == 0)
			{
				PRINTF_VERSION;
				exit(EXIT_SUCCESS);
			}
			else
			{
				fprintf(stderr, "uname: invalid option -- '%s'\n", argv[i]);
				PrintUsage();
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}
