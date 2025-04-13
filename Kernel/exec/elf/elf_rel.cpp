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

#include "../../kernel.h"

namespace Execute
{
	/* Originally from https://wiki.osdev.org/ELF_Tutorial */

	void ELFLoadRel(void *BaseImage, const char *Name, Tasking::PCB *Process)
	{
		UNUSED(Name);
		debug("Relocatable");
		/* TODO: I have to fully implement this, but for now I will leave it as it is now. */
		warn("Relocatable ELF is not fully supported yet");
		Elf_Shdr *shdr = GetELFSheader(((Elf_Ehdr *)BaseImage));
		for (Elf_Half i = 0; i < ((Elf_Ehdr *)BaseImage)->e_shnum; i++)
		{
			Elf_Shdr *section = &shdr[i];
			if (section->sh_type == SHT_NOBITS)
			{
				if (!section->sh_size)
					continue;
				if (section->sh_flags & SHF_ALLOC)
				{
					void *buffer = KernelAllocator.RequestPages(TO_PAGES(section->sh_size + 1));
					memset(buffer, 0, section->sh_size);

					Memory::Virtual(Process->PageTable).Map((void *)buffer, (void *)buffer, section->sh_size, Memory::PTFlag::RW | Memory::PTFlag::US);

					section->sh_offset = (uintptr_t)buffer - (uintptr_t)BaseImage;
					debug("Section %ld", section->sh_size);
				}
			}
		}

		for (Elf_Half i = 0; i < ((Elf_Ehdr *)BaseImage)->e_shnum; i++)
		{
			Elf_Shdr *section = &shdr[i];
			if (section->sh_type == SHT_REL)
			{
				for (size_t i = 0; i < section->sh_size / section->sh_entsize; i++)
				{
					Elf_Rel *rel = &((Elf_Rel *)((uintptr_t)BaseImage + section->sh_offset))[i];
					Elf_Shdr *target = GetELFSection(((Elf_Ehdr *)BaseImage), section->sh_info);

					uintptr_t *relPtr = (uintptr_t *)(((uintptr_t)BaseImage + target->sh_offset) + rel->r_offset);
					uintptr_t value = 0;

					if (ELF_R_SYM(rel->r_info) != SHN_UNDEF)
					{
						value = ELFGetSymbolValue(((Elf_Ehdr *)BaseImage), section->sh_link, ELF_R_SYM(rel->r_info));
						if (value == (uintptr_t)-1)
							return;
					}

					switch (ELF64_R_TYPE(rel->r_info))
					{
					case R_386_NONE:
						break;
					case R_386_32:
						*relPtr = DO_64_64(value, *relPtr);
						break;
					case R_386_PC32:
						*relPtr = DO_64_PC32(value, *relPtr, (uintptr_t)relPtr);
						break;
					default:
					{
						error("Unsupported relocation type: %d", ELF64_R_TYPE(rel->r_info));
						return;
					}
					}
					debug("Symbol value: %#lx", value);
				}
			}
		}
	}
}
