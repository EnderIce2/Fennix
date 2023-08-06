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
	std::vector<Elf64_Shdr> ELFGetSections_x86_64(int fd,
												  const char *SectionName)
	{
#if defined(a64) || defined(aa64)
		off_t OldOffset = lseek(fd, 0, SEEK_CUR);
		std::vector<Elf64_Shdr> Ret;

		Elf64_Ehdr ELFHeader;
		lseek(fd, 0, SEEK_SET);
		fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		Elf64_Shdr *SectionHeaders = new Elf64_Shdr[ELFHeader.e_shnum];
		lseek(fd, ELFHeader.e_shoff, SEEK_SET);
		fread(fd, (uint8_t *)SectionHeaders, sizeof(Elf64_Shdr) * ELFHeader.e_shnum);

		char *SectionNames = new char[SectionHeaders[ELFHeader.e_shstrndx].sh_size];
		lseek(fd, SectionHeaders[ELFHeader.e_shstrndx].sh_offset, SEEK_SET);
		fread(fd, (uint8_t *)SectionNames, SectionHeaders[ELFHeader.e_shstrndx].sh_size);

		for (Elf64_Half i = 0; i < ELFHeader.e_shnum; ++i)
		{
			const char *Name = SectionNames + SectionHeaders[i].sh_name;
			if (strcmp(Name, SectionName) == 0)
				Ret.push_back(SectionHeaders[i]);
		}

		lseek(fd, OldOffset, SEEK_SET);
		delete[] SectionHeaders;
		delete[] SectionNames;
		return Ret;
#elif defined(a32)
		return {};
#endif
	}
}
