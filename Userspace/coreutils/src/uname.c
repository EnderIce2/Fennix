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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/utsname.h>

typedef union
{
	struct
	{
		uint8_t KernelName : 1;
		uint8_t NodeName : 1;
		uint8_t KernelRelease : 1;
		uint8_t KernelVersion : 1;
		uint8_t Machine : 1;
		uint8_t Processor : 1;
		uint8_t HardwarePlatform : 1;
		uint8_t OperatingSystem : 1;
	};
	uint8_t raw;
} UnameFlags;

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

	UnameFlags flags = {0};

	if (argc == 1)
		flags.KernelName = 1;
	else
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
			{
				flags.raw = 0xFF;
				break;
			}
			else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--kernel-name") == 0)
				flags.KernelName = 1;
			else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nodename") == 0)
				flags.NodeName = 1;
			else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--kernel-release") == 0)
				flags.KernelRelease = 1;
			else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--kernel-version") == 0)
				flags.KernelVersion = 1;
			else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--machine") == 0)
				flags.Machine = 1;
			else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--processor") == 0)
				flags.Processor = 1;
			else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--hardware-platform") == 0)
				flags.HardwarePlatform = 1;
			else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--operating-system") == 0)
				flags.OperatingSystem = 1;
			else if (strcmp(argv[i], "--help") == 0)
			{
				PrintUsage();
				exit(EXIT_SUCCESS);
			}
			else if (strcmp(argv[i], "--version") == 0)
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

	bool first = true;
#define PRINT_IF(flag, value) \
	if (flags.flag)           \
	{                         \
		if (!first)           \
			putchar(' ');     \
		printf("%s", value);  \
		first = false;        \
	}

	PRINT_IF(KernelName, buffer.sysname);
	PRINT_IF(NodeName, buffer.nodename);
	PRINT_IF(KernelRelease, buffer.release);
	PRINT_IF(KernelVersion, buffer.version);
	PRINT_IF(Machine, buffer.machine);
	PRINT_IF(Processor, GetProcessorType(buffer.machine));
	PRINT_IF(HardwarePlatform, GetHardwarePlatform(buffer.machine));
	PRINT_IF(OperatingSystem, GetOperatingSystemName(buffer.sysname));

	putchar('\n');
	return 0;
}
