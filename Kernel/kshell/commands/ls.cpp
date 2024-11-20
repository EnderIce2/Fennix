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

const char *ColorNodeType(FileNode *node)
{
	if (node->IsRegularFile())
		return "\x1b[32m";
	else if (node->IsDirectory())
		return "\x1b[34m";
	else if (node->IsBlockDevice())
		return "\x1b[33m";
	else if (node->IsCharacterDevice())
		return "\x1b[33m";
	else if (node->IsFIFO())
		return "\x1b[33m";
	else if (node->IsSymbolicLink())
		return "\x1b[35m";
	else
		return "\x1b[0m";
}

__no_sanitize("alignment") size_t MaxNameLength(FileNode *nodes)
{
	size_t maxLength = 0;

	kdirent *dirBuffer = new kdirent[16];
	ssize_t read = 0;
	off_t offset = 0;
	while ((read = nodes->ReadDir(dirBuffer, sizeof(kdirent) * 16, offset, LONG_MAX)) > 0)
	{
		if (read / sizeof(kdirent) == 0)
			break;

		off_t bufOffset = 0;
		debug("There are %ld entries in this directory", read / sizeof(kdirent));
		for (size_t i = 0; i < read / sizeof(kdirent); i++)
		{
			kdirent *dirent = (kdirent *)((uintptr_t)dirBuffer + bufOffset);
			if (dirent->d_reclen == 0)
				break;
			bufOffset += dirent->d_reclen;
			maxLength = std::max(maxLength, strlen(dirent->d_name));
			debug("dirent->d_name: %s (max length: %ld)", dirent->d_name, maxLength);
		}
		offset += read / sizeof(kdirent);
	}
	delete[] dirBuffer;
	return maxLength;
}

__no_sanitize("alignment") void PrintLS(FileNode *node)
{
	size_t maxNameLength = MaxNameLength(node);
	int count = 0;
	bool first = true;

	kdirent *dirBuffer = new kdirent[16];
	ssize_t read = 0;
	off_t offset = 0;
	while ((read = node->ReadDir(dirBuffer, sizeof(kdirent) * 16, offset, LONG_MAX)) > 0)
	{
		if (read / sizeof(kdirent) == 0)
			break;

		off_t bufOffset = 0;
		for (size_t i = 0; i < read / sizeof(kdirent); i++)
		{
			if (count % 5 == 0 && !first)
				printf("\n");
			kdirent *dirent = (kdirent *)((uintptr_t)dirBuffer + bufOffset);
			if (dirent->d_reclen == 0)
				break;
			bufOffset += dirent->d_reclen;
			printf(" %s%-*s ", ColorNodeType(node), (int)maxNameLength, dirent->d_name);
			count++;
			first = false;
		}
		offset += read / sizeof(kdirent);
	}

	printf("\x1b[0m\n");
	delete[] dirBuffer;
}

void cmd_ls(const char *args)
{
	if (args[0] == '\0')
	{
		FileNode *rootNode = thisProcess->CWD;

		if (rootNode == nullptr)
			rootNode = fs->GetRoot(0);

		PrintLS(rootNode);
		return;
	}

	FileNode *thisNode = fs->GetByPath(args, nullptr);

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
