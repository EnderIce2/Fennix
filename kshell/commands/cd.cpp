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

void cmd_cd(const char *args)
{
	if (args[0] == '\0')
		return;

	/* FIXME: Reimplement this later */
	assert(!"Function not implemented");
	// Node *thisNode = fs->GetByPath(args, thisProcess->CWD, true);

	// if (thisNode == nullptr)
	// {
	// 	printf("cd: %s: No such file or directory\n", args);
	// 	return;
	// }

	// if (thisNode->Stat.IsType(SYMLINK))
	// 	thisNode = fs->GetByPath(thisNode->GetSymLink(), nullptr, true);

	// if (!thisNode->Stat.IsType(DIRECTORY))
	// {
	// 	printf("cd: %s: Not a directory\n", args);
	// 	return;
	// }

	// thisProcess->CWD = thisNode;
}
