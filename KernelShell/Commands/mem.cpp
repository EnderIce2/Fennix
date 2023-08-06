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
#include <task.hpp>

#include "../../kernel.h"

using namespace VirtualFileSystem;
using namespace Tasking;

void cmd_mem(const char *)
{
	uint64_t total = KernelAllocator.GetTotalMemory();
	uint64_t used = KernelAllocator.GetUsedMemory();
	uint64_t free = KernelAllocator.GetFreeMemory();
	uint64_t reserved = KernelAllocator.GetReservedMemory();

	int usedPercent = (int)((used * 100) / total);
	int usedBar = (int)(usedPercent / 2);

	int reservedPercent = (int)((reserved * 100) / total);
	int reservedBar = (int)(reservedPercent / 2);

	printf("[");
	for (int i = 0; i < usedBar; i++)
		printf("=");
	for (int i = 0; i < 50 - usedBar; i++)
		printf(" ");
	printf("] %d%% Used\n", usedPercent);

	printf("[");
	for (int i = 0; i < reservedBar; i++)
		printf("=");
	for (int i = 0; i < 50 - reservedBar; i++)
		printf(" ");
	printf("] %d%% Reserved\n", reservedPercent);

	// printf("Total: %d MiB\n", (int)(TO_MiB(total)));
	// printf("Used: %d MiB\n", (int)(TO_MiB(used)));
	// printf("Free: %d MiB\n", (int)(TO_MiB(free)));
	// printf("Reserved: %d MiB\n", (int)(TO_MiB(reserved)));

	printf("TOTAL      USED      FREE      RESERVED\n");
	printf("%d MiB    %d MiB    %d MiB    %d MiB\n",
		   (int)(TO_MiB(total)), (int)(TO_MiB(used)),
		   (int)(TO_MiB(free)), (int)(TO_MiB(reserved)));
}
