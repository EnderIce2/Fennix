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

void tree_loop(Node *rootNode, int depth = 0)
{
	foreach (auto Child in rootNode->Children)
	{
		Display->UpdateBuffer();
		if (Child->Type == NodeType::DIRECTORY ||
			Child->Type == NodeType::MOUNTPOINT)
		{
			printf("%*s%*s%*s|- %s\n",
				   depth, "",
				   depth, "",
				   depth, "",
				   Child->Name);
			tree_loop(Child, depth + 1);
		}
		else
			printf("%*s%*s%*s|- %s\n",
				   depth, "",
				   depth, "",
				   depth, "",
				   Child->Name);
	}
}

void cmd_tree(const char *args)
{
	Node *rootNode = thisProcess->CurrentWorkingDirectory;
	if (args[0] == '\0')
	{
		if (rootNode == nullptr)
			rootNode = fs->GetRootNode()->Children[0];
	}
	else
	{
		rootNode = fs->GetNodeFromPath(args, thisProcess->CurrentWorkingDirectory);
		if (rootNode == nullptr)
		{
			printf("ls: %s: No such file or directory\n", args);
			return;
		}
		if (rootNode->Type != NodeType::DIRECTORY)
		{
			printf("%s\n", rootNode->Name);
			return;
		}
	}

	printf("%s\n", rootNode->Name);
	tree_loop(rootNode);
}
