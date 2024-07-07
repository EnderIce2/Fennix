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

	FileNode *node = fs->GetByPath(args, nullptr);

	if (node == nullptr)
	{
		printf("cat: %s: No such file or directory\n", args);
		return;
	}

	if (!node->IsRegularFile() && !node->IsCharacterDevice())
	{
		printf("cat: %s: Not a regular file or character device\n", args);
		return;
	}

	if (node->IsCharacterDevice())
	{
		printf("cat: %s: Character devices are not supported yet\n", args);
		return;
	}

	kstat stat = {};
	node->Stat(&stat);

	uint8_t *buffer = new uint8_t[stat.Size + 1];
	ssize_t rBytes = node->Read(buffer, stat.Size, 0);
	if (rBytes > 0)
		printf("%s\n", buffer);
	else
		printf("cat: %s: Could not read file\n", args);
	delete[] buffer;
}
