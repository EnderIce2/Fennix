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

#include "../../../kernel.h"

namespace Execute
{
	std::vector<Elf64_Phdr> ELFGetSymbolType_x86_64(int fd,
													SegmentTypes Tag)
	{
#if defined(a64) || defined(aa64)
		off_t OldOffset = lseek(fd, 0, SEEK_CUR);
		std::vector<Elf64_Phdr> Ret;

		Elf64_Ehdr ELFHeader;
		lseek(fd, 0, SEEK_SET);
		fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		Elf64_Phdr ProgramHeaders;
		lseek(fd, ELFHeader.e_phoff, SEEK_SET);
		fread(fd, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));

		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			if (ProgramHeaders.p_type == Tag)
				Ret.push_back(ProgramHeaders);

			lseek(fd, sizeof(Elf64_Phdr), SEEK_CUR);
			fread(fd, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));
		}

		lseek(fd, OldOffset, SEEK_SET);
		return Ret;
#elif defined(a32)
		return {};
#endif
	}
}
