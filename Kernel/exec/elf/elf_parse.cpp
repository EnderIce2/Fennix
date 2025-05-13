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
	bool ELFIs64(void *Header)
	{
		Elf_Ehdr *ehdr = (Elf_Ehdr *)Header;
		if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
			return true;
		return false;
	}

	/* Originally from https://wiki.osdev.org/ELF_Tutorial */

	Elf_Shdr *GetELFSheader(Elf_Ehdr *Header)
	{
		return (Elf_Shdr *)((uintptr_t)Header + Header->e_shoff);
	}

	Elf_Shdr *GetELFSection(Elf_Ehdr *Header, uintptr_t Index)
	{
		return &GetELFSheader(Header)[Index];
	}

	char *GetELFStringTable(Elf_Ehdr *Header)
	{
		if (Header->e_shstrndx == SHN_UNDEF)
			return nullptr;
		return (char *)Header + GetELFSection(Header, Header->e_shstrndx)->sh_offset;
	}

	char *ELFLookupString(Elf_Ehdr *Header, uintptr_t Offset)
	{
		char *table = GetELFStringTable(Header);
		if (table == nullptr)
			return nullptr;
		return table + Offset;
	}

	Elf_Sym *ELFLookupSymbol(Elf_Ehdr *Header, std::string Name)
	{
		Elf_Shdr *symTable = nullptr;
		Elf_Shdr *stringTable = nullptr;

		for (Elf_Half i = 0; i < Header->e_shnum; i++)
		{
			Elf_Shdr *shdr = GetELFSection(Header, i);
			switch (shdr->sh_type)
			{
			case SHT_SYMTAB:
				symTable = shdr;
				stringTable = GetELFSection(Header, shdr->sh_link);
				break;
			default:
			{
				break;
			}
			}
		}

		if (symTable == nullptr || stringTable == nullptr)
			return nullptr;

		for (size_t i = 0; i < (symTable->sh_size / sizeof(Elf_Sym)); i++)
		{
			Elf_Sym *sym = (Elf_Sym *)((uintptr_t)Header + symTable->sh_offset + (i * sizeof(Elf_Sym)));
			char *String = (char *)((uintptr_t)Header + stringTable->sh_offset + sym->st_name);
			if (strcmp(String, Name.c_str()) == 0)
				return sym;
		}
		return nullptr;
	}

	Elf_Sym ELFLookupSymbol(Node fd, std::string Name)
	{
		Elf_Ehdr ehdr{};
		fs->Read(fd, &ehdr, sizeof(Elf_Ehdr), 0);

		Elf_Shdr symTable{};
		Elf_Shdr stringTable{};

		for (Elf64_Half i = 0; i < ehdr.e_shnum; i++)
		{
			Elf_Shdr shdr;
			fs->Read(fd, &shdr, sizeof(Elf_Shdr), ehdr.e_shoff + (i * sizeof(Elf_Shdr)));

			switch (shdr.sh_type)
			{
			case SHT_SYMTAB:
				symTable = shdr;
				fs->Read(fd, &stringTable, sizeof(Elf_Shdr), ehdr.e_shoff + (shdr.sh_link * sizeof(Elf_Shdr)));
				break;
			default:
				break;
			}
		}

		if (symTable.sh_name == 0 || stringTable.sh_name == 0)
		{
			error("Symbol table not found.");
			return {};
		}

		for (size_t i = 0; i < (symTable.sh_size / sizeof(Elf_Sym)); i++)
		{
			// Elf_Sym *sym = (Elf_Sym *)((uintptr_t)Header + symTable->sh_offset + (i * sizeof(Elf_Sym)));
			Elf_Sym sym;
			fs->Read(fd, &sym, sizeof(Elf_Sym), symTable.sh_offset + (i * sizeof(Elf_Sym)));

			// char *str = (char *)((uintptr_t)Header + stringTable->sh_offset + sym->st_name);
			char str[256];
			fs->Read(fd, &str, sizeof(str), stringTable.sh_offset + sym.st_name);

			if (strcmp(str, Name.c_str()) == 0)
				return sym;
		}
		error("Symbol not found.");
		return {};
	}

	uintptr_t ELFGetSymbolValue(Elf_Ehdr *Header, uintptr_t Table, uintptr_t Index)
	{
		if (Table == SHN_UNDEF || Index == SHN_UNDEF)
			return 0;

		Elf_Shdr *symTable = GetELFSection(Header, Table);

		uintptr_t entries = symTable->sh_size / symTable->sh_entsize;
		if (Index >= entries)
		{
			error("Symbol index out of range %d-%u.", Table, Index);
			return -1;
		}

		uintptr_t symbolPtr = (uintptr_t)Header + symTable->sh_offset;
		Elf_Sym *sym = &((Elf_Sym *)symbolPtr)[Index];

		if (sym->st_shndx == SHN_UNDEF)
		{
			Elf_Shdr *stringTable = GetELFSection(Header, symTable->sh_link);
			const char *name = (const char *)Header + stringTable->sh_offset + sym->st_name;

			void *target = (void *)ELFLookupSymbol(Header, name)->st_value;
			if (target == nullptr)
			{
				if (ELF64_ST_BIND(sym->st_info) & STB_WEAK)
					return 0;
				else
				{
					error("Undefined external symbol \"%s\".", name);
					return -1;
				}
			}
			else
				return (uintptr_t)target;
		}
		else if (sym->st_shndx == SHN_ABS)
			return sym->st_value;
		else
		{
			Elf_Shdr *shdr = GetELFSection(Header, sym->st_shndx);
			return (uintptr_t)Header + sym->st_value + shdr->sh_offset;
		}
	}
}
