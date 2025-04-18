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
#include <stdbool.h>

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION "<unknown>"
#endif

int printf(const char *, ...);
int _dl_main(int, char *[], char *[]);

void print_license()
{
	printf("Fennix C Library Interpreter  Copyright (C) %s  EnderIce2\n", (__DATE__ + 7));
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions.\n");
}

void print_version()
{
	printf("%s\n", PROGRAM_VERSION);
}

void print_help()
{
	printf("Usage: ld.so [options] <program>\n");
	printf("Options:\n");
	printf("  --help       Display this help message\n");
	printf("  --version    Display version information\n");
	printf("  --license    Display license information\n");
	printf("\n");
	print_license();
}

bool IsManuallyInvoked(const char *argv0)
{
	const char *lastSlash = strrchr(argv0, '/');
	const char *name = lastSlash ? lastSlash + 1 : argv0;
	/* TODO: check if exe is actually "ld.so" or it has other name */
	return strcmp(name, "ld.so") == 0;
}

extern char **environ;
int main(int argc, char *argv[], char *envp[])
{
	environ = envp;
	if (!IsManuallyInvoked(argv[0]))
	{
		int status = _dl_main(argc, argv, envp);
		return status;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--help") == 0)
		{
			print_help();
			return 0;
		}
		if (strcmp(argv[i], "--version") == 0)
		{
			print_version();
			return 0;
		}
		if (strcmp(argv[i], "--license") == 0)
		{
			print_license();
			return 0;
		}
	}

	printf("Error: No program specified.\n");
	printf("Try 'ld.so --help' for more information.\n");
	return -1;
}
