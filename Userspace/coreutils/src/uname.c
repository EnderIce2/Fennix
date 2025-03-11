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

#include <stdio.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const char *GetOperatingSystemName(const char *sysname)
{
	if (strcmp(sysname, "Fennix") == 0)
		return "Fennix";
	if (strncmp(sysname, "Linux", 5) == 0)
		return "GNU/Linux";
	if (strncmp(sysname, "Darwin", 6) == 0)
		return "macOS";
	if (strncmp(sysname, "FreeBSD", 7) == 0)
		return "FreeBSD";
	if (strncmp(sysname, "NetBSD", 6) == 0)
		return "NetBSD";
	if (strncmp(sysname, "OpenBSD", 7) == 0)
		return "OpenBSD";
	if (strncmp(sysname, "DragonFly", 9) == 0)
		return "DragonFly BSD";
	if (strncmp(sysname, "SunOS", 5) == 0)
		return "SunOS";
	if (strncmp(sysname, "AIX", 3) == 0)
		return "AIX";
	if (strncmp(sysname, "HP-UX", 5) == 0)
		return "HP-UX";
	if (strncmp(sysname, "GNU", 3) == 0)
		return "GNU";
	if (strncmp(sysname, "Minix", 5) == 0)
		return "Minix";
	if (strncmp(sysname, "QNX", 3) == 0)
		return "QNX";
	if (strncmp(sysname, "Haiku", 5) == 0)
		return "Haiku";
	if (strncmp(sysname, "OS/2", 4) == 0)
		return "OS/2";

	return sysname;
}

const char *GetProcessorType(const char *machine)
{
	if (strcmp(machine, "x86_64") == 0)
		return "x86_64";
	if (strcmp(machine, "i686") == 0 || strcmp(machine, "i386") == 0)
		return "i686";
	if (strncmp(machine, "arm", 3) == 0)
		return "arm";
	if (strncmp(machine, "aarch64", 7) == 0)
		return "aarch64";
	if (strncmp(machine, "riscv64", 7) == 0)
		return "riscv64";
	if (strncmp(machine, "mips", 4) == 0)
		return "mips";
	if (strncmp(machine, "powerpc", 7) == 0)
		return "powerpc";
	if (strncmp(machine, "sparc", 5) == 0)
		return "sparc";

	return "unknown";
}

const char *GetHardwarePlatform(const char *machine)
{
	if (strcmp(machine, "x86_64") == 0)
		return "x86_64";
	if (strcmp(machine, "i686") == 0 || strcmp(machine, "i386") == 0)
		return "pc";
	if (strncmp(machine, "arm", 3) == 0)
		return "arm";
	if (strncmp(machine, "aarch64", 7) == 0)
		return "aarch64";
	if (strncmp(machine, "riscv64", 7) == 0)
		return "riscv64";
	if (strncmp(machine, "mips", 4) == 0)
		return "mips";
	if (strncmp(machine, "powerpc64le", 11) == 0)
		return "ppc64le";
	if (strncmp(machine, "powerpc", 7) == 0)
		return "powerpc";
	if (strncmp(machine, "sparc", 5) == 0)
		return "sparc";

	return "unknown";
}

void PrintUsage()
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
				PrintUsage();
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
		printf("%s ", GetProcessorType(buffer.machine));
	if (print_all || print_hardware_platform)
		printf("%s ", GetHardwarePlatform(buffer.machine));
	if (print_all || print_operating_system)
		printf("%s ", GetOperatingSystemName(buffer.sysname));
	printf("\n");
	return 0;
}
