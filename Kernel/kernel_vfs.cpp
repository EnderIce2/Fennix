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

#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <memory.hpp>

vfs::Virtual *fs = nullptr;

void SearchForInitrd()
{
	for (size_t i = 0; i < MAX_MODULES; i++)
	{
		uintptr_t moduleAddress = (uintptr_t)bInfo.Modules[i].Address;
		size_t moduleSize = bInfo.Modules[i].Size;
		const char *moduleCommand = bInfo.Modules[i].CommandLine;

		if (moduleAddress == 0)
			continue;

		Memory::Virtual vmm;
		if (!vmm.CheckRegion((void *)moduleAddress, moduleSize))
		{
			warn("module entry is not mapped!");
			vmm.Map((void *)moduleAddress, (void *)moduleAddress, moduleSize, Memory::RW);
		}

		if (strcmp(moduleCommand, "rootfs") == 0)
		{
			KPrint("rootfs found at %#lx", moduleAddress);
			if (TestAndInitializeUSTAR(moduleAddress, moduleSize, 0))
				continue;
		}
	}
}

EXTERNC NIF void KernelVFS()
{
	KPrint("Initializing Virtual File System");

	fs = new vfs::Virtual;
	SearchForInitrd();
	fs->Initialize();
}
