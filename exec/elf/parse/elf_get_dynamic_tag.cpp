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
	std::vector<Elf64_Dyn> ELFGetDynamicTag_x86_64(int fd,
												   DynamicArrayTags Tag)
	{
#if defined(a64) || defined(aa64)
		off_t OldOffset = lseek(fd, 0, SEEK_CUR);
		std::vector<Elf64_Dyn> Ret;

		Elf64_Ehdr ELFHeader;
		lseek(fd, 0, SEEK_SET);
		fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		std::vector<Elf64_Phdr> DYNAMICPhdrs = ELFGetSymbolType_x86_64(fd, PT_DYNAMIC);

		if (DYNAMICPhdrs.size() < 1)
		{
			error("No dynamic phdrs found.");
			return Ret;
		}

		foreach (auto Phdr in DYNAMICPhdrs)
		{
			Elf64_Dyn Dynamic;
			for (size_t i = 0; i < Phdr.p_filesz / sizeof(Elf64_Dyn); i++)
			{
				lseek(fd, Phdr.p_offset + (i * sizeof(Elf64_Dyn)), SEEK_SET);
				fread(fd, (uint8_t *)&Dynamic, sizeof(Elf64_Dyn));

				if (Dynamic.d_tag != Tag)
					continue;

				debug("Found dynamic tag %d at %#lx [d_val: %#lx]",
					  Tag, &Dynamic, Dynamic.d_un.d_val);
				Ret.push_back(Dynamic);
			}
		}

		lseek(fd, OldOffset, SEEK_SET);
		return Ret;
#elif defined(a32)
		return {};
#endif
	}
}
