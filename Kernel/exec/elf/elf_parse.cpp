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
		Elf32_Ehdr *ELFHeader = (Elf32_Ehdr *)Header;
		if (ELFHeader->e_ident[EI_CLASS] == ELFCLASS64)
			return true;
		return false;
	}

	/* Originally from https://wiki.osdev.org/ELF_Tutorial */

	Elf64_Shdr *GetELFSheader(Elf64_Ehdr *Header)
	{
		return (Elf64_Shdr *)((uintptr_t)Header + Header->e_shoff);
	}

	Elf64_Shdr *GetELFSection(Elf64_Ehdr *Header, uint64_t Index)
	{
		return &GetELFSheader(Header)[Index];
	}

	char *GetELFStringTable(Elf64_Ehdr *Header)
	{
		if (Header->e_shstrndx == SHN_UNDEF)
			return nullptr;
		return (char *)Header + GetELFSection(Header, Header->e_shstrndx)->sh_offset;
	}

	char *ELFLookupString(Elf64_Ehdr *Header, uintptr_t Offset)
	{
		char *StringTable = GetELFStringTable(Header);
		if (StringTable == nullptr)
			return nullptr;
		return StringTable + Offset;
	}

	Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, std::string Name)
	{
		Elf64_Shdr *SymbolTable = nullptr;
		Elf64_Shdr *StringTable = nullptr;

		for (Elf64_Half i = 0; i < Header->e_shnum; i++)
		{
			Elf64_Shdr *shdr = GetELFSection(Header, i);
			switch (shdr->sh_type)
			{
			case SHT_SYMTAB:
				SymbolTable = shdr;
				StringTable = GetELFSection(Header, shdr->sh_link);
				break;
			default:
			{
				break;
			}
			}
		}

		if (SymbolTable == nullptr || StringTable == nullptr)
			return nullptr;

		for (size_t i = 0; i < (SymbolTable->sh_size / sizeof(Elf64_Sym)); i++)
		{
			Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
			char *String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
			if (strcmp(String, Name.c_str()) == 0)
				return Symbol;
		}
		return nullptr;
	}

	Elf64_Sym ELFLookupSymbol(FileNode *fd, std::string Name)
	{
#if defined(__amd64__)
		Elf64_Ehdr Header{};
		fd->Read(&Header, sizeof(Elf64_Ehdr), 0);

		Elf64_Shdr SymbolTable{};
		Elf64_Shdr StringTable{};

		for (Elf64_Half i = 0; i < Header.e_shnum; i++)
		{
			Elf64_Shdr shdr;
			fd->Read(&shdr, sizeof(Elf64_Shdr), Header.e_shoff + (i * sizeof(Elf64_Shdr)));

			switch (shdr.sh_type)
			{
			case SHT_SYMTAB:
				SymbolTable = shdr;
				fd->Read(&StringTable, sizeof(Elf64_Shdr), Header.e_shoff + (shdr.sh_link * sizeof(Elf64_Shdr)));
				break;
			default:
			{
				break;
			}
			}
		}

		if (SymbolTable.sh_name == 0 || StringTable.sh_name == 0)
		{
			error("Symbol table not found.");
			return {};
		}

		for (size_t i = 0; i < (SymbolTable.sh_size / sizeof(Elf64_Sym)); i++)
		{
			// Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
			Elf64_Sym Symbol;
			fd->Read(&Symbol, sizeof(Elf64_Sym), SymbolTable.sh_offset + (i * sizeof(Elf64_Sym)));

			// char *String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
			char String[256];
			fd->Read(&String, sizeof(String), StringTable.sh_offset + Symbol.st_name);

			if (strcmp(String, Name.c_str()) == 0)
				return Symbol;
		}
		error("Symbol not found.");
#endif
		return {};
	}

	uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index)
	{
#if defined(__amd64__)
		if (Table == SHN_UNDEF || Index == SHN_UNDEF)
			return 0;
		Elf64_Shdr *SymbolTable = GetELFSection(Header, Table);

		uint64_t STEntries = SymbolTable->sh_size / SymbolTable->sh_entsize;
		if (Index >= STEntries)
		{
			error("Symbol index out of range %d-%u.", Table, Index);
			return 0xdead;
		}

		uint64_t SymbolAddress = (uint64_t)Header + SymbolTable->sh_offset;
		Elf64_Sym *Symbol = &((Elf64_Sym *)SymbolAddress)[Index];

		if (Symbol->st_shndx == SHN_UNDEF)
		{
			Elf64_Shdr *StringTable = GetELFSection(Header, SymbolTable->sh_link);
			const char *Name = (const char *)Header + StringTable->sh_offset + Symbol->st_name;

			void *Target = (void *)ELFLookupSymbol(Header, Name)->st_value;
			if (Target == nullptr)
			{
				if (ELF64_ST_BIND(Symbol->st_info) & STB_WEAK)
					return 0;
				else
				{
					error("Undefined external symbol \"%s\".", Name);
					return 0xdead;
				}
			}
			else
				return (uintptr_t)Target;
		}
		else if (Symbol->st_shndx == SHN_ABS)
			return Symbol->st_value;
		else
		{
			Elf64_Shdr *Target = GetELFSection(Header, Symbol->st_shndx);
			return (uintptr_t)Header + Symbol->st_value + Target->sh_offset;
		}
#elif defined(__i386__)
		return 0xdead;
#elif defined(__aarch64__)
		return 0xdead;
#endif
	}
}
