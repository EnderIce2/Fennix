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

const char *ColorNodeType(Node *node)
{
	switch (node->Type)
	{
	case NodeType::DIRECTORY:
		return "\e3871F5";
	case NodeType::BLOCKDEVICE:
		return "\eE8CD1E";
	case NodeType::CHARDEVICE:
		return "\e86E01F";
	case NodeType::PIPE:
		return "\eE0991F";
	case NodeType::SYMLINK:
		return "\e1FB9E0";
	case NodeType::FILE:
		return "\eCCCCCC";
	default:
		return "\eF72020";
	}
}

size_t MaxNameLength(Node *nodes)
{
	size_t maxLength = 0;
	foreach (auto &node in nodes->Children)
		maxLength = std::max(maxLength, strlen(node->Name));
	return maxLength;
}

void PrintLS(Node *node)
{
	size_t maxNameLength = MaxNameLength(node);
	int count = 0;
	bool first = true;
	foreach (auto &var in node->Children)
	{
		if (count % 5 == 0 && !first)
			printf("\n");
		printf(" %s%-*s ", ColorNodeType(var), (int)maxNameLength, var->Name);
		count++;
		first = false;
	}
	printf("\eCCCCCC\n");
}

void cmd_ls(const char *args)
{
	if (args[0] == '\0')
	{
		Node *rootNode = thisProcess->CurrentWorkingDirectory;

		if (rootNode == nullptr)
			rootNode = fs->GetRootNode()->Children[0];

		PrintLS(rootNode);
	}
	else
	{
		Node *thisNode = fs->GetNodeFromPath(args, thisProcess->CurrentWorkingDirectory);

		if (thisNode == nullptr)
		{
			printf("ls: %s: No such file or directory\n", args);
			return;
		}

		if (thisNode->Type == NodeType::SYMLINK)
			thisNode = fs->GetNodeFromPath(thisNode->Symlink);

		if (thisNode->Type != NodeType::DIRECTORY)
		{
			printf("%s%s\n", ColorNodeType(thisNode), thisNode->Name);
			return;
		}

		PrintLS(thisNode);
	}
}
