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

#include <filesystem.hpp>
#include <errno.h>

#include "../../kernel.h"

using namespace VirtualFileSystem;

ReadFSFunction(Zero_Read)
{
	if (Size <= 0)
		return 0;

	memset(Buffer, 0, Size);
	return Size;
}

ReadFSFunction(Zero_Write)
{
	return Size;
}

FileSystemOperations zero_op = {
	.Name = "Zero",
	.Read = Zero_Read,
	.Write = Zero_Write,
};

void Init_Zero(Virtual *vfs_ctx)
{
	Node *n = vfs_ctx->Create("zero", CHARDEVICE, DevFS);
	n->Operator = &zero_op;
}
