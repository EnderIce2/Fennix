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

#include <filesystem/ramfs.hpp>

#include "../../kernel.h"

namespace Subsystem::Windows
{
	bool Initialized = false;

	void InitializeSubSystem()
	{
		if (fs->RootExists(2) == false)
		{
			FileNode *nmnt = fs->GetByPath("/mnt", fs->GetRoot(0));
			assert(MountRAMFS(nmnt, "windows", 2));
			FileNode *win = fs->GetRoot(2);

			FileNode *windows = fs->ForceCreate(win, "Windows", 0755);
			FileNode *programFiles = fs->ForceCreate(windows, "Program Files", 0755);
			FileNode *programFilesX86 = fs->ForceCreate(windows, "Program Files (x86)", 0755);
			FileNode *programData = fs->ForceCreate(windows, "ProgramData", 0755);
			FileNode *users = fs->ForceCreate(windows, "Users", 0755);
		}
	}
}
