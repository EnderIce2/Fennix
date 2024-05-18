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

void cmd_modinfo(const char *args)
{
	if (args[0] == '\0')
	{
		printf("Usage: modinfo <driver id/name>\n");
		return;
	}

	dev_t id = atoi(args);

	std::unordered_map<dev_t, Driver::DriverObject> drivers =
		DriverManager->GetDrivers();

	if (drivers.find(id) == drivers.end())
	{
		bool found = false;
		foreach (auto var in drivers)
		{
			if (strcmp(var.second.Name, args) == 0)
			{
				id = var.first;
				found = true;
				break;
			}
		}

		if (!found)
		{
			printf("Driver not found\n");
			return;
		}
	}

	Driver::DriverObject drv = drivers[id];
	printf("Base Info:\n");
	printf(" Name: %s\n", drv.Name);
	printf(" Description: %s\n", drv.Description);
	printf(" Author: %s\n", drv.Author);
	printf(" Version: %s\n", drv.Version);
	printf(" License: %s\n", drv.License);
	printf("Resource Info:\n");
	printf(" Initialized: %s\n", drv.Initialized ? "yes" : "no");
	printf(" Error Code: %i (%s)\n", drv.ErrorCode, strerror(drv.ErrorCode));
	printf(" Path: %s\n", drv.Path.c_str());
	printf(" Used Memory: %ld KiB\n", TO_KiB(drv.vma->GetAllocatedMemorySize()));
	printf(" Used IRQs:%s\n", drv.InterruptHandlers->empty() ? " none" : "");
	foreach (auto var in *drv.InterruptHandlers)
	{
		printf("  IRQ%-2d: %#lx\n",
			   var.first, (uintptr_t)var.second);
	}
}
