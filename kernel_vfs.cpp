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
		uintptr_t initrdAddress = (uintptr_t)bInfo.Modules[i].Address;

		if (!initrdAddress)
			continue;

		if (strcmp(bInfo.Modules[i].CommandLine, "initrd") != 0)
			continue;

		KPrint("initrd found at %#lx", initrdAddress);

		Memory::Virtual vmm;
		if (!vmm.Check((void *)initrdAddress))
		{
			warn("Initrd is not mapped!");
			vmm.Map((void *)initrdAddress, (void *)initrdAddress,
					bInfo.Modules[i].Size, Memory::RW);
		}

		if (TestAndInitializeUSTAR(initrdAddress, bInfo.Modules[i].Size))
			continue; /* Maybe add another root? */
	}
}

EXTERNC NIF void KernelVFS()
{
	KPrint("Initializing Virtual File System");

	fs = new vfs::Virtual;
	SearchForInitrd();
	fs->Initialize();
}
