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

using namespace Tasking;
using namespace vfs;

namespace Execute
{
	void ELFObject::GenerateAuxiliaryVector(Memory::VirtualMemoryArea *vma, FileNode *fd, Elf_Ehdr ELFHeader, uintptr_t EntryPoint, uintptr_t BaseAddress)
	{
		char *aux_platform = (char *)vma->RequestPages(1, true); /* TODO: 4KiB is too much for this */
		strcpy(aux_platform, "x86_64");

		void *execfn_str = vma->RequestPages(TO_PAGES(fd->Path.size() + 1), true);
		strcpy((char *)execfn_str, fd->Path.c_str());
		void *at_random = vma->RequestPages(1, true);
		*(uintptr_t *)at_random = Random::rand16();

		Elfauxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_PLATFORM, .a_un = {.a_val = (uintptr_t)aux_platform}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_EXECFN, .a_un = {.a_val = (uintptr_t)execfn_str}}});
		// AT_HWCAP2 26
		Elfauxv.push_back({.archaux = {.a_type = AT_RANDOM, .a_un = {.a_val = (uintptr_t)at_random}}});
		Elfauxv.push_back({.archaux = {.a_type = AT_SECURE, .a_un = {.a_val = (uintptr_t)0}}}); /* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_EGID, .a_un = {.a_val = (uintptr_t)0}}});	/* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_GID, .a_un = {.a_val = (uintptr_t)0}}});	/* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_EUID, .a_un = {.a_val = (uintptr_t)0}}});	/* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_UID, .a_un = {.a_val = (uintptr_t)0}}});	/* FIXME */
		Elfauxv.push_back({.archaux = {.a_type = AT_ENTRY, .a_un = {.a_val = (uintptr_t)EntryPoint}}});
		// AT_FLAGS 8
		Elfauxv.push_back({.archaux = {.a_type = AT_BASE, .a_un = {.a_val = (uintptr_t)BaseAddress}}});

		if (ELFProgramHeaders)
		{
			Elfauxv.push_back({.archaux = {.a_type = AT_PHNUM, .a_un = {.a_val = (uintptr_t)ELFHeader.e_phnum}}});
			Elfauxv.push_back({.archaux = {.a_type = AT_PHENT, .a_un = {.a_val = (uintptr_t)ELFHeader.e_phentsize}}});
			Elfauxv.push_back({.archaux = {.a_type = AT_PHDR, .a_un = {.a_val = (uintptr_t)ELFProgramHeaders}}});
		}

		// AT_CLKTCK 17
		Elfauxv.push_back({.archaux = {.a_type = AT_PAGESZ, .a_un = {.a_val = (uintptr_t)PAGE_SIZE}}});
		// AT_HWCAP 16
		// AT_SYSINFO_EHDR 33
		// AT_MINSIGSTKSZ 51

#ifdef DEBUG
		for (auto var : Elfauxv)
		{
			debug("auxv: %ld %#lx",
				  var.archaux.a_type,
				  var.archaux.a_un.a_val);
		}
