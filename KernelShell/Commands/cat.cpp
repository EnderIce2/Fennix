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

using namespace VirtualFileSystem;

void cmd_cat(const char *args)
{
	if (args[0] == '\0')
		return;

	Node *thisNode = vfs->GetNodeFromPath(args, thisProcess->CurrentWorkingDirectory);
	if (thisNode == nullptr)
	{
		printf("cat: %s: No such file or directory\n", args);
		return;
	}

	if (thisNode->Flags != NodeFlags::FILE &&
		thisNode->Flags != NodeFlags::CHARDEVICE)
	{
		printf("cat: %s: Not a file\n", args);
		return;
	}
	std::string path = vfs->GetPathFromNode(thisNode);

	int fd = fopen(path.c_str(), "r");
	struct stat st;
	fstat(fd, &st);

	char *buffer = new char[st.st_size + 1];
	fread(fd, buffer, st.st_size);
	printf("%s\n", buffer);
	delete[] buffer;
	fclose(fd);
}
