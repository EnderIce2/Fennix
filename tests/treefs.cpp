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

#ifdef DEBUG

#include "t.h"

#include "../kernel.h"

void TreeFS(vfs::Node *node, int Depth)
{
	return;
	foreach (auto Chld in node->Children)
	{
		printf("%*c %s\eFFFFFF\n", Depth, ' ', Chld->Name);

		if (!Config.Quiet)
			Display->SetBuffer(0);
		TaskManager->Sleep(100);
		TreeFS(Chld, Depth + 1);
	}
}

#endif // DEBUG
