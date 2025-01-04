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

#include <stdio.h>
#include <string.h>

int printf(const char *, ...);
int _dl_main(int, char *[], char *[]);

void print_help()
{
	printf("Usage: ld.so [options] <program>\n");
	printf("Options:\n");
	printf("  --help       Display this help message\n");
}

int main(int argc, char *argv[], char *envp[])
{
	if (argc < 2)
	{
		printf("Error: No program specified.\n");
		print_help();
		return -1;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			print_help();
			return 0;
		}
	}

	int status = _dl_main(argc, argv, envp);
	return status;
}
