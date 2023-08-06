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

void cmd_ls(const char *args)
{
	if (args[0] == '\0')
	{
		Node *rootNode = thisProcess->CurrentWorkingDirectory;

		if (rootNode == nullptr)
			rootNode = vfs->GetRootNode()->Children[0];

		foreach (auto var in rootNode->Children)
			printf("%s\n", var->Name);
	}
	else
	{
		Node *thisNode = vfs->GetNodeFromPath(args, thisProcess->CurrentWorkingDirectory);

		if (thisNode == nullptr)
		{
			printf("ls: %s: No such file or directory\n", args);
			return;
		}

		if (thisNode->Flags != NodeFlags::DIRECTORY)
		{
			printf("%s\n", thisNode->Name);
			return;
		}

		foreach (auto var in thisNode->Children)
			printf("%s\n", var->Name);
	}
}
