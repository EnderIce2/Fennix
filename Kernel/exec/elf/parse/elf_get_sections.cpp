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
	std::vector<Elf64_Shdr> ELFGetSections_x86_64(FileNode *fd,
												  const char *SectionName)
	{
#if defined(__amd64__) || defined(__aarch64__)
		std::vector<Elf64_Shdr> Ret;

		Elf64_Ehdr ELFHeader{};
		fd->Read(&ELFHeader, sizeof(Elf64_Ehdr), 0);

		Elf64_Shdr *SectionHeaders = new Elf64_Shdr[ELFHeader.e_shnum];
		fd->Read(SectionHeaders, sizeof(Elf64_Shdr) * ELFHeader.e_shnum, ELFHeader.e_shoff);

		char *SectionNames = new char[SectionHeaders[ELFHeader.e_shstrndx].sh_size];
		fd->Read(SectionNames, SectionHeaders[ELFHeader.e_shstrndx].sh_size, SectionHeaders[ELFHeader.e_shstrndx].sh_offset);

		for (Elf64_Half i = 0; i < ELFHeader.e_shnum; ++i)
		{
			const char *Name = SectionNames + SectionHeaders[i].sh_name;
			if (strcmp(Name, SectionName) == 0)
				Ret.push_back(SectionHeaders[i]);
		}

		delete[] SectionHeaders;
		delete[] SectionNames;
		return Ret;
#elif defined(__i386__)
		return {};
#endif
	}
}
