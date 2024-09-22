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
		BinaryType Type;

		if (Node == nullptr)
			ReturnLogError((BinaryType)-ENOENT, "Node is null");

		Elf32_Ehdr ELFHeader;
		Node->Read(&ELFHeader, sizeof(Elf32_Ehdr), 0);

		mach_header MachHeader;
		Node->Read(&MachHeader, sizeof(mach_header), 0);

		IMAGE_DOS_HEADER MZHeader;
		Node->Read(&MZHeader, sizeof(IMAGE_DOS_HEADER), 0);

		/* Check ELF header. */
		if (ELFHeader.e_ident[EI_MAG0] == ELFMAG0 &&
			ELFHeader.e_ident[EI_MAG1] == ELFMAG1 &&
			ELFHeader.e_ident[EI_MAG2] == ELFMAG2 &&
			ELFHeader.e_ident[EI_MAG3] == ELFMAG3)
		{
			debug("Image - ELF");
			Type = BinaryType::BinTypeELF;
			goto Success;
		}

		if (MachHeader.magic == MH_MAGIC || MachHeader.magic == MH_CIGAM)
		{
			debug("Image - Mach-O");
			Type = BinaryType::BinTypeMachO;
			goto Success;
		}

		/* Check MZ header. */
		else if (MZHeader.e_magic == IMAGE_DOS_SIGNATURE)
		{
			IMAGE_NT_HEADERS PEHeader;
			Node->Read(&PEHeader, sizeof(IMAGE_NT_HEADERS), MZHeader.e_lfanew);

			IMAGE_OS2_HEADER NEHeader;
			Node->Read(&NEHeader, sizeof(IMAGE_OS2_HEADER), MZHeader.e_lfanew);

			/* TODO: LE, EDOS */
			if (PEHeader.Signature == IMAGE_NT_SIGNATURE)
			{
				debug("Image - PE");
				Type = BinaryType::BinTypePE;
				goto Success;
			}
			else if (NEHeader.ne_magic == IMAGE_OS2_SIGNATURE)
			{
				debug("Image - NE");
				Type = BinaryType::BinTypeNE;
				goto Success;
			}
			else
			{
				debug("Image - MZ");
				Type = BinaryType::BinTypeMZ;
				goto Success;
			}
		}

		/* ... */

		Type = BinaryType::BinTypeUnknown;
	Success:
		return Type;
	}

	BinaryType GetBinaryType(std::string Path)
	{
		FileNode *node = fs->GetByPath(Path.c_str(), nullptr);
		debug("Checking binary type of %s (returning %p)", Path.c_str(), node);
		assert(node != nullptr);
		return GetBinaryType(node);
	}
}
