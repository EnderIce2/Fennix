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
		uint8_t kernelName : 1;
		uint8_t nodeName : 1;
		uint8_t kernelRelease : 1;
		uint8_t kernelVersion : 1;
		uint8_t machine : 1;
		uint8_t processor : 1;
		uint8_t hardwarePlatform : 1;
		uint8_t operatingSystem : 1;
	};
	uint8_t raw;
} UnameFlags;

const char *GetOperatingSystemName(const char *systemName)
{
	if (strcmp(systemName, "Fennix") == 0)
		return "Fennix";
	if (strncmp(systemName, "Linux", 5) == 0)
		return "GNU/Linux";
	if (strncmp(systemName, "Darwin", 6) == 0)
		return "macOS";
	if (strncmp(systemName, "FreeBSD", 7) == 0)
		return "FreeBSD";
	if (strncmp(systemName, "NetBSD", 6) == 0)
		return "NetBSD";
	if (strncmp(systemName, "OpenBSD", 7) == 0)
		return "OpenBSD";
	if (strncmp(systemName, "DragonFly", 9) == 0)
		return "DragonFly BSD";
	if (strncmp(systemName, "SunOS", 5) == 0)
		return "SunOS";
	if (strncmp(systemName, "AIX", 3) == 0)
		return "AIX";
	if (strncmp(systemName, "HP-UX", 5) == 0)
		return "HP-UX";
	if (strncmp(systemName, "GNU", 3) == 0)
		return "GNU";
	if (strncmp(systemName, "Minix", 5) == 0)
		return "Minix";
	if (strncmp(systemName, "QNX", 3) == 0)
		return "QNX";
	if (strncmp(systemName, "Haiku", 5) == 0)
		return "Haiku";
	if (strncmp(systemName, "OS/2", 4) == 0)
		return "OS/2";

	return systemName;
}

const char *GetProcessorType(const char *machine)
{
	if (strcmp(machine, "x86_64") == 0)
		return "x86_64";
	if (strcmp(machine, "i386") == 0)
		return "i386";
	if (strcmp(machine, "i686") == 0)
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

	if (argc == 1)
	{
		printf("%s\n", buffer.sysname);
		return 0;
	}

	UnameFlags flags = {0};
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0)
		{
			flags.raw = 0xFF;
			break;
		}
		else if (argv[i][0] == '-' && argv[i][1] != '\0')
		{
			for (size_t j = 1; j < strlen(argv[i]); j++)
			{
				switch (argv[i][j])
				{
				case 's':
					flags.kernelName = 1;
					break;
				case 'n':
					flags.nodeName = 1;
					break;
				case 'r':
					flags.kernelRelease = 1;
					break;
				case 'v':
					flags.kernelVersion = 1;
					break;
				case 'm':
					flags.machine = 1;
					break;
				case 'p':
					flags.processor = 1;
					break;
				case 'i':
					flags.hardwarePlatform = 1;
					break;
				case 'o':
					flags.operatingSystem = 1;
					break;
				default:
					fprintf(stderr, "uname: invalid option -- '%c'\n", argv[i][j]);
					PrintUsage();
					exit(EXIT_FAILURE);
				}
			}
		}
		else if (strcmp(argv[i], "--kernel-name") == 0)
			flags.kernelName = 1;
		else if (strcmp(argv[i], "--nodename") == 0)
			flags.nodeName = 1;
		else if (strcmp(argv[i], "--kernel-release") == 0)
			flags.kernelRelease = 1;
		else if (strcmp(argv[i], "--kernel-version") == 0)
			flags.kernelVersion = 1;
		else if (strcmp(argv[i], "--machine") == 0)
			flags.machine = 1;
		else if (strcmp(argv[i], "--processor") == 0)
			flags.processor = 1;
		else if (strcmp(argv[i], "--hardware-platform") == 0)
			flags.hardwarePlatform = 1;
		else if (strcmp(argv[i], "--operating-system") == 0)
			flags.operatingSystem = 1;
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

	bool first = true;
#define PRINT_IF(flag, value) \
	if (flags.flag)           \
	{                         \
		if (!first)           \
			putchar(' ');     \
		printf("%s", value);  \
		first = false;        \
	}

	PRINT_IF(kernelName, buffer.sysname);
	PRINT_IF(nodeName, buffer.nodename);
	PRINT_IF(kernelRelease, buffer.release);
	PRINT_IF(kernelVersion, buffer.version);
	PRINT_IF(machine, buffer.machine);
	PRINT_IF(processor, GetProcessorType(buffer.machine));
	PRINT_IF(hardwarePlatform, GetHardwarePlatform(buffer.machine));
	PRINT_IF(operatingSystem, GetOperatingSystemName(buffer.sysname));

	putchar('\n');
	return 0;
}
