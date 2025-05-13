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

#include <fs/vfs.hpp>

#include "../../kernel.h"

using namespace vfs;

const char *ColorNodeType(Node node)
{
	if (node->IsRegularFile())
		return "\x1b[0m";
	else if (node->IsDirectory())
		return "\x1b[1;34m";
	else if (node->IsBlockDevice())
		return "\x1b[1;33m";
	else if (node->IsCharacterDevice())
		return "\x1b[1;33m";
	else if (node->IsFIFO())
		return "\x1b[0;33m";
	else if (node->IsSymbolicLink())
		return "\x1b[1;36m";
	else
		return "\x1b[0m";
}

__no_sanitize("alignment") void PrintLS(Node node)
{
	size_t maxNameLength = 0;
	int count = 0;
	bool first = true;

	std::list<Node> children = fs->ReadDirectory(node);

	for (auto &&i : children)
		std::max(maxNameLength, i->Name.length());

	for (auto &&i : children)
	{
		if (count % 5 == 0 && !first)
			printf("\n");

		printf(" %s%-*s ", ColorNodeType(i), (int)maxNameLength, i->Name.c_str());

		count++;
		first = false;
	}

	printf("\x1b[0m\n");
}

void cmd_ls(const char *args)
{
	if (args[0] == '\0')
	{
		Node rootNode = thisProcess->CWD;

		if (rootNode == nullptr)
			rootNode = fs->GetRoot(0);

		PrintLS(rootNode);
		return;
	}

	Node thisNode = fs->Lookup(thisProcess->CWD, args);

	if (thisNode == nullptr)
	{
		printf("ls: %s: No such file or directory\n", args);
		return;
	}

	if (!thisNode->IsDirectory())
	{
		printf("%s%s\n", ColorNodeType(thisNode), thisNode->Path.c_str());
		return;
	}

	PrintLS(thisNode);
}
