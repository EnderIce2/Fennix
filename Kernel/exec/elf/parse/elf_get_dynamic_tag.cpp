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
	std::vector<Elf64_Dyn> ELFGetDynamicTag_x86_64(FileNode *fd,
												   DynamicArrayTags Tag)
	{
#if defined(__amd64__) || defined(__aarch64__)
		std::vector<Elf64_Dyn> Ret;

		Elf64_Ehdr ELFHeader{};
		fd->Read(&ELFHeader, sizeof(Elf64_Ehdr), 0);

		std::vector<Elf64_Phdr> DYNAMICPhdrs = ELFGetSymbolType_x86_64(fd, PT_DYNAMIC);

		if (DYNAMICPhdrs.size() < 1)
		{
			error("No dynamic phdrs found.");
			return Ret;
		}

		for (auto Phdr : DYNAMICPhdrs)
		{
			Elf64_Dyn Dynamic{};
			for (size_t i = 0; i < Phdr.p_filesz / sizeof(Elf64_Dyn); i++)
			{
				fd->Read(&Dynamic, sizeof(Elf64_Dyn), Phdr.p_offset + (i * sizeof(Elf64_Dyn)));

				if (Dynamic.d_tag != Tag)
					continue;

				debug("Found dynamic tag %d at %#lx [d_val: %#lx]",
					  Tag, &Dynamic, Dynamic.d_un.d_val);
				Ret.push_back(Dynamic);
			}
		}

		return Ret;
#elif defined(__i386__)
		return {};
#endif
	}
}
