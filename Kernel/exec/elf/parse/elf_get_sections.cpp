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
	std::vector<Elf_Shdr> ELFGetSections(FileNode *fd, const char *SectionName)
	{
		std::vector<Elf_Shdr> ret;

		Elf_Ehdr ehdr{};
		fd->Read(&ehdr, sizeof(Elf_Ehdr), 0);

		Elf_Shdr *sections = new Elf_Shdr[ehdr.e_shnum];
		fd->Read(sections, sizeof(Elf_Shdr) * ehdr.e_shnum, ehdr.e_shoff);

		char *sectionNames = new char[sections[ehdr.e_shstrndx].sh_size];
		fd->Read(sectionNames, sections[ehdr.e_shstrndx].sh_size, sections[ehdr.e_shstrndx].sh_offset);

		for (Elf_Half i = 0; i < ehdr.e_shnum; ++i)
		{
			const char *Name = sectionNames + sections[i].sh_name;
			if (strcmp(Name, SectionName) == 0)
				ret.push_back(sections[i]);
		}

		delete[] sections;
		delete[] sectionNames;
		return ret;
	}
}
