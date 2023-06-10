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

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;
using namespace VirtualFileSystem;

namespace Execute
{
	ELFBaseLoad ELFObject::LoadExec_x86_32(File &ElfFile, PCB *TargetProcess)
	{
		stub;
		return {};
	}

	ELFBaseLoad ELFObject::LoadExec_x86_64(File &ElfFile, PCB *TargetProcess)
	{
		ELFBaseLoad ELFBase{};

		uintptr_t BaseAddress;
		size_t ElfPHDRsSize;
		GetBaseAndSize(ElfFile, BaseAddress, ElfPHDRsSize);

		Elf64_Ehdr ELFHeader;
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));
		uintptr_t EntryPoint = ELFHeader.e_entry;
		debug("%s's entry point is %#lx", ElfFile.Name, EntryPoint);

		ELFBase.TmpMem = new Memory::MemMgr(TargetProcess->PageTable); /* This should be deleted inside BaseLoad.cpp */
		Memory::Virtual vmm(TargetProcess->PageTable);

		/* If required, MemoryImage will be at virtual address. (unless is PIC)
		   tl;dr this is where the code is stored. */
		MmImage MemoryImage = ELFCreateMemoryImage(ELFBase.TmpMem, vmm, ElfFile, ElfPHDRsSize);

		debug("Solving symbols for %s", ElfFile.Name);
		std::vector<Elf64_Shdr> DynamicString = ELFGetSections_x86_64(ElfFile, ".dynstr");
		std::vector<Elf64_Shdr> StringTable = ELFGetSections_x86_64(ElfFile, ".strtab");

		if (DynamicString.size() < 1) /* TODO: check if this is required */
			DynamicString = StringTable;

		/* Calculate entry point */
		Elf64_Phdr FirstPhdr;
		vfs->Seek(ElfFile, ELFHeader.e_phoff, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&FirstPhdr, sizeof(Elf64_Phdr));
		/* FIXME: this is not the correct way to calculate entry point */
		if (FirstPhdr.p_vaddr == 0)
		{
			debug("Entry point is null. Adding virtual address to entry point");
			EntryPoint += (uintptr_t)MemoryImage.Virtual;
		}

		CopyLOADSegments(ElfFile, BaseAddress, (uintptr_t)MemoryImage.Physical);

		foreach (auto Tag in ELFGetDynamicTag_x86_64(ElfFile, DT_NEEDED))
		{
			const char *ReqLib = new char[256];
			vfs->Seek(ElfFile, DynamicString[0].sh_offset + Tag.d_un.d_val, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)ReqLib, 256);

			debug("DT_NEEDED - %s", ReqLib);
			ELFBase.NeededLibraries.push_back(ReqLib);
		}

		char InterpreterPath[256] = {'\0'};
		std::vector<Elf64_Phdr> PhdrINTERP = ELFGetSymbolType_x86_64(ElfFile, PT_INTERP);
		foreach (auto Interp in PhdrINTERP)
		{
			const char *InterpPath = new char[256];
			vfs->Seek(ElfFile, Interp.p_offset, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)InterpPath, 256);

			memcpy((void *)InterpreterPath, InterpPath,
				   (strlen(InterpPath) > 256) ? 256 : strlen(InterpPath));
			debug("Interpreter: %s", InterpreterPath);
			delete[] InterpPath;

			VirtualFileSystem::File InterpreterFile = vfs->Open(InterpreterPath);
			if (!InterpreterFile.IsOK())
			{
				warn("Failed to open interpreter file: %s", InterpreterPath);
				vfs->Close(InterpreterFile);
				continue;
			}
			else
			{
				if (GetBinaryType(InterpreterPath) != BinTypeELF)
				{
					warn("Interpreter %s is not an ELF file", InterpreterPath);
					vfs->Close(InterpreterFile);
					continue;
				}
				vfs->Close(InterpreterFile);
				break;
			}
		}

		if (strlen(InterpreterPath) > 1)
		{
			EntryPoint = LoadELFInterpreter(ELFBase.TmpMem, vmm, InterpreterPath);
			ELFBase.Interpreter = true;
		}

		debug("Entry Point: %#lx", EntryPoint);

		char *aux_platform = (char *)ELFBase.TmpMem->RequestPages(1, true); /* TODO: 4096 bytes is too much for this */
		strcpy(aux_platform, "x86_64");

		ELFBase.auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)0 /* FIXME */}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t)aux_platform}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)EntryPoint}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)MemoryImage.Virtual}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader.e_phnum}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader.e_phentsize}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFHeader.e_phoff}}});

		ELFBase.InstructionPointer = EntryPoint;
		ELFBase.MemoryImage = MemoryImage.Physical;
		ELFBase.VirtualMemoryImage = MemoryImage.Virtual;

		ELFBase.Success = true;
		return ELFBase;
	}

	ELFBaseLoad ELFObject::LoadDyn_x86_32(File &ElfFile, PCB *TargetProcess, bool IsLibrary)
	{
		stub;
		return {};
	}

	ELFBaseLoad ELFObject::LoadDyn_x86_64(File &ElfFile, PCB *TargetProcess, bool IsLibrary)
	{
		ELFBaseLoad ELFBase{};

		uintptr_t BaseAddress;
		size_t ElfPHDRsSize;
		GetBaseAndSize(ElfFile, BaseAddress, ElfPHDRsSize);

		Elf64_Ehdr ELFHeader;
		vfs->Read(ElfFile, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));
		uintptr_t EntryPoint = ELFHeader.e_entry;
		debug("%s's entry point is %#lx", ElfFile.Name, EntryPoint);

		ELFBase.TmpMem = new Memory::MemMgr(TargetProcess->PageTable); /* This should be deleted inside BaseLoad.cpp */
		Memory::Virtual vmm(TargetProcess->PageTable);

		/* If required, MemoryImage will be at virtual address. (unless is PIC)
		   tl;dr this is where the code is stored. */
		MmImage MemoryImage = ELFCreateMemoryImage(ELFBase.TmpMem, vmm, ElfFile, ElfPHDRsSize);

		debug("Solving symbols for %s", ElfFile.Name);
		std::vector<Elf64_Shdr> DynamicString = ELFGetSections_x86_64(ElfFile, ".dynstr");
		std::vector<Elf64_Shdr> StringTable = ELFGetSections_x86_64(ElfFile, ".strtab");

		if (DynamicString.size() < 1) /* TODO: check if this is required */
			DynamicString = StringTable;

		/* Calculate entry point */
		Elf64_Phdr FirstPhdr;
		vfs->Seek(ElfFile, ELFHeader.e_phoff, SEEK_SET);
		vfs->Read(ElfFile, (uint8_t *)&FirstPhdr, sizeof(Elf64_Phdr));
		/* FIXME: this is not the correct way to calculate entry point */
		if (FirstPhdr.p_vaddr == 0)
		{
			debug("Entry point is null. Adding virtual address to entry point");
			EntryPoint += (uintptr_t)MemoryImage.Virtual;
		}

		CopyLOADSegments(ElfFile, BaseAddress, (uintptr_t)MemoryImage.Physical);

		foreach (auto Tag in ELFGetDynamicTag_x86_64(ElfFile, DT_NEEDED))
		{
			const char *ReqLib = new char[256];
			vfs->Seek(ElfFile, DynamicString[0].sh_offset + Tag.d_un.d_val, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)ReqLib, 256);

			debug("DT_NEEDED - %s", ReqLib);
			ELFBase.NeededLibraries.push_back(ReqLib);
		}

		std::vector<Elf64_Dyn> JmpRel = ELFGetDynamicTag_x86_64(ElfFile, DT_JMPREL);
		std::vector<Elf64_Dyn> SymTab = ELFGetDynamicTag_x86_64(ElfFile, DT_SYMTAB);
		std::vector<Elf64_Dyn> StrTab = ELFGetDynamicTag_x86_64(ElfFile, DT_STRTAB);

		if (JmpRel.size() < 1)
		{
			debug("No DT_JMPREL");
		}

		if (SymTab.size() < 1)
		{
			debug("No DT_SYMTAB");
		}

		if (StrTab.size() < 1)
		{
			debug("No DT_STRTAB");
		}

		if (JmpRel.size() > 1 &&
			SymTab.size() > 1 &&
			StrTab.size() > 1)
		{
			debug("JmpRel: %#lx, SymTab: %#lx, StrTab: %#lx", JmpRel[0].d_un.d_ptr, SymTab[0].d_un.d_ptr, StrTab[0].d_un.d_ptr);
			Elf64_Rela *_JmpRel = (Elf64_Rela *)((uintptr_t)MemoryImage.Physical + (JmpRel[0].d_un.d_ptr - BaseAddress));
			Elf64_Sym *_SymTab = (Elf64_Sym *)((uintptr_t)MemoryImage.Physical + (SymTab[0].d_un.d_ptr - BaseAddress));

			char *_DynStr = (char *)((uintptr_t)MemoryImage.Physical + (StrTab[0].d_un.d_ptr - BaseAddress));

			Elf64_Shdr *gotSection = nullptr;
			Elf64_Shdr shdr;
			for (Elf64_Half i = 0; i < ELFHeader.e_shnum; i++)
			{
				vfs->Seek(ElfFile, ELFHeader.e_shoff + i * sizeof(Elf64_Shdr), SEEK_SET);
				vfs->Read(ElfFile, (uint8_t *)&shdr, sizeof(Elf64_Shdr));
				if (shdr.sh_type == SHT_PROGBITS &&
					(shdr.sh_flags & SHF_WRITE) &&
					(shdr.sh_flags & SHF_ALLOC))
				{
					gotSection = new Elf64_Shdr;
					*gotSection = shdr;
					break;
				}
			}

			if (gotSection)
			{
				Elf64_Xword numEntries = gotSection->sh_size / sizeof(Elf64_Addr);
				for (Elf64_Xword i = 0; i < numEntries - 3; i++)
				{
					Elf64_Rela *Rel = _JmpRel + i;
					Elf64_Addr *GOTEntry = (Elf64_Addr *)(Rel->r_offset + (uintptr_t)MemoryImage.Physical);

					Elf64_Xword RelType = ELF64_R_TYPE(Rel->r_info);
					debug("r_offset: %#lx RelType: %d", Rel->r_offset, RelType);

					switch (RelType)
					{
					case R_X86_64_NONE:
						break;
					case R_X86_64_JUMP_SLOT:
					{
						Elf64_Xword SymIndex = ELF64_R_SYM(Rel->r_info);
						Elf64_Sym *Sym = _SymTab + SymIndex;

						if (Sym->st_name)
						{
							char *SymName = _DynStr + Sym->st_name;
							debug("SymName: %s", SymName);

							Elf64_Sym LibSym = ELFLookupSymbol(ElfFile, SymName);

							if (LibSym.st_value)
							{
								*GOTEntry = (Elf64_Addr)((uintptr_t)MemoryImage.Physical + LibSym.st_value);
								debug("GOT[%ld]: %#lx + %#lx = %#lx",
									  i, (uintptr_t)MemoryImage.Physical, LibSym.st_value, *GOTEntry);
							}
						}
						break;
					}
					default:
					{
						fixme("RelType %d not supported", RelType);
						break;
					}
					}

					debug("GOT[%ld](%#lx): %#lx", i, GOTEntry, *GOTEntry);
				}

				delete gotSection;
			}
			else
			{
				debug("GOT section not found");
			}
		}

		/* ------------------------------------------------------------------------ */

		foreach (auto Tag in ELFGetDynamicTag_x86_64(ElfFile, DT_NEEDED))
		{
			const char *ReqLib = new char[256];
			vfs->Seek(ElfFile, DynamicString[0].sh_offset + Tag.d_un.d_val, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)ReqLib, 256);

			debug("DT_NEEDED - %s", ReqLib);
			ELFBase.NeededLibraries.push_back(ReqLib);
		}

		char InterpreterPath[256] = {'\0'};
		std::vector<Elf64_Phdr> PhdrINTERP = ELFGetSymbolType_x86_64(ElfFile, PT_INTERP);
		foreach (auto Interp in PhdrINTERP)
		{
			const char *InterpPath = new char[256];
			vfs->Seek(ElfFile, Interp.p_offset, SEEK_SET);
			vfs->Read(ElfFile, (uint8_t *)InterpPath, 256);

			memcpy((void *)InterpreterPath, InterpPath,
				   (strlen(InterpPath) > 256) ? 256 : strlen(InterpPath));
			debug("Interpreter: %s", InterpreterPath);
			delete[] InterpPath;

			VirtualFileSystem::File InterpreterFile = vfs->Open(InterpreterPath);
			if (!InterpreterFile.IsOK())
			{
				warn("Failed to open interpreter file: %s", InterpreterPath);
				vfs->Close(InterpreterFile);
				continue;
			}
			else
			{
				if (GetBinaryType(InterpreterPath) != BinTypeELF)
				{
					warn("Interpreter %s is not an ELF file", InterpreterPath);
					vfs->Close(InterpreterFile);
					continue;
				}
				vfs->Close(InterpreterFile);
				break;
			}
		}

		if (strlen(InterpreterPath) > 1)
		{
			EntryPoint = LoadELFInterpreter(ELFBase.TmpMem, vmm, InterpreterPath);
			ELFBase.Interpreter = true;
		}
		else if (IsLibrary)
		{
			/* FIXME: Detect interpreter from current running process. */
			EntryPoint = LoadELFInterpreter(ELFBase.TmpMem, vmm, "/lib/ld.so");
			ELFBase.Interpreter = true;
		}

		debug("Entry Point: %#lx", EntryPoint);

		char *aux_platform = (char *)ELFBase.TmpMem->RequestPages(1, true); /* TODO: 4096 bytes is too much for this */
		strcpy(aux_platform, "x86_64");

		ELFBase.auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)0 /* FIXME */}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t)aux_platform}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)EntryPoint}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)MemoryImage.Virtual}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader.e_phnum}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader.e_phentsize}}});
		ELFBase.auxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFHeader.e_phoff}}});

		ELFBase.InstructionPointer = EntryPoint;
		ELFBase.MemoryImage = MemoryImage.Physical;
		ELFBase.VirtualMemoryImage = MemoryImage.Virtual;

		ELFBase.Success = true;
		return ELFBase;
	}

	ELFObject::ELFObject(char *AbsolutePath,
						 PCB *TargetProcess,
						 bool IsLibrary)
	{
		if (GetBinaryType(AbsolutePath) != BinaryType::BinTypeELF)
		{
			error("%s is not an ELF file or is invalid.", AbsolutePath);
			return;
		}

		VirtualFileSystem::File ExFile = vfs->Open(AbsolutePath);

		Elf32_Ehdr ELFHeader;
		vfs->Read(ExFile, (uint8_t *)&ELFHeader, sizeof(Elf32_Ehdr));

		ELFBaseLoad bl;
		switch (ELFHeader.e_type)
		{
		case ET_REL:
		{
			fixme("ET_REL not implemented");
			break;
		}
		case ET_EXEC:
		{
			switch (ELFHeader.e_machine)
			{
			case EM_386:
				this->BaseLoadInfo = this->LoadExec_x86_32(ExFile,
														   TargetProcess);
				break;
			case EM_X86_64:
				this->BaseLoadInfo = this->LoadExec_x86_64(ExFile,
														   TargetProcess);
				break;
			case EM_ARM:
				error("ARM is not supported yet!");
				break;
			case EM_AARCH64:
				error("ARM64 is not supported yet!");
				break;
			default:
				error("Unknown architecture: %d", ELFHeader.e_machine);
				break;
			}
			break;
		}
		case ET_DYN:
		{
			switch (ELFHeader.e_machine)
			{
			case EM_386:
				this->BaseLoadInfo = this->LoadDyn_x86_32(ExFile,
														  TargetProcess,
														  IsLibrary);
				break;
			case EM_X86_64:
				this->BaseLoadInfo = this->LoadDyn_x86_64(ExFile,
														  TargetProcess,
														  IsLibrary);
				break;
			case EM_ARM:
				error("ARM is not supported yet!");
				break;
			case EM_AARCH64:
				error("ARM64 is not supported yet!");
				break;
			default:
				error("Unknown architecture: %d", ELFHeader.e_machine);
				break;
			}
			break;
		}
		case ET_CORE:
		{
			fixme("ET_CORE not implemented");
			break;
		}
		case ET_NONE:
		default:
		{
			error("Unknown ELF Type: %d", ELFHeader.e_type);
			break;
		}
		}

		vfs->Close(ExFile);
	}

	ELFObject::~ELFObject()
	{
	}
}