#endif
	}

	void ELFObject::LoadSegments(FileNode *fd, PCB *TargetProcess, Elf_Ehdr &ELFHeader, uintptr_t &BaseAddress)
	{
		Memory::Virtual vmm(TargetProcess->PageTable);
		Memory::VirtualMemoryArea *vma = TargetProcess->vma;
		Elf_Phdr ProgramBreakHeader{};
		Elf_Phdr ProgramHeader;

		if (ELFHeader.e_type == ET_DYN)
		{
			size_t SegmentsSize = 0;
			for (Elf_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				fd->Read(&ProgramHeader, sizeof(Elf_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf_Phdr)));

				if (ProgramHeader.p_type == PT_LOAD || ProgramHeader.p_type == PT_DYNAMIC)
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
			void *SegmentsAddress = vma->RequestPages(TO_PAGES(SegmentsSize) + 1, true);
			BaseAddress = (uintptr_t)SegmentsAddress;
			debug("BaseAddress: %#lx, End: %#lx (%#lx)", BaseAddress, BaseAddress + FROM_PAGES(TO_PAGES(SegmentsSize)), SegmentsSize);
			ProgramBreakHeader.p_vaddr += BaseAddress;

			for (Elf_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				fd->Read(&ProgramHeader, sizeof(Elf_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf_Phdr)));

				switch (ProgramHeader.p_type)
				{
				case PT_LOAD:
				{
					/* Because this is ET_DYN, we can load the segments
						anywhere we want. */
					uintptr_t SegmentDestination = BaseAddress + ProgramHeader.p_vaddr;

					if (ProgramHeader.p_memsz == 0)
						continue;

					debug("Copying PT_LOAD to %#lx-%#lx (%ld file bytes, %ld mem bytes)",
						  SegmentDestination, SegmentDestination + ProgramHeader.p_memsz,
						  ProgramHeader.p_filesz, ProgramHeader.p_memsz);

					if (ProgramHeader.p_filesz > 0)
					{
						fd->Read(SegmentDestination, ProgramHeader.p_filesz, ProgramHeader.p_offset);
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
						fd->Read(DynamicSegmentDestination, ProgramHeader.p_filesz, ProgramHeader.p_offset);
					}

					if (ProgramHeader.p_memsz - ProgramHeader.p_filesz > 0)
					{
						void *zAddr = (void *)(DynamicSegmentDestination + ProgramHeader.p_filesz);
						memset(zAddr, 0, ProgramHeader.p_memsz - ProgramHeader.p_filesz);
					}
					break;
				}
				case PT_PHDR:
				{
					ELFProgramHeaders = (void *)(BaseAddress + ProgramHeader.p_vaddr);
					debug("ELFProgramHeaders: %#lx", ELFProgramHeaders);
					break;
				}
				case 0x6474E550: /* PT_GNU_EH_FRAME */
				{
					fixme("PT_GNU_EH_FRAME");
					break;
				}
				case 0x6474e551: /* PT_GNU_STACK */
				{
					fixme("PT_GNU_STACK");
					break;
				}
				case 0x6474e552: /* PT_GNU_RELRO */
				{
					fixme("PT_GNU_RELRO");
					break;
				}
				case 0x6474e553: /* PT_GNU_PROPERTY */
				{
					fixme("PT_GNU_PROPERTY");
					break;
				}
				case PT_INTERP:
					break;
				default:
				{
					fixme("Unhandled program header type: %#lx",
						  ProgramHeader.p_type);
					break;
				}
				}
			}

			if (!ELFProgramHeaders)
				ELFProgramHeaders = (void *)(BaseAddress + ELFHeader.e_phoff);
		}
		else if (ELFHeader.e_type == ET_EXEC)
		{
			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				fd->Read(&ProgramHeader, sizeof(Elf_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf_Phdr)));
				switch (ProgramHeader.p_type)
				{
				case PT_LOAD:
				{
					if (ProgramHeader.p_memsz == 0)
						continue;

					if (BaseAddress == 0)
						BaseAddress = ALIGN_DOWN(ProgramHeader.p_vaddr, PAGE_SIZE);

					void *pAddr = vma->RequestPages(TO_PAGES(ProgramHeader.p_memsz + (ProgramHeader.p_vaddr % PAGE_SIZE)), true);
					void *vAddr = (void *)ALIGN_DOWN(ProgramHeader.p_vaddr, PAGE_SIZE);
					uintptr_t destOffset = ProgramHeader.p_vaddr - uintptr_t(vAddr);

					size_t totalSize = ALIGN_UP(destOffset + ProgramHeader.p_memsz, PAGE_SIZE);
					vmm.Map(vAddr, pAddr, totalSize, Memory::RW | Memory::US);

					debug("Mapped %#lx-%#lx to %#lx-%#lx (%#lx bytes)",
						  uintptr_t(pAddr), uintptr_t(pAddr) + totalSize,
						  uintptr_t(vAddr), uintptr_t(vAddr) + totalSize, totalSize);
					debug("Segment Offset is %#lx", destOffset);

					debug("Copying PT_LOAD to p: %#lx-%#lx; v: %#lx-%#lx (%ld file bytes, %ld mem bytes)",
						  uintptr_t(pAddr) + destOffset,
						  uintptr_t(pAddr) + destOffset + ProgramHeader.p_memsz,
						  ProgramHeader.p_vaddr,
						  ProgramHeader.p_vaddr + ProgramHeader.p_memsz,
						  ProgramHeader.p_filesz, ProgramHeader.p_memsz);

					if (ProgramHeader.p_filesz > 0)
					{
						debug("%d %#lx %d", ProgramHeader.p_offset, (uint8_t *)pAddr + destOffset, ProgramHeader.p_filesz);
						fd->Read((uint8_t *)pAddr + destOffset, ProgramHeader.p_filesz, ProgramHeader.p_offset);
					}

					if (ProgramHeader.p_memsz - ProgramHeader.p_filesz > 0)
					{
						void *zAddr = (void *)(uintptr_t(pAddr) + destOffset + ProgramHeader.p_filesz);

						debug("Zeroing %d bytes at %#lx (%#lx-%#lx)",
							  ProgramHeader.p_memsz - ProgramHeader.p_filesz, zAddr,
							  ProgramHeader.p_vaddr + ProgramHeader.p_filesz,
							  ProgramHeader.p_vaddr + ProgramHeader.p_memsz);

						memset(zAddr, 0, ProgramHeader.p_memsz - ProgramHeader.p_filesz);
					}
					ProgramBreakHeader = ProgramHeader;
					break;
				}
				case PT_NOTE:
				{
					Elf_Nhdr NoteHeader;
					fd->Read(&NoteHeader, sizeof(Elf_Nhdr), ProgramHeader.p_offset);

					switch (NoteHeader.n_type)
					{
					case NT_PRSTATUS:
					{
						Elf_Prstatus prstatus;
						fd->Read(&prstatus, sizeof(Elf_Prstatus), ProgramHeader.p_offset + sizeof(Elf_Nhdr));
						debug("PRSTATUS: %#lx", prstatus.pr_reg[0]);
						break;
					}
					case NT_PRPSINFO:
					{
						Elf_Prpsinfo prpsinfo;
						fd->Read(&prpsinfo, sizeof(Elf_Prpsinfo), ProgramHeader.p_offset + sizeof(Elf_Nhdr));
						debug("PRPSINFO: %s", prpsinfo.pr_fname);
						break;
					}
					case NT_PLATFORM:
					{
						char platform[256];
						fd->Read(&platform, sizeof(platform), ProgramHeader.p_offset + sizeof(Elf_Nhdr));
						debug("PLATFORM: %s", platform);
						break;
					}
					case NT_AUXV:
					{
						Elf_auxv_t auxv;
						fd->Read(&auxv, sizeof(Elf_auxv_t), ProgramHeader.p_offset + sizeof(Elf_Nhdr));
						debug("AUXV: %#lx", auxv.a_un.a_val);
						break;
					}
					default:
					{
						fixme("Unhandled note type: %#lx", NoteHeader.n_type);
						break;
					}
					}
					break;
				}
				case PT_TLS:
				{
					size_t tlsSize = ProgramHeader.p_memsz;
					debug("TLS Size: %ld (%ld pages)",
						  tlsSize, TO_PAGES(tlsSize));
					void *tlsMemory = vma->RequestPages(TO_PAGES(tlsSize));
					fd->Read(tlsMemory, tlsSize, ProgramHeader.p_offset);
					TargetProcess->TLS = {
						.pBase = uintptr_t(tlsMemory),
						.vBase = ProgramHeader.p_vaddr,
						.Align = ProgramHeader.p_align,
						.Size = ProgramHeader.p_memsz,
						.fSize = ProgramHeader.p_filesz,
					};
					break;
				}
				case PT_PHDR:
				{
					ELFProgramHeaders = (void *)ProgramHeader.p_vaddr;
					debug("ELFProgramHeaders: %#lx", ELFProgramHeaders);
					break;
				}
				case PT_GNU_EH_FRAME:
				{
					fixme("PT_GNU_EH_FRAME");
					break;
				}
				case PT_GNU_STACK:
				{
					Elf_Phdr gnuStack = ProgramHeader;
					fixme("EXSTACK: %d", gnuStack.p_flags & PF_X);
					break;
				}
				case PT_GNU_RELRO:
				{
					fixme("PT_GNU_RELRO");
					break;
				}
				case PT_GNU_PROPERTY:
				{
					Elf_Nhdr NoteHeader;
					fd->Read(&NoteHeader, sizeof(Elf_Nhdr), ProgramHeader.p_offset);

					if (NoteHeader.n_type == NT_GNU_PROPERTY_TYPE_0)
					{
						char noteName[0x400];
						fd->Read(noteName, NoteHeader.n_namesz, ProgramHeader.p_offset + sizeof(Elf_Nhdr));
						noteName[NoteHeader.n_namesz - 1] = '\0';

						if (strcmp(noteName, "GNU") == 0)
						{
							debug("GNU Property Note found");
						}
						else
						{
							warn("Unexpected note name in PT_GNU_PROPERTY: %s", noteName);
						}
					}
					else
					{
						warn("Unhandled note type in PT_GNU_PROPERTY: %#lx", NoteHeader.n_type);
					}
					break;
				}
				case PT_INTERP:
					break;
				case PT_LOPROC ... PT_HIPROC:
				{
					debug("i guess i ignore this? %#lx", ProgramHeader.p_type);
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

			if (!ELFProgramHeaders)
				fixme("ELFProgramHeaders is null");
		}

		/* Set program break */
		uintptr_t ProgramBreak = ROUND_UP(ProgramBreakHeader.p_vaddr + ProgramBreakHeader.p_memsz, PAGE_SIZE);
		TargetProcess->ProgramBreak->InitBrk(ProgramBreak);
	}

	void ELFObject::LoadExec(FileNode *fd, PCB *TargetProcess)
	{
		Elf_Ehdr ehdr{};
		fd->Read(&ehdr, sizeof(Elf_Ehdr), 0);
		uintptr_t entry = ehdr.e_entry;
		debug("Entry point is %#lx", entry);

		Memory::Virtual vmm(TargetProcess->PageTable);
		Memory::VirtualMemoryArea *vma = TargetProcess->vma;
		debug("Target process page table is %#lx", TargetProcess->PageTable);

		uintptr_t base = 0;
		this->LoadSegments(fd, TargetProcess, ehdr, base);

		debug("Entry Point: %#lx", entry);

		this->GenerateAuxiliaryVector(vma, fd, ehdr, entry, 0);

		this->ip = entry;
		this->IsElfValid = true;

#ifdef DEBUG
		std::string sanitizedPath = fd->Path;
		size_t pos = sanitizedPath.find("\x06root-0\x06");
		if (pos != std::string::npos)
			sanitizedPath.erase(pos, std::string("\x06root-0\x06").length());
		debug("gdb: \"-exec add-symbol-file-all /workspaces/Fennix/tmp_rootfs%s %#lx\" entry:%#lx", sanitizedPath.c_str(), base, entry);
#endif
	}

	void ELFObject::LoadDyn(FileNode *fd, PCB *TargetProcess)
	{
		Elf_Ehdr ehdr{};
		fd->Read(&ehdr, sizeof(Elf_Ehdr), 0);
		uintptr_t entry = ehdr.e_entry;
		debug("Entry point is %#lx", entry);

		Memory::Virtual vmm(TargetProcess->PageTable);
		Memory::VirtualMemoryArea *vma = TargetProcess->vma;
		uintptr_t base = 0;
		this->LoadSegments(fd, TargetProcess, ehdr, base);
		entry += base;
		debug("The new ep is %#lx", entry);

		/* ------------------------------------------------------------------------ */

		debug("Entry Point: %#lx", entry);

		this->GenerateAuxiliaryVector(vma, fd, ehdr, entry, base);

		this->ip = entry;
		this->IsElfValid = true;

#ifdef DEBUG
		std::string sanitizedPath = fd->Path;
		size_t pos = sanitizedPath.find("\x06root-0\x06");
		if (pos != std::string::npos)
			sanitizedPath.erase(pos, std::string("\x06root-0\x06").length());
		debug("gdb: \"-exec add-symbol-file-all /workspaces/Fennix/tmp_rootfs%s %#lx\" entry:%#lx", sanitizedPath.c_str(), base, entry);
#endif

		std::vector<Elf_Phdr> interpVec = ELFGetSymbolType(fd, PT_INTERP);
		if (interpVec.empty())
		{
			debug("No interpreter found");
			return;
		}

		Elf_Phdr interp = interpVec.front();
		std::string interpreterPath;
		interpreterPath.resize(256);
		fd->Read(interpreterPath.data(), 256, interp.p_offset);
		debug("Interpreter: %s", interpreterPath.c_str());

		FileNode *ifd = fs->GetByPath(interpreterPath.c_str(), TargetProcess->Info.RootNode);
		if (ifd == nullptr)
		{
			warn("Failed to open interpreter file: %s", interpreterPath.c_str());
			return;
		}

		if (ifd->IsSymbolicLink())
		{
			char buffer[512];
			ifd->ReadLink(buffer, sizeof(buffer));
			ifd = fs->GetByPath(buffer, ifd->Parent);
		}

		debug("ifd: %p, interpreter: %s", ifd, interpreterPath.c_str());
		if (GetBinaryType(interpreterPath) != BinTypeELF)
		{
			warn("Interpreter %s is not an ELF file", interpreterPath.c_str());
			return;
		}

		LoadInterpreter(ifd, TargetProcess);
	}

	bool ELFObject::LoadInterpreter(FileNode *fd, PCB *TargetProcess)
	{
		Elf_Ehdr ehdr;
		fd->Read(&ehdr, sizeof(Elf_Ehdr), 0);

		switch (ehdr.e_type)
		{
		case ET_EXEC:
			assert(ehdr.e_type != ET_EXEC);
			break;
		case ET_DYN:
		{
			uintptr_t base = 0;
			this->LoadSegments(fd, TargetProcess, ehdr, base);
			this->ip = base + ehdr.e_entry;
			for (auto &&aux : Elfauxv)
			{
				if (aux.archaux.a_type != AT_BASE)
					continue;

				aux.archaux.a_un.a_val = base;
				break;
			}

#ifdef DEBUG
			std::string sanitizedPath = fd->Path;
			size_t pos = sanitizedPath.find("\x06root-0\x06");
			if (pos != std::string::npos)
				sanitizedPath.erase(pos, std::string("\x06root-0\x06").length());
			debug("gdb: \"-exec add-symbol-file-all /workspaces/Fennix/tmp_rootfs%s %#lx\" entry:%#lx", sanitizedPath.c_str(), base, ehdr.e_entry);
#endif

			return true;
		}
		case ET_CORE:
		case ET_REL:
		case ET_NONE:
		{
			warn("Ignoring interpreter: %s (reason: ET_ is %#lx)", fd->Path.c_str(), ehdr.e_type);
			break;
		}
		default:
		{
			error("Unknown ELF Type: %d", ehdr.e_type);
			break;
		}
		}
		return false;
	}

	ELFObject::ELFObject(std::string AbsolutePath, PCB *TargetProcess, const char **argv, const char **envp)
	{
		if (GetBinaryType(AbsolutePath) != BinaryType::BinTypeELF)
		{
			error("%s is not an ELF file or is invalid.", AbsolutePath.c_str());
			return;
		}

		FileNode *fd = fs->GetByPath(AbsolutePath.c_str(), TargetProcess->Info.RootNode);
		if (fd == nullptr)
		{
			error("Failed to open %s, errno: %d", AbsolutePath.c_str(), fd);
			return;
		}

		if (fd->IsSymbolicLink())
		{
			char buffer[512];
			fd->ReadLink(buffer, sizeof(buffer));
			fd = fs->GetByPath(buffer, fd->Parent);
		}

		debug("Opened %s", AbsolutePath.c_str());

		int argc = 0;
		int envc = 0;

		while (argv[argc] != nullptr)
			argc++;
		while (envp[envc] != nullptr)
			envc++;

		Elf_Ehdr ehdr{};
		fd->Read(&ehdr, sizeof(Elf_Ehdr), 0);

		// ELFargv = new const char *[argc + 2];
		size_t argv_size = argc + 2 * sizeof(char *);
		ELFargv = (const char **)TargetProcess->vma->RequestPages(TO_PAGES(argv_size));

		for (int i = 0; i < argc; i++)
		{
			size_t arg_size = strlen(argv[i]) + 1;
			ELFargv[i] = (const char *)TargetProcess->vma->RequestPages(TO_PAGES(arg_size));
			strcpy((char *)ELFargv[i], argv[i]);
		}
		ELFargv[argc] = nullptr;

		// ELFenvp = new const char *[envc + 1];
		size_t envp_size = envc + 1 * sizeof(char *);
		ELFenvp = (const char **)TargetProcess->vma->RequestPages(TO_PAGES(envp_size));
		for (int i = 0; i < envc; i++)
		{
			assert(envp[i] != nullptr);
			size_t env_size = strlen(envp[i]) + 1;
			ELFenvp[i] = (const char *)TargetProcess->vma->RequestPages(TO_PAGES(env_size));
			strcpy((char *)ELFenvp[i], envp[i]);
		}
		ELFenvp[envc] = nullptr;

		switch (ehdr.e_type)
		{
		case ET_REL:
		{
			fixme("ET_REL not implemented");
			break;
		}
		case ET_EXEC:
		{
			switch (ehdr.e_machine)
			{
			case EM_386:
			case EM_X86_64:
			case EM_ARM:
			case EM_AARCH64:
				this->LoadExec(fd, TargetProcess);
				break;
			default:
				error("Unknown architecture: %d", ehdr.e_machine);
				break;
			}
			break;
		}
		case ET_DYN:
		{
			switch (ehdr.e_machine)
			{
			case EM_386:
			case EM_X86_64:
			case EM_ARM:
			case EM_AARCH64:
				this->LoadDyn(fd, TargetProcess);
				break;
			default:
				error("Unknown architecture: %d", ehdr.e_machine);
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
			error("Unknown ELF Type: %d", ehdr.e_type);
			break;
		}
		}
	}

	ELFObject::~ELFObject()
	{
	}
}
