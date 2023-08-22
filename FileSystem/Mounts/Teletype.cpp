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

ReadFSFunction(tty_Write)
{
	for (size_t i = 0; i < Size; i++)
		putchar(((char *)Buffer)[i]);

	Display->SetBuffer(0);
	return Size;
}

FileSystemOperations tty_op = {
	.Name = "tty",
	.Write = tty_Write,
};

void Init_Teletype(Virtual *vfs_ctx)
{
	Node *n = vfs_ctx->Create("tty", CHARDEVICE, DevFS);
	n->Operator = &tty_op;
}
