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

	void ELFLoadRel(void *BaseImage,
						   const char *Name,
						   Tasking::PCB *Process)
	{
#if defined(__amd64__)
		UNUSED(Name);
		debug("Relocatable");
		/* TODO: I have to fully implement this, but for now I will leave it as it is now. */
		warn("Relocatable ELF is not fully supported yet");
		Elf64_Shdr *shdr = GetELFSheader(((Elf64_Ehdr *)BaseImage));
		for (Elf64_Half i = 0; i < ((Elf64_Ehdr *)BaseImage)->e_shnum; i++)
		{
			Elf64_Shdr *Section = &shdr[i];
			if (Section->sh_type == SHT_NOBITS)
			{
				if (!Section->sh_size)
					continue;
				if (Section->sh_flags & SHF_ALLOC)
				{
					void *Buffer = KernelAllocator.RequestPages(TO_PAGES(Section->sh_size + 1));
					memset(Buffer, 0, Section->sh_size);

					Memory::Virtual(Process->PageTable).Map((void *)Buffer, (void *)Buffer, Section->sh_size, Memory::PTFlag::RW | Memory::PTFlag::US);

					Section->sh_offset = (uintptr_t)Buffer - (uintptr_t)BaseImage;
					debug("Section %ld", Section->sh_size);
				}
			}
		}

		for (Elf64_Half i = 0; i < ((Elf64_Ehdr *)BaseImage)->e_shnum; i++)
		{
			Elf64_Shdr *Section = &shdr[i];
			if (Section->sh_type == SHT_REL)
			{
				for (size_t Index = 0; Index < Section->sh_size / Section->sh_entsize; Index++)
				{
					Elf64_Rel *RelTable = &((Elf64_Rel *)((uintptr_t)BaseImage + Section->sh_offset))[Index];
					Elf64_Shdr *Target = GetELFSection(((Elf64_Ehdr *)BaseImage), Section->sh_info);

					uintptr_t *RelAddress = (uintptr_t *)(((uintptr_t)BaseImage + Target->sh_offset) + RelTable->r_offset);
					uint64_t SymbolValue = 0;

					if (ELF64_R_SYM(RelTable->r_info) != SHN_UNDEF)
					{
						SymbolValue = ELFGetSymbolValue(((Elf64_Ehdr *)BaseImage), Section->sh_link, ELF64_R_SYM(RelTable->r_info));
						if (SymbolValue == 0xdead)
							return;
					}

					switch (ELF64_R_TYPE(RelTable->r_info))
					{
					case R_386_NONE:
						break;
					case R_386_32:
						*RelAddress = DO_64_64(SymbolValue, *RelAddress);
						break;
					case R_386_PC32:
						*RelAddress = DO_64_PC32(SymbolValue, *RelAddress, (uintptr_t)RelAddress);
						break;
					default:
					{
						error("Unsupported relocation type: %d", ELF64_R_TYPE(RelTable->r_info));
						return;
					}
					}
					debug("Symbol value: %#lx", SymbolValue);
				}
			}
		}
#elif defined(__i386__)
#endif
	}
}
