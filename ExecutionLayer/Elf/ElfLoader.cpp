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
#include <rand.hpp>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;
using namespace VirtualFileSystem;

namespace Execute
{
	void ELFObject::LoadPhdrs_x86_32(int fd,
									 Elf64_Ehdr ELFHeader,
									 Memory::MemMgr *mm,
									 PCB *TargetProcess)
	{
		stub;
		UNUSED(fd);
		UNUSED(ELFHeader);
		UNUSED(TargetProcess);
	}

	void ELFObject::LoadPhdrs_x86_64(int fd,
									 Elf64_Ehdr ELFHeader,
									 Memory::MemMgr *mm,
									 PCB *TargetProcess)
	{
#if defined(a64)
		// assert(ELFHeader.e_phentsize == sizeof(Elf64_Phdr));

		size_t size = ELFHeader.e_phnum * sizeof(Elf64_Phdr);
		assert(size > 0);
		assert(size < 65536);
		assert(size < PAGE_SIZE);

		ELFProgramHeaders = mm->RequestPages(TO_PAGES(size), true);
		lseek(fd, ELFHeader.e_phoff, SEEK_SET);
		fread(fd, (uint8_t *)ELFProgramHeaders, size);
#endif
	}

	void ELFObject::GenerateAuxiliaryVector_x86_32(Memory::MemMgr *mm,
												   int fd,
												   Elf32_Ehdr ELFHeader,
												   uint32_t EntryPoint,
												   uint32_t BaseAddress)
	{
	}

	void ELFObject::GenerateAuxiliaryVector_x86_64(Memory::MemMgr *mm,
												   int fd,
												   Elf64_Ehdr ELFHeader,
												   uint64_t EntryPoint,
												   uint64_t BaseAddress)
	{
#if defined(a64)
		char *aux_platform = (char *)mm->RequestPages(1, true); /* TODO: 4KiB is too much for this */
		strcpy(aux_platform, "x86_64");

		std::string execfn = thisProcess->FileDescriptors->GetAbsolutePath(fd);
		void *execfn_str = mm->RequestPages(TO_PAGES(execfn.size() + 1), true);
		strcpy((char *)execfn_str, execfn.c_str());
		void *at_random = mm->RequestPages(1, true);
		*(uint64_t *)at_random = Random::rand16();

		Elfauxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uint64_t)aux_platform}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uint64_t)execfn_str}}});
		// AT_HWCAP2 26
		Elfauxv.push_back({.archaux = {.a_type = AT_RANDOM, .a_un = {.a_val = (uint64_t)at_random}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_SECURE, .a_un = {.a_val = (uint64_t)0}}}); /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_EGID, .a_un = {.a_val = (uint64_t)0}}});   /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_GID, .a_un = {.a_val = (uint64_t)0}}});	   /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_EUID, .a_un = {.a_val = (uint64_t)0}}});   /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_UID, .a_un = {.a_val = (uint64_t)0}}});	   /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uint64_t)EntryPoint}}});
		// AT_FLAGS 8
		Elfauxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uint64_t)BaseAddress}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uint64_t)ELFHeader.e_phnum}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uint64_t)ELFHeader.e_phentsize}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uint64_t)ELFProgramHeaders}}});
		// AT_CLKTCK 17
		Elfauxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uint64_t)PAGE_SIZE}}});
