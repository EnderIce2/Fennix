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
	std::vector<Elf64_Phdr> ELFGetSymbolType_x86_64(FileNode *fd,
													SegmentTypes Tag)
	{
#if defined(__amd64__) || defined(__aarch64__)
		std::vector<Elf64_Phdr> Ret;

		Elf64_Ehdr ELFHeader{};
		fd->Read(&ELFHeader, sizeof(Elf64_Ehdr), 0);

		Elf64_Phdr ProgramHeaders{};
		fd->Read(&ProgramHeaders, sizeof(Elf64_Phdr), ELFHeader.e_phoff);

		off_t currentOffset = ELFHeader.e_phoff;
		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			if (ProgramHeaders.p_type == Tag)
				Ret.push_back(ProgramHeaders);

			currentOffset += sizeof(Elf64_Phdr);
			fd->Read(&ProgramHeaders, sizeof(Elf64_Phdr), currentOffset);
		}

		return Ret;
#elif defined(__i386__)
		return {};
#endif
	}
}
