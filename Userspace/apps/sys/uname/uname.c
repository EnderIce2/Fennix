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

#include <stdio.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void print_usage()
{
	printf("Usage: uname [OPTION]...\n");
	printf("Display specific system information. With no OPTION, defaults to -s.\n\n");
	printf("  -a, --all                display all information, in the following order,\n");
	printf("                             except omit -p and -i if unknown:\n");
	printf("  -s, --kernel-name        display the kernel name\n");
	printf("  -n, --nodename           display the network node hostname\n");
	printf("  -r, --kernel-release     display the kernel release\n");
	printf("  -v, --kernel-version     display the kernel version\n");
	printf("  -m, --machine            display the machine hardware name\n");
	printf("  -p, --processor          display the processor type (non-portable)\n");
	printf("  -i, --hardware-platform  display the hardware platform (non-portable)\n");
	printf("  -o, --operating-system   display the operating system\n");
	printf("      --help               show this help message and exit\n");
}

int main(int argc, char *argv[])
{
	struct utsname buffer;
	if (uname(&buffer) != 0)
	{
		perror("uname");
		exit(EXIT_FAILURE);
	}

	bool print_all = false;
	bool print_kernel_name = false;
	bool print_nodename = false;
	bool print_kernel_release = false;
	bool print_kernel_version = false;
	bool print_machine = false;
	bool print_processor = false;
	bool print_hardware_platform = false;
	bool print_operating_system = false;

	if (argc == 1)
		print_kernel_name = true;
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
			{
				print_all = true;
				break;
			}
			else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--kernel-name") == 0)
				print_kernel_name = true;
			else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nodename") == 0)
				print_nodename = true;
			else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--kernel-release") == 0)
				print_kernel_release = true;
			else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--kernel-version") == 0)
				print_kernel_version = true;
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--machine") == 0)
				print_machine = true;
			else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--processor") == 0)
				print_processor = true;
			else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--hardware-platform") == 0)
				print_hardware_platform = true;
			else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--operating-system") == 0)
				print_operating_system = true;
			else if (strcmp(argv[i], "--help") == 0)
			{
				print_usage();
				exit(EXIT_SUCCESS);
			}
			else
			{
				fprintf(stderr, "uname: invalid option -- '%s'\n", argv[i]);
				print_usage();
				exit(EXIT_FAILURE);
			}
		}
	}

	if (print_all || print_kernel_name)
		printf("%s ", buffer.sysname);
	if (print_all || print_nodename)
		printf("%s ", buffer.nodename);
	if (print_all || print_kernel_release)
		printf("%s ", buffer.release);
	if (print_all || print_kernel_version)
		printf("%s ", buffer.version);
	if (print_all || print_machine)
		printf("%s ", buffer.machine);
	if (print_all || print_processor)
		printf("%s ", buffer.machine); /* FIXME */
	if (print_all || print_hardware_platform)
		printf("%s ", buffer.machine); /* FIXME */
	if (print_all || print_operating_system)
		printf("%s ", buffer.sysname); /* FIXME */
	printf("\n");
	return 0;
}