// AT_HWCAP 16
// AT_MINSIGSTKSZ 51
// AT_SYSINFO_EHDR 33
#endif
	}

	void ELFObject::LoadExec_x86_32(int fd, PCB *TargetProcess)
	{
		stub;
		UNUSED(fd);
		UNUSED(TargetProcess);
	}

	void ELFObject::LoadExec_x86_64(int fd, PCB *TargetProcess)
	{
#if defined(a64)
		std::string InterpreterPath;
		std::vector<Elf64_Phdr> PhdrINTERP = ELFGetSymbolType_x86_64(fd, PT_INTERP);
		foreach (auto Interp in PhdrINTERP)
		{
			Memory::SmartHeap sh = Memory::SmartHeap(256);
			lseek(fd, Interp.p_offset, SEEK_SET);
			fread(fd, sh, 256);
			InterpreterPath = sh;

			int ifd = fopen(InterpreterPath.c_str(), "r");
			if (ifd < 0)
			{
				warn("Failed to open interpreter file: %s",
					 InterpreterPath.c_str());
				continue;
			}
			else
			{
				if (GetBinaryType(InterpreterPath.c_str()) != BinTypeELF)
				{
					warn("Interpreter %s is not an ELF file",
						 InterpreterPath.c_str());
					fclose(ifd);
					continue;
				}

				if (LoadInterpreter(ifd, TargetProcess))
				{
					/* FIXME: specify argv[1] as the location for the interpreter */

					debug("Interpreter loaded successfully");
					fclose(ifd);
					return;
				}
			}
		}

		Elf64_Ehdr ELFHeader;
		fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));
		uintptr_t EntryPoint = ELFHeader.e_entry;
		debug("Entry point is %#lx", EntryPoint);

		Memory::Virtual vmm = Memory::Virtual(TargetProcess->PageTable);
		Memory::MemMgr *mm = TargetProcess->Memory;

		/* Copy segments into memory */
		{
			Elf64_Phdr ProgramBreakHeader{};
			Elf64_Phdr ProgramHeader;
			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				lseek(fd, ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)), SEEK_SET);
				fread(fd, (uint8_t *)&ProgramHeader, sizeof(Elf64_Phdr));
				switch (ProgramHeader.p_type)
				{
				case PT_LOAD:
				{
					if (ProgramHeader.p_memsz == 0)
						continue;

					void *pAddr = mm->RequestPages(TO_PAGES(ProgramHeader.p_memsz), true);
					void *SegmentDestination = (void *)ProgramHeader.p_vaddr;

					vmm.Map(SegmentDestination, pAddr,
							ProgramHeader.p_memsz,
							Memory::P | Memory::RW | Memory::US);

					debug("Mapped %#lx to %#lx", SegmentDestination, pAddr);

					debug("Copying segment to p: %#lx-%#lx; v: %#lx-%#lx (%ld file bytes, %ld mem bytes)",
						  pAddr, uintptr_t(pAddr) + ProgramHeader.p_memsz,
						  SegmentDestination, uintptr_t(SegmentDestination) + ProgramHeader.p_memsz,
						  ProgramHeader.p_filesz, ProgramHeader.p_memsz);

					if (ProgramHeader.p_filesz > 0)
					{
						lseek(fd, ProgramHeader.p_offset, SEEK_SET);
						fread(fd, (uint8_t *)pAddr, ProgramHeader.p_filesz);
					}

					if (ProgramHeader.p_memsz - ProgramHeader.p_filesz > 0)
					{
						void *zAddr = (void *)(uintptr_t(pAddr) + ProgramHeader.p_filesz);
						memset(zAddr, 0, ProgramHeader.p_memsz - ProgramHeader.p_filesz);
					}
					ProgramBreakHeader = ProgramHeader;
					break;
				}
				default:
				{
					fixme("Unhandled program header type: %#lx",
						  ProgramHeader.p_type);
					break;
				}
				}
			}

			/* Set program break */
			uintptr_t ProgramBreak = ROUND_UP(ProgramBreakHeader.p_vaddr +
												  ProgramBreakHeader.p_memsz,
											  PAGE_SIZE);

			TargetProcess->ProgramBreak->InitBrk(ProgramBreak);
		}

		struct stat statbuf;
		fstat(fd, &statbuf);
		Memory::SmartHeap sh = Memory::SmartHeap(statbuf.st_size);
		lseek(fd, 0, SEEK_SET);
		fread(fd, sh, statbuf.st_size);
		TargetProcess->ELFSymbolTable->AppendSymbols(uintptr_t(sh.GetObject()));

		debug("Entry Point: %#lx", EntryPoint);

		this->GenerateAuxiliaryVector_x86_64(mm, fd, ELFHeader,
											 EntryPoint, 0);

		this->ip = EntryPoint;
		this->IsElfValid = true;
