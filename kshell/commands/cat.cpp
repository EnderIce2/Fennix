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

using namespace vfs;

void cmd_cat(const char *args)
{
	if (args[0] == '\0')
		return;

	/* FIXME: Reimplement this later */
	assert(!"Function not implemented");
	// Node *thisNode = fs->GetByPath(args, thisProcess->CWD, true);
	// if (thisNode == nullptr)
	// {
	// 	printf("cat: %s: No such file or directory\n", args);
	// 	return;
	// }

	// if (!thisNode->Stat.IsType(FILE) && !thisNode->Stat.IsType(CHARDEVICE))
	// {
	// 	printf("cat: %s: Not a file\n", args);
	// 	return;
	// }

	// vfs::FileHandle *fd = fs->Open(thisNode->FilePath, nullptr, true);

	// uint8_t *buffer = new uint8_t[fd->node->Stat.Size + 1];
	// ssize_t rBytes = fd->read(buffer, fd->node->Stat.Size);
	// if (rBytes > 0)
	// 	printf("%s\n", buffer);
	// else
	// 	printf("cat: %s: Could not read file\n", args);
	// delete[] buffer;
	// delete fd;
}
