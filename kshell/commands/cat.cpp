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

	Node *thisNode = fs->GetNodeFromPath(args, thisProcess->CurrentWorkingDirectory);
	if (thisNode == nullptr)
	{
		printf("cat: %s: No such file or directory\n", args);
		return;
	}

	if (thisNode->Type != NodeType::FILE &&
		thisNode->Type != NodeType::CHARDEVICE)
	{
		printf("cat: %s: Not a file\n", args);
		return;
	}

	int fd = fopen(thisNode->FullPath, "r");
	struct stat st;
	fstat(fd, &st);

	char *buffer = new char[st.st_size + 1];
	ssize_t rBytes = fread(fd, buffer, st.st_size);
	if (rBytes > 0)
		printf("%s\n", buffer);
	else
		printf("cat: %s: Could not read file\n", args);
	delete[] buffer;
	fclose(fd);
}
