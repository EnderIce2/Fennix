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

#include "../kernel.h"
#include "../Fex.hpp"

namespace Execute
{
	BinaryType GetBinaryType(const char *Path)
	{
		BinaryType Type;
		int fd = fopen(Path, "r");

		if (fd < 0)
			return (BinaryType)fd;

		debug("File opened: %s, descriptor %d", Path, fd);
		Memory::SmartHeap sh = Memory::SmartHeap(1024);
		fread(fd, sh, 128);

		Fex *FexHdr = (Fex *)sh.GetObject();
		Elf32_Ehdr *ELFHeader = (Elf32_Ehdr *)sh.GetObject();
		IMAGE_DOS_HEADER *MZHeader = (IMAGE_DOS_HEADER *)sh.GetObject();

		/* Check Fex header. */
		if (FexHdr->Magic[0] == 'F' &&
			FexHdr->Magic[1] == 'E' &&
			FexHdr->Magic[2] == 'X' &&
			FexHdr->Magic[3] == '\0')
		{
			/* If the fex type is driver, we shouldn't return as Fex. */
			if (FexHdr->Type == FexFormatType_Executable)
			{
				debug("Image - Fex");
				Type = BinaryType::BinTypeFex;
				goto Success;
			}
			else if (FexHdr->Type == FexFormatType_Driver)
			{
				fixme("Fex Driver is not supposed to be executed.");
				/* TODO: Driver installation pop-up. */
			}
		}

		/* Check ELF header. */
		else if (ELFHeader->e_ident[EI_MAG0] == ELFMAG0 &&
				 ELFHeader->e_ident[EI_MAG1] == ELFMAG1 &&
				 ELFHeader->e_ident[EI_MAG2] == ELFMAG2 &&
				 ELFHeader->e_ident[EI_MAG3] == ELFMAG3)
		{
			debug("Image - ELF");
			Type = BinaryType::BinTypeELF;
			goto Success;
		}

		/* Check MZ header. */
		else if (MZHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			lseek(fd, MZHeader->e_lfanew, SEEK_SET);
			fread(fd, sh, 512);
			IMAGE_NT_HEADERS *PEHeader =
				(IMAGE_NT_HEADERS *)(((char *)sh.GetObject()) +
									 MZHeader->e_lfanew);

			IMAGE_OS2_HEADER *NEHeader =
				(IMAGE_OS2_HEADER *)(((char *)sh.GetObject()) +
									 MZHeader->e_lfanew);

			/* TODO: LE, EDOS */
			if (PEHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				debug("Image - PE");
				Type = BinaryType::BinTypePE;
				goto Success;
			}
			else if (NEHeader->ne_magic == IMAGE_OS2_SIGNATURE)
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
		fclose(fd);
		return Type;
	}
}
