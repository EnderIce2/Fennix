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

#include <exec.hpp>

#include <msexec.h>
#include <macho.h>
#include <memory>

#include "../kernel.h"

namespace Execute
{
	BinaryType GetBinaryType(FileNode *Node)
	{
		debug("Checking binary type of %s", Node->Path.c_str());
		BinaryType type;

		if (Node == nullptr)
			ReturnLogError((BinaryType)-ENOENT, "Node is null");

		Elf_Ehdr ehdr;
		Node->Read(&ehdr, sizeof(Elf_Ehdr), 0);

		mach_header mach;
		Node->Read(&mach, sizeof(mach_header), 0);

		IMAGE_DOS_HEADER mz;
		Node->Read(&mz, sizeof(IMAGE_DOS_HEADER), 0);

		/* Check ELF header. */
		if (ehdr.e_ident[EI_MAG0] == ELFMAG0 &&
			ehdr.e_ident[EI_MAG1] == ELFMAG1 &&
			ehdr.e_ident[EI_MAG2] == ELFMAG2 &&
			ehdr.e_ident[EI_MAG3] == ELFMAG3)
		{
			debug("Image - ELF");
			type = BinaryType::BinTypeELF;
			goto Success;
		}

		if (mach.magic == MH_MAGIC || mach.magic == MH_CIGAM)
		{
			debug("Image - Mach-O");
			type = BinaryType::BinTypeMachO;
			goto Success;
		}

		/* Check MZ header. */
		else if (mz.e_magic == IMAGE_DOS_SIGNATURE)
		{
			IMAGE_NT_HEADERS pe;
			Node->Read(&pe, sizeof(IMAGE_NT_HEADERS), mz.e_lfanew);

			IMAGE_OS2_HEADER ne;
			Node->Read(&ne, sizeof(IMAGE_OS2_HEADER), mz.e_lfanew);

			/* TODO: LE, EDOS */
			if (pe.Signature == IMAGE_NT_SIGNATURE)
			{
				debug("Image - PE");
				type = BinaryType::BinTypePE;
				goto Success;
			}
			else if (ne.ne_magic == IMAGE_OS2_SIGNATURE)
			{
				debug("Image - NE");
				type = BinaryType::BinTypeNE;
				goto Success;
			}
			else
			{
				debug("Image - MZ");
				type = BinaryType::BinTypeMZ;
				goto Success;
			}
		}

		/* ... */

		type = BinaryType::BinTypeUnknown;
	Success:
		return type;
	}

	BinaryType GetBinaryType(std::string Path)
	{
		FileNode *node = fs->GetByPath(Path.c_str(), nullptr);
		if (node->IsSymbolicLink())
		{
			char buffer[512];
			node->ReadLink(buffer, sizeof(buffer));
			node = fs->GetByPath(buffer, node->Parent);
		}
		debug("Checking binary type of %s (returning %p)", Path.c_str(), node);
		assert(node != nullptr);
		return GetBinaryType(node);
	}
}
