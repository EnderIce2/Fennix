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

#include "../../kernel.h"

void cmd_lsmod(const char *)
{
	std::unordered_map<dev_t, Driver::DriverObject> drivers =
		DriverManager->GetDrivers();

	printf("DRIVER          |    ID | INIT | MEMORY\n");

	foreach (auto &drv in drivers)
	{
		printf("%-15s | %5ld |  %s   | %ld KiB\n",
			   drv.second.Name,
			   drv.first,
			   drv.second.Initialized ? "Y" : "N",
			   TO_KiB(drv.second.vma->GetAllocatedMemorySize()));
	}
}
