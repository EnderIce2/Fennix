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

void tree_loop(FileNode *rootNode, int depth = 0)
{
	// for (auto Child : rootNode->GetChildren(true))
	// {
	// 	Display->UpdateBuffer();
	// 	if (Child->Stat.IsType(DIRECTORY) || Child->Stat.IsType(MOUNTPOINT))
	// 	{
	// 		printf("%*s%*s%*s|- %s\n",
	// 			   depth, "",
	// 			   depth, "",
	// 			   depth, "",
	// 			   Child->FileName);
	// 		tree_loop(Child, depth + 1);
	// 	}
	// 	else
	// 		printf("%*s%*s%*s|- %s\n",
	// 			   depth, "",
	// 			   depth, "",
	// 			   depth, "",
	// 			   Child->FileName);
	// }

	kdirent *dirBuffer = new kdirent[16];
	ssize_t read = 0;
	off_t offset = 0;
	while ((read = rootNode->ReadDir(dirBuffer, sizeof(kdirent) * 16, offset, LONG_MAX)) > 0)
	{
		if (read / sizeof(kdirent) == 0)
			break;

		off_t bufOffset = 0;
		for (size_t i = 0; i < read / sizeof(kdirent); i++)
		{
			kdirent *dirent = (kdirent *)((uintptr_t)dirBuffer + bufOffset);
			if (dirent->d_reclen == 0)
				break;
			bufOffset += dirent->d_reclen;

			if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
				continue;

			FileNode *node = fs->GetByPath(dirent->d_name, rootNode);
			if (node == nullptr)
				continue;

			for (int i = 0; i < depth; i++)
				printf("  ");
			printf("|- %s\n", dirent->d_name);

			if (node->IsDirectory())
				tree_loop(node, depth + 1);
		}
		offset += read;
	}
	delete[] dirBuffer;
}

void cmd_tree(const char *args)
{
	FileNode *rootNode = thisProcess->CWD;
	if (args[0] == '\0')
	{
		if (rootNode == nullptr)
			rootNode = fs->GetRoot(0);
	}
	else
	{
		rootNode = fs->GetByPath(args, nullptr);
		if (rootNode == nullptr)
		{
			printf("ls: %s: No such file or directory\n", args);
			return;
		}
	}

	printf("%s\n", rootNode->Name.c_str());
	tree_loop(rootNode);
}
