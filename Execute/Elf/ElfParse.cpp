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
#include "../../Fex.hpp"

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

	Elf64_Sym *ELFLookupSymbol(Elf64_Ehdr *Header, const char *Name)
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
			if (strcmp(String, Name) == 0)
				return Symbol;
		}
		return nullptr;
	}

	Elf64_Sym ELFLookupSymbol(VirtualFileSystem::File &ElfFile, const char *Name)
	{
		off_t OldOffset = vfs->Seek(ElfFile, 0, SEEK_CUR);

		Elf64_Ehdr Header;
		vfs->Seek(ElfFile, 0, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&Header, sizeof(Elf64_Ehdr));

		Elf64_Shdr SymbolTable;
		Elf64_Shdr StringTable;

		for (Elf64_Half i = 0; i < Header.e_shnum; i++)
		{
			Elf64_Shdr shdr;
			vfs->Seek(ElfFile, Header.e_shoff + (i * sizeof(Elf64_Shdr)), SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)&shdr, sizeof(Elf64_Shdr));

			switch (shdr.sh_type)
			{
			case SHT_SYMTAB:
				SymbolTable = shdr;
				vfs->Seek(ElfFile, Header.e_shoff + (shdr.sh_link * sizeof(Elf64_Shdr)), SEEK_SET);
				vfs->Read(ElfFile, (uint8_t *)&StringTable, sizeof(Elf64_Shdr));
				break;
			default:
			{
				break;
			}
			}
		}

		if (SymbolTable.sh_name == 0 ||
			StringTable.sh_name == 0)
		{
			error("Symbol table not found.");
			vfs->Seek(ElfFile, OldOffset, SEEK_SET);
			return {};
		}

		for (size_t i = 0; i < (SymbolTable.sh_size / sizeof(Elf64_Sym)); i++)
		{
			// Elf64_Sym *Symbol = (Elf64_Sym *)((uintptr_t)Header + SymbolTable->sh_offset + (i * sizeof(Elf64_Sym)));
			Elf64_Sym Symbol;
			vfs->Seek(ElfFile, SymbolTable.sh_offset + (i * sizeof(Elf64_Sym)), SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)&Symbol, sizeof(Elf64_Sym));

			// char *String = (char *)((uintptr_t)Header + StringTable->sh_offset + Symbol->st_name);
			char String[256];
			vfs->Seek(ElfFile, StringTable.sh_offset + Symbol.st_name, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)&String, 256);

			if (strcmp(String, Name) == 0)
			{
				vfs->Seek(ElfFile, OldOffset, SEEK_SET);
				return Symbol;
			}
		}
		error("Symbol not found.");
		vfs->Seek(ElfFile, OldOffset, SEEK_SET);
		return {};
	}

	uintptr_t ELFGetSymbolValue(Elf64_Ehdr *Header, uint64_t Table, uint64_t Index)
	{
#if defined(a64)
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
#endif
	}

	void CopyLOADSegments(VirtualFileSystem::File &ElfFile, uintptr_t HdrsBase, uintptr_t PhysicalBase)
	{
		off_t OldOffset = vfs->Seek(ElfFile, 0, SEEK_CUR);

		Elf64_Ehdr ELFHeader;
		vfs->Seek(ElfFile, 0, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		Elf64_Phdr ProgramHeaders;
		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			vfs->Seek(ElfFile, ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)), SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));
			if (ProgramHeaders.p_type != PT_LOAD)
				continue;

			uintptr_t SegmentDestination = (ProgramHeaders.p_vaddr - HdrsBase) + PhysicalBase;

			debug("Copying segment to %#lx (%ld file bytes, %ld mem bytes)",
				  SegmentDestination,
				  ProgramHeaders.p_filesz, ProgramHeaders.p_memsz);

			if (ProgramHeaders.p_filesz > 0)
			{
				vfs->Seek(ElfFile, ProgramHeaders.p_offset, SEEK_SET);
				vfs->Read(ElfFile, (uint8_t *)SegmentDestination, ProgramHeaders.p_filesz);
			}

			if (ProgramHeaders.p_memsz - ProgramHeaders.p_filesz > 0)
				memset((void *)(SegmentDestination + ProgramHeaders.p_filesz),
					   0,
					   ProgramHeaders.p_memsz - ProgramHeaders.p_filesz);
		}

		vfs->Seek(ElfFile, OldOffset, SEEK_SET);
	}

	void GetBaseAndSize(VirtualFileSystem::File &ElfFile, uintptr_t &Base, size_t &Size)
	{
		Base = UINTPTR_MAX;
		Size = 0;

		off_t OldOffset = vfs->Seek(ElfFile, 0, SEEK_CUR);

		Elf64_Ehdr ELFHeader;
		vfs->Seek(ElfFile, 0, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		Elf64_Phdr ProgramHeaders;
		vfs->Seek(ElfFile, ELFHeader.e_phoff, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));

		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			Base = MIN(Base, ProgramHeaders.p_vaddr);
			uintptr_t SegmentEnd = ProgramHeaders.p_vaddr - Base + ProgramHeaders.p_memsz;
			Size = MAX(Size, SegmentEnd);

			vfs->Seek(ElfFile, sizeof(Elf64_Phdr), SEEK_CUR);
			vfs->Read(ElfFile, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));
		}
		debug("Base: %#lx, Size: %#lx (%ld, %ld KB)", Base, Size, Size, TO_KB(Size));
		vfs->Seek(ElfFile, OldOffset, SEEK_SET);
	}

	MmImage ELFCreateMemoryImage(Memory::MemMgr *mem,
								 Memory::Virtual &vmm,
								 VirtualFileSystem::File &ElfFile,
								 size_t Length)
	{
		off_t OldOffset = vfs->Seek(ElfFile, 0, SEEK_CUR);

		void *MemoryImage = nullptr;
		Elf64_Ehdr ELFHeader;
		vfs->Seek(ElfFile, 0, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));

		bool IsPIC = ELFHeader.e_type == ET_DYN;
		UNUSED(IsPIC);
		debug("Elf %s PIC", IsPIC ? "is" : "is not");

		bool FirstProgramHeader = false;
		uintptr_t FirstProgramHeaderVirtualAddress = 0x0;

		Elf64_Phdr ProgramHeaders;
		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			vfs->Seek(ElfFile, ELFHeader.e_phoff + i * sizeof(Elf64_Phdr), SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)&ProgramHeaders, sizeof(Elf64_Phdr));
			if (ProgramHeaders.p_type != PT_LOAD)
				continue;

			if (!FirstProgramHeader)
			{
				FirstProgramHeader = true;
				FirstProgramHeaderVirtualAddress = ProgramHeaders.p_vaddr;
			}

			if (ProgramHeaders.p_vaddr == 0)
			{
				debug("p_vaddr is 0, allocating %ld pages for image (size: %#lx)", TO_PAGES(Length), Length);
				MemoryImage = mem->RequestPages(TO_PAGES(Length), true);
				debug("MemoryImage: %#lx-%#lx", MemoryImage, (uintptr_t)MemoryImage + Length);
				memset(MemoryImage, 0, Length);

				vfs->Seek(ElfFile, OldOffset, SEEK_SET);
				return {MemoryImage, (void *)FirstProgramHeaderVirtualAddress};
			}
		}

		debug("Allocating %ld pages for image (size: %#lx)", TO_PAGES(Length), Length);
		MemoryImage = mem->RequestPages(TO_PAGES(Length));
		debug("MemoryImage: %#lx-%#lx", MemoryImage, (uintptr_t)MemoryImage + Length);
		memset(MemoryImage, 0, Length);

		uintptr_t FirstProgramHeaderVirtualAddressAligned = 0;

		if (FirstProgramHeaderVirtualAddress != 0)
		{
			FirstProgramHeaderVirtualAddressAligned = ALIGN_DOWN(FirstProgramHeaderVirtualAddress, PAGE_SIZE);
			debug("Aligning address %#lx to %#lx",
				  FirstProgramHeaderVirtualAddress,
				  FirstProgramHeaderVirtualAddressAligned);
		}
		else
			FirstProgramHeaderVirtualAddress = (uintptr_t)MemoryImage;

		for (size_t i = 0; i < TO_PAGES(Length); i++)
		{
			vmm.Remap((void *)((uintptr_t)FirstProgramHeaderVirtualAddressAligned + (i * PAGE_SIZE)), (void *)((uintptr_t)MemoryImage + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
			debug("Remapped: %#lx -> %#lx", (uintptr_t)FirstProgramHeaderVirtualAddressAligned + (i * PAGE_SIZE), (uintptr_t)MemoryImage + (i * PAGE_SIZE));
		}

		vfs->Seek(ElfFile, OldOffset, SEEK_SET);
		return {MemoryImage, (void *)FirstProgramHeaderVirtualAddress};
	}
}
