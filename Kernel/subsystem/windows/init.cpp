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

#include <fs/ramfs.hpp>

#include "../../kernel.h"

namespace Subsystem::Windows
{
	bool Initialized = false;

	void __CreateStubRoot()
	{
		Node root = fs->GetRoot(0);
		Node nmnt = fs->Lookup(root, "/mnt");
		assert(MountAndRootRAMFS(nmnt, "windows", 2));
		Node win = fs->GetRoot(2);

		Node windows = fs->Create(win, "Windows", 0755);
		Node programFiles = fs->Create(windows, "Program Files", 0755);
		Node programFilesX86 = fs->Create(windows, "Program Files (x86)", 0755);
		Node programData = fs->Create(windows, "ProgramData", 0755);
		Node users = fs->Create(windows, "Users", 0755);

		UNUSED(windows);
		UNUSED(programFiles);
		UNUSED(programFilesX86);
		UNUSED(programData);
		UNUSED(users);
	}

	void InitializeSubSystem()
	{
		if (fs->RootExists(2) == false)
		{
		}
	}
}
