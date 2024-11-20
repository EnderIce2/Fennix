/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include "../cmds.hpp"

#include <filesystem.hpp>
#include <acpi.hpp>

#include "../../kernel.h"

using namespace vfs;

void cmd_dump(const char *args)
{
	if (args[0] == '\0')
		return;

	// args == "0xdeadbeef 512" aka "address size"

	char *strAddr = (char *)args;
	char *strLen = (char *)args;
	while (*strLen != ' ')
		strLen++;
	*strLen = '\0';
	strLen++;

	void *Address = (void *)strtoul(strAddr, nullptr, 16);
	unsigned long Length = strtoul(strLen, nullptr, 10);

	{
		unsigned char *AddressChar = (unsigned char *)Address;
		unsigned char Buffer[17];
		uint32_t Iterate;

		printf("Dumping %lu bytes from %#lx\n", Length, (uintptr_t)AddressChar);

		for (Iterate = 0; Iterate < (uint32_t)Length; Iterate++)
		{
			if ((Iterate % 16) == 0)
			{
				if (Iterate != 0)
					printf("  %s\n", Buffer);
				printf("  %04x ", Iterate);
			}

			printf(" %02x", AddressChar[Iterate]);

			if ((AddressChar[Iterate] < 0x20) || (AddressChar[Iterate] > 0x7E))
				Buffer[Iterate % 16] = '.';
			else
				Buffer[Iterate % 16] = AddressChar[Iterate];

			Buffer[(Iterate % 16) + 1] = '\0';
		}

		while ((Iterate % 16) != 0)
		{
			printf("   ");
			Iterate++;
		}
		putchar('\n');
	}
}