#endif
	}

	void ELFObject::LoadDyn_x86_32(int fd, PCB *TargetProcess)
	{
		stub;
		UNUSED(fd);
		UNUSED(TargetProcess);
	}

	void ELFObject::LoadDyn_x86_64(int fd, PCB *TargetProcess)
	{
#if defined(a64)
		std::string InterpreterPath;
		std::vector<Elf64_Phdr> PhdrINTERP = ELFGetSymbolType_x86_64(fd, PT_INTERP);
		foreach (auto Interp in PhdrINTERP)
		{
			Memory::SmartHeap sh = Memory::SmartHeap(256);
			lseek(fd, Interp.p_offset, SEEK_SET);
			fread(fd, sh, 256);
			InterpreterPath = sh;

			int ifd = fopen(InterpreterPath.c_str(), "r");
			if (ifd < 0)
			{
				warn("Failed to open interpreter file: %s",
					 InterpreterPath.c_str());
				continue;
			}
			else
			{
				if (GetBinaryType(InterpreterPath.c_str()) != BinTypeELF)
				{
					warn("Interpreter %s is not an ELF file",
						 InterpreterPath.c_str());
					fclose(ifd);
					continue;
				}

				if (LoadInterpreter(ifd, TargetProcess))
				{
					/* FIXME: specify argv[1] as the location for the interpreter */

					debug("Interpreter loaded successfully");
					fclose(ifd);
					return;
				}
			}
		}

		Elf64_Ehdr ELFHeader;
		fread(fd, (uint8_t *)&ELFHeader, sizeof(Elf64_Ehdr));
		uintptr_t EntryPoint = ELFHeader.e_entry;
		debug("Entry point is %#lx", EntryPoint);

		Memory::Virtual vmm = Memory::Virtual(TargetProcess->PageTable);
		Memory::MemMgr *mm = TargetProcess->Memory;
		uintptr_t BaseAddress = 0;

		LoadPhdrs_x86_64(fd, ELFHeader, mm, TargetProcess);

		/* Copy segments into memory */
		{
			Elf64_Phdr ProgramBreakHeader{};
			Elf64_Phdr ProgramHeader;

			size_t SegmentsSize = 0;
			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				lseek(fd, ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)), SEEK_SET);
				fread(fd, (uint8_t *)&ProgramHeader, sizeof(Elf64_Phdr));

				if (ProgramHeader.p_type == PT_LOAD ||
					ProgramHeader.p_type == PT_DYNAMIC)
				{
					if (SegmentsSize < ProgramHeader.p_vaddr + ProgramHeader.p_memsz)
					{
						SegmentsSize = ProgramHeader.p_vaddr + ProgramHeader.p_memsz;
						ProgramBreakHeader = ProgramHeader;
					}
				}
			}
			debug("SegmentsSize: %#lx", SegmentsSize);

			/* TODO: Check if this is correct and/or it needs more
				complex calculations & allocations */
			void *SegmentsAddress = mm->RequestPages(TO_PAGES(SegmentsSize) + 1, true);
			BaseAddress = (uintptr_t)SegmentsAddress;
			debug("BaseAddress: %#lx, End: %#lx (%#lx)", BaseAddress,
				  BaseAddress + FROM_PAGES(TO_PAGES(SegmentsSize)),
				  SegmentsSize);

			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				lseek(fd, ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)), SEEK_SET);
				fread(fd, (uint8_t *)&ProgramHeader, sizeof(Elf64_Phdr));

				switch (ProgramHeader.p_type)
				{
				case PT_LOAD:
				{
					/* Because this is ET_DYN, we can load the segments
						anywhere we want. */
					uintptr_t SegmentDestination = BaseAddress + ProgramHeader.p_vaddr;

					if (ProgramHeader.p_memsz == 0)
						continue;

					debug("Copying PT_LOAD    to %#lx-%#lx (%ld file bytes, %ld mem bytes)",
						  SegmentDestination, SegmentDestination + ProgramHeader.p_memsz,
						  ProgramHeader.p_filesz, ProgramHeader.p_memsz);

					if (ProgramHeader.p_filesz > 0)
					{
						lseek(fd, ProgramHeader.p_offset, SEEK_SET);
						fread(fd, (uint8_t *)SegmentDestination, ProgramHeader.p_filesz);
					}

					if (ProgramHeader.p_memsz - ProgramHeader.p_filesz > 0)
					{
						void *zAddr = (void *)(SegmentDestination + ProgramHeader.p_filesz);
						memset(zAddr, 0, ProgramHeader.p_memsz - ProgramHeader.p_filesz);
					}
					break;
				}
				case PT_DYNAMIC:
				{
					/* PT_DYNAMIC contains the dynamic linking information for the
					   executable or shared library. */

					uintptr_t DynamicSegmentDestination = BaseAddress + ProgramHeader.p_vaddr;

					if (ProgramHeader.p_memsz == 0)
						continue;

					debug("Copying PT_DYNAMIC to %#lx-%#lx (%ld file bytes, %ld mem bytes)",
						  DynamicSegmentDestination, DynamicSegmentDestination + ProgramHeader.p_memsz,
						  ProgramHeader.p_filesz, ProgramHeader.p_memsz);

					if (ProgramHeader.p_filesz > 0)
					{
						lseek(fd, ProgramHeader.p_offset, SEEK_SET);
						fread(fd, (uint8_t *)DynamicSegmentDestination, ProgramHeader.p_filesz);
					}

					if (ProgramHeader.p_memsz - ProgramHeader.p_filesz > 0)
					{
						void *zAddr = (void *)(DynamicSegmentDestination + ProgramHeader.p_filesz);
						memset(zAddr, 0, ProgramHeader.p_memsz - ProgramHeader.p_filesz);
					}
					break;
				}
				default:
				{
					fixme("Unhandled program header type: %#lx",
						  ProgramHeader.p_type);
					break;
				}
				}
			}

			/* Set program break */
			uintptr_t ProgramBreak = ROUND_UP(BaseAddress +
												  ProgramBreakHeader.p_vaddr +
												  ProgramBreakHeader.p_memsz,
											  PAGE_SIZE);

			TargetProcess->ProgramBreak->InitBrk(ProgramBreak);
		}

		EntryPoint += BaseAddress;
		debug("The new ep is %#lx", EntryPoint);

		std::vector<Elf64_Dyn> JmpRel = ELFGetDynamicTag_x86_64(fd, DT_JMPREL);
		std::vector<Elf64_Dyn> SymTab = ELFGetDynamicTag_x86_64(fd, DT_SYMTAB);
		std::vector<Elf64_Dyn> StrTab = ELFGetDynamicTag_x86_64(fd, DT_STRTAB);
		std::vector<Elf64_Dyn> RelaDyn = ELFGetDynamicTag_x86_64(fd, DT_RELA);
		std::vector<Elf64_Dyn> RelaDynSize = ELFGetDynamicTag_x86_64(fd, DT_RELASZ);

		size_t JmpRelSize = JmpRel.size();
		size_t SymTabSize = SymTab.size();
		size_t StrTabSize = StrTab.size();
		size_t RelaDynSize_v = RelaDyn.size();

		if (JmpRelSize < 1)
		{
			debug("No DT_JMPREL");
		}

		if (SymTabSize < 1)
		{
			debug("No DT_SYMTAB");
		}

		if (StrTabSize < 1)
		{
			debug("No DT_STRTAB");
		}

		if (RelaDynSize_v < 1)
		{
			debug("No DT_RELA");
		}

		if (RelaDynSize[0].d_un.d_val < 1)
		{
			debug("DT_RELASZ is < 1");
		}

		if (JmpRelSize > 0 && SymTabSize > 0 && StrTabSize > 0)
		{
			debug("JmpRel: %#lx, SymTab: %#lx, StrTab: %#lx",
				  JmpRel[0].d_un.d_ptr, SymTab[0].d_un.d_ptr,
				  StrTab[0].d_un.d_ptr);

			Elf64_Rela *_JmpRel = (Elf64_Rela *)((uintptr_t)BaseAddress + JmpRel[0].d_un.d_ptr);
			Elf64_Sym *_SymTab = (Elf64_Sym *)((uintptr_t)BaseAddress + SymTab[0].d_un.d_ptr);
			char *_DynStr = (char *)((uintptr_t)BaseAddress + StrTab[0].d_un.d_ptr);
			Elf64_Rela *_RelaDyn = (Elf64_Rela *)((uintptr_t)BaseAddress + RelaDyn[0].d_un.d_ptr);

			Elf64_Shdr shdr;
			for (Elf64_Half i = 0; i < ELFHeader.e_shnum; i++)
			{
				lseek(fd, ELFHeader.e_shoff + i * sizeof(Elf64_Shdr), SEEK_SET);
				fread(fd, (uint8_t *)&shdr, sizeof(Elf64_Shdr));

				char sectionName[32];
				Elf64_Shdr n_shdr;
				lseek(fd, ELFHeader.e_shoff + ELFHeader.e_shstrndx * sizeof(Elf64_Shdr), SEEK_SET);
				fread(fd, (uint8_t *)&n_shdr, sizeof(Elf64_Shdr));
				lseek(fd, n_shdr.sh_offset + shdr.sh_name, SEEK_SET);
				fread(fd, (uint8_t *)sectionName, 32);
				debug("shdr: %s", sectionName);

				if (strcmp(sectionName, ".rela.plt") == 0)
				{
					// .rela.plt
					// R_X86_64_JUMP_SLOT
					Elf64_Xword numEntries = shdr.sh_size / shdr.sh_entsize;
					for (Elf64_Xword i = 0; i < numEntries; i++)
					{
						Elf64_Addr *GOTEntry = (Elf64_Addr *)(shdr.sh_addr +
															  BaseAddress +
															  i * sizeof(Elf64_Addr));
						Elf64_Rela *Rel = _JmpRel + i;
						Elf64_Xword RelType = ELF64_R_TYPE(Rel->r_info);

						switch (RelType)
						{
						case R_X86_64_JUMP_SLOT:
						{
							Elf64_Xword SymIndex = ELF64_R_SYM(Rel->r_info);
							Elf64_Sym *Sym = _SymTab + SymIndex;

							if (Sym->st_name)
							{
								char *SymName = _DynStr + Sym->st_name;
								debug("SymName: %s", SymName);

								Elf64_Sym LibSym = ELFLookupSymbol(fd, SymName);
								if (LibSym.st_value)
								{
									*GOTEntry = (Elf64_Addr)(BaseAddress + LibSym.st_value);
									debug("GOT[%ld](%#lx): %#lx",
										  i, uintptr_t(GOTEntry) - BaseAddress,
										  *GOTEntry);
								}
							}
							continue;
						}
						default:
						{
							fixme("Unhandled relocation type: %#lx", RelType);
							break;
						}
						}
					}
				}
				else if (strcmp(sectionName, ".rela.dyn") == 0)
				{
					// .rela.dyn
					// R_X86_64_RELATIVE
					// R_X86_64_GLOB_DAT
					if (RelaDynSize_v < 1 || RelaDynSize[0].d_un.d_val < 1)
						continue;
					Elf64_Xword numRelaDynEntries = RelaDynSize[0].d_un.d_val / sizeof(Elf64_Rela);
					for (Elf64_Xword i = 0; i < numRelaDynEntries; i++)
					{
						Elf64_Rela *Rel = _RelaDyn + i;
						Elf64_Addr *GOTEntry = (Elf64_Addr *)(Rel->r_offset + BaseAddress);
						Elf64_Xword RelType = ELF64_R_TYPE(Rel->r_info);

						switch (RelType)
						{
						case R_X86_64_RELATIVE:
						{
							*GOTEntry = (Elf64_Addr)(BaseAddress + Rel->r_addend);
							debug("GOT[%ld](%#lx): %#lx (R_X86_64_RELATIVE)",
								  i, uintptr_t(GOTEntry) - BaseAddress,
								  *GOTEntry);
							break;
						}
						case R_X86_64_GLOB_DAT:
						{
							Elf64_Xword SymIndex = ELF64_R_SYM(Rel->r_info);
							Elf64_Sym *Sym = _SymTab + SymIndex;

							if (Sym->st_name)
							{
								char *SymName = _DynStr + Sym->st_name;
								debug("SymName: %s", SymName);

								Elf64_Sym LibSym = ELFLookupSymbol(fd, SymName);
								if (LibSym.st_value)
								{
									*GOTEntry = (Elf64_Addr)(BaseAddress + LibSym.st_value);
									debug("GOT[%ld](%#lx): %#lx (R_X86_64_GLOB_DAT)",
										  i, uintptr_t(GOTEntry) - BaseAddress,
										  *GOTEntry);
								}
							}
							break;
						}
						default:
						{
							fixme("Unhandled relocation type: %#lx", RelType);
							break;
						}
						}
					}
				}
				else if (strcmp(sectionName, ".dynsym") == 0)
				{
					// .dynsym
					// STT_OBJECT

					Elf64_Sym *SymArray = (Elf64_Sym *)(shdr.sh_addr + BaseAddress);
					Elf64_Xword numEntries = shdr.sh_size / shdr.sh_entsize;
					debug("start %#lx (off %#lx), entries %ld",
						  SymArray, shdr.sh_addr, numEntries);
					for (Elf64_Xword j = 0; j < numEntries; j++)
					{
						Elf64_Sym Sym = SymArray[j];

						if (Sym.st_shndx == SHN_UNDEF)
							continue;

						if (Sym.st_value == 0)
							continue;

						unsigned char SymType = ELF64_ST_TYPE(Sym.st_info);

						if (SymType == STT_OBJECT)
						{
							Elf64_Addr *GOTEntry = (Elf64_Addr *)(Sym.st_value + BaseAddress);
							*GOTEntry = (Elf64_Addr)(BaseAddress + Sym.st_value);

							debug("%ld: \"%s\" %#lx -> %#lx", j,
								  _DynStr + Sym.st_name,
								  uintptr_t(GOTEntry) - BaseAddress,
								  *GOTEntry);
						}
					}
				}
				else if (strcmp(sectionName, ".symtab") == 0)
				{
					// .symtab
					// STT_OBJECT

					Elf64_Xword numEntries = shdr.sh_size / shdr.sh_entsize;
					Elf64_Sym *SymArray = new Elf64_Sym[numEntries];
					lseek(fd, shdr.sh_offset, SEEK_SET);
					fread(fd, (uint8_t *)SymArray, shdr.sh_size);

					debug("start %#lx (off %#lx), entries %ld",
						  SymArray, shdr.sh_addr, numEntries);
					for (Elf64_Xword j = 0; j < numEntries; j++)
					{
						Elf64_Sym Sym = SymArray[j];

						if (Sym.st_shndx == SHN_UNDEF)
							continue;

						if (Sym.st_value == 0)
							continue;

						unsigned char SymType = ELF64_ST_TYPE(Sym.st_info);

						if (SymType == STT_OBJECT)
						{
							Elf64_Addr *GOTEntry = (Elf64_Addr *)(Sym.st_value + BaseAddress);
							*GOTEntry = (Elf64_Addr)(BaseAddress + Sym.st_value);

							debug("%ld: \"<fixme>\" %#lx -> %#lx", j,
								  /*_DynStr + Sym.st_name,*/
								  uintptr_t(GOTEntry) - BaseAddress,
								  *GOTEntry);
						}
					}
					delete[] SymArray;
				}

				// if (shdr.sh_type == SHT_PROGBITS &&
				// 	(shdr.sh_flags & SHF_WRITE) &&
				// 	(shdr.sh_flags & SHF_ALLOC))
			}
		}

		/* ------------------------------------------------------------------------ */

		struct stat statbuf;
		fstat(fd, &statbuf);
		Memory::SmartHeap sh = Memory::SmartHeap(statbuf.st_size);
		lseek(fd, 0, SEEK_SET);
		fread(fd, sh, statbuf.st_size);
		TargetProcess->ELFSymbolTable->AppendSymbols(uintptr_t(sh.GetObject()), BaseAddress);

		debug("Entry Point: %#lx", EntryPoint);

		this->GenerateAuxiliaryVector_x86_64(mm, fd, ELFHeader,
											 EntryPoint, BaseAddress);

		this->ip = EntryPoint;
		this->IsElfValid = true;
