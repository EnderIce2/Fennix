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

#include "../../kernel.h"

using namespace VirtualFileSystem;

void cmd_uname(const char *args)
{
	if (args)
	{
		if (strcmp(args, "-a") == 0)
		{
			printf("Fennix Kernel %s %s %s %s\n",
				   KERNEL_VERSION, KERNEL_NAME, __DATE__,
				   KERNEL_ARCH);
		}
		else if (strcmp(args, "-s") == 0)
		{
			printf("%s\n", KERNEL_NAME);
		}
		else if (strcmp(args, "-v") == 0)
		{
			printf("%s\n", KERNEL_VERSION);
		}
		else if (strcmp(args, "-n") == 0)
		{
			printf("unknown\n");
		}
		else if (strcmp(args, "-r") == 0)
		{
			printf("%s\n", KERNEL_NAME);
		}
		else if (strcmp(args, "-m") == 0)
		{
			printf("%s\n", KERNEL_ARCH);
		}
		else
		{
			printf("uname: invalid option: %s\n", args);
		}
	}
	else
	{
		printf("Fennix Kernel\n");
	}
}
