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

namespace Subsystem::Linux
{
	bool Initialized = false;

	void InitializeSubSystem()
	{
		if (fs->RootExists(1) == false)
		{
			FileNode *nmnt = fs->GetByPath("/mnt", fs->GetRoot(0));
			assert(MountRAMFS(nmnt, "linux", 1));
			FileNode *linux = fs->GetRoot(1);

			FileNode *bin = fs->ForceCreate(linux, "bin", 0755);
			FileNode *boot = fs->ForceCreate(linux, "boot", 0755);
			FileNode *dev = fs->ForceCreate(linux, "dev", 0755);
			FileNode *etc = fs->ForceCreate(linux, "etc", 0755);
			FileNode *home = fs->ForceCreate(linux, "home", 0755);
			FileNode *lib = fs->ForceCreate(linux, "lib", 0755);
			FileNode *lib64 = fs->ForceCreate(linux, "lib64", 0755);
			FileNode *media = fs->ForceCreate(linux, "media", 0755);
			FileNode *mnt = fs->ForceCreate(linux, "mnt", 0755);
			FileNode *opt = fs->ForceCreate(linux, "opt", 0755);
			FileNode *proc = fs->ForceCreate(linux, "proc", 0755);
		}
	}
}