#endif
	}

	bool ELFObject::LoadInterpreter(int fd, PCB *TargetProcess)
	{
		Elf32_Ehdr ELFHeader;
		fread(fd, &ELFHeader, sizeof(Elf32_Ehdr));

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
				this->LoadExec_x86_32(fd, TargetProcess);
				return true;
			case EM_X86_64:
				this->LoadExec_x86_64(fd, TargetProcess);
				return true;
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
				this->LoadDyn_x86_32(fd, TargetProcess);
				return true;
			case EM_X86_64:
				this->LoadDyn_x86_64(fd, TargetProcess);
				return true;
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
		return false;
	}

	ELFObject::ELFObject(char *AbsolutePath,
						 PCB *TargetProcess,
						 const char **argv,
						 const char **envp)
	{
		if (GetBinaryType(AbsolutePath) != BinaryType::BinTypeELF)
		{
			error("%s is not an ELF file or is invalid.", AbsolutePath);
			return;
		}

		int fd = fopen(AbsolutePath, "r");
		if (fd < 0)
		{
			error("Failed to open %s, errno: %d", AbsolutePath, fd);
			return;
		}

		int argc = 0;
		int envc = 0;

		while (argv[argc] != nullptr)
			argc++;
		while (envp[envc] != nullptr)
			envc++;

		// ELFargv = new const char *[argc + 2];
		size_t argv_size = TO_PAGES(argc + 2 * sizeof(char *));
		ELFargv = (const char **)TargetProcess->Memory->RequestPages(argv_size);
		for (int i = 0; i < argc; i++)
		{
			size_t arg_size = TO_PAGES(strlen(argv[i]) + 1);
			ELFargv[i] = (const char *)TargetProcess->Memory->RequestPages(arg_size);
			strcpy((char *)ELFargv[i], argv[i]);
		}
		ELFargv[argc] = nullptr;

		// ELFenvp = new const char *[envc + 1];
		size_t envp_size = TO_PAGES(envc + 1 * sizeof(char *));
		ELFenvp = (const char **)TargetProcess->Memory->RequestPages(envp_size);
		for (int i = 0; i < envc; i++)
		{
			size_t env_size = TO_PAGES(strlen(envp[i]) + 1);
			ELFenvp[i] = (const char *)TargetProcess->Memory->RequestPages(env_size);
			strcpy((char *)ELFenvp[i], envp[i]);
		}
		ELFenvp[envc] = nullptr;

		Elf32_Ehdr ELFHeader;
		fread(fd, &ELFHeader, sizeof(Elf32_Ehdr));

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
				this->LoadExec_x86_32(fd, TargetProcess);
				break;
			case EM_X86_64:
				this->LoadExec_x86_64(fd, TargetProcess);
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
				this->LoadDyn_x86_32(fd, TargetProcess);
				break;
			case EM_X86_64:
				this->LoadDyn_x86_64(fd, TargetProcess);
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

		fclose(fd);
	}

	ELFObject::~ELFObject()
	{
	}
}
