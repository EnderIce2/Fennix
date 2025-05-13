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

// Enhancing the tree_loop function to display a fancier tree structure

__no_sanitize("alignment") void tree_loop(Node rootNode, int depth = 0, std::string prefix = "")
{
    std::list<Node> children = fs->ReadDirectory(rootNode);
    size_t count = children.size();
    size_t index = 0;

    for (auto &&child : children)
    {
        if (child->Name == "." || child->Name == "..")
            continue;

        bool isLast = (++index == count);

        printf("%s%s- %s\n", prefix.c_str(), isLast ? "\\" : "|-", child->Name.c_str());

        if (child->IsDirectory())
        {
            std::string newPrefix = prefix + (isLast ? "  " : "| ");
            tree_loop(child, depth + 1, newPrefix);
        }
    }
}

void cmd_tree(const char *args)
{
	Node rootNode = thisProcess->CWD;
	if (args[0] == '\0')
	{
		if (rootNode == nullptr)
			rootNode = fs->GetRoot(0);
	}
	else
	{
		rootNode = fs->Lookup(thisProcess->CWD, args);
		if (rootNode == nullptr)
		{
			printf("ls: %s: No such file or directory\n", args);
			return;
		}
	}

	printf("%s\n", rootNode->Name.c_str());
	tree_loop(rootNode);
}
