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

#include <driver.hpp>

#include <interface/driver.h>
#include <memory.hpp>
#include <ints.hpp>
#include <task.hpp>
#include <printf.h>
#include <exec.hpp>
#include <cwalk.h>
#include <md5.h>

#include "../../kernel.h"

using namespace vfs;

namespace Driver
{
	void Manager::PreloadDrivers()
	{
		debug("Initializing driver manager");
		const char *DriverDirectory = Config.DriverDirectory;
		FileNode *drvDirNode = fs->GetByPath(DriverDirectory, nullptr);
		if (!drvDirNode)
		{
			error("Failed to open driver directory %s", DriverDirectory);
			KPrint("Failed to open driver directory %s", DriverDirectory);
			return;
		}

		foreach (const auto &drvNode in drvDirNode->Children)
		{
			debug("Checking driver %s", drvNode->Path.c_str());
			if (!drvNode->IsRegularFile())
				continue;

			if (Execute::GetBinaryType(drvNode->Path) != Execute::BinTypeELF)
			{
				error("Driver %s is not an ELF binary", drvNode->Path.c_str());
				continue;
			}

			Memory::VirtualMemoryArea *dVma = new Memory::VirtualMemoryArea(thisProcess->PageTable);

			uintptr_t EntryPoint, BaseAddress;
			int err = this->LoadDriverFile(EntryPoint, BaseAddress, dVma, drvNode);
			debug("err = %d (%s)", err, strerror(err));
			if (err != 0)
			{
				error("Failed to load driver %s: %s",
					  drvNode->Path.c_str(), strerror(err));

				delete dVma;
				continue;
			}

			Drivers[DriverIDCounter++] = {
				.BaseAddress = BaseAddress,
				.EntryPoint = EntryPoint,
				.vma = dVma,
				.Path = drvNode->Path,
				.InterruptHandlers = new std::unordered_map<uint8_t, void *>};

			dev_t countr = DriverIDCounter - 1;
			const char *drvName;
			size_t drvNameLen;
			cwk_path_get_basename(drvNode->Path.c_str(), &drvName, &drvNameLen);
			strncpy(Drivers[countr].Name, drvName, sizeof(Drivers[countr].Name));
		}
	}

	void Manager::LoadAllDrivers()
	{
		if (Drivers.empty())
		{
			KPrint("\eE85230No drivers to load");
			return;
		}

		foreach (auto &var in Drivers)
		{
			DriverObject *Drv = &var.second;
			size_t dapiPgs = TO_PAGES(sizeof(__driverAPI));
			__driverAPI *dApi = (__driverAPI *)Drv->vma->RequestPages(dapiPgs);
			debug("Driver API at %#lx-%#lx", dApi, dApi + sizeof(__driverAPI));

			fixme("api version");
			dApi->APIVersion.Major = 0;
			dApi->APIVersion.Minor = 0;
			dApi->APIVersion.Patch = 0;

			dApi->MajorID = var.first;
			dApi->Base = Drv->BaseAddress;
			PopulateDriverAPI(dApi);

			debug("Calling driver %s at %#lx", Drv->Path.c_str(), Drv->EntryPoint);
			int (*DrvInit)(__driverAPI *) = (int (*)(__driverAPI *))Drv->EntryPoint;
			Drv->ErrorCode = DrvInit(dApi);
			if (Drv->ErrorCode < 0)
			{
				KPrint("FATAL: _start() failed for %s: %s",
					   Drv->Name, strerror(Drv->ErrorCode));
				error("Failed to load driver %s: %s",
					  Drv->Path.c_str(), strerror(Drv->ErrorCode));

				Drv->vma->FreeAllPages();
				continue;
			}

			KPrint("Loading driver %s", Drv->Name);

			debug("Calling Probe()=%#lx on driver %s",
				  Drv->Probe, Drv->Path.c_str());
			Drv->ErrorCode = Drv->Probe();
			if (Drv->ErrorCode < 0)
			{
				KPrint("Probe() failed for %s: %s",
					   Drv->Name, strerror(Drv->ErrorCode));
				error("Failed to probe driver %s: %s",
					  Drv->Path.c_str(), strerror(Drv->ErrorCode));

				Drv->vma->FreeAllPages();
				continue;
			}

			debug("Calling driver Entry()=%#lx function on driver %s",
				  Drv->Entry, Drv->Path.c_str());
			Drv->ErrorCode = Drv->Entry();
			if (Drv->ErrorCode < 0)
			{
				KPrint("Entry() failed for %s: %s",
					   Drv->Name, strerror(Drv->ErrorCode));
				error("Failed to initialize driver %s: %s",
					  Drv->Path.c_str(), strerror(Drv->ErrorCode));

				Drv->vma->FreeAllPages();
				continue;
			}

			debug("Loaded driver %s", Drv->Path.c_str());
			Drv->Initialized = true;
		}
	}

	void Manager::UnloadAllDrivers()
	{
		foreach (auto &var in Drivers)
		{
			DriverObject *Drv = &var.second;
			if (!Drv->Initialized)
				continue;

			debug("Unloading driver %s", Drv->Name);
			int err = Drv->Final();
			if (err < 0)
			{
				warn("Failed to unload driver %s: %s",
					 Drv->Name, strerror(err));
			}

			if (!Drv->InterruptHandlers->empty())
			{
				foreach (auto &rInt in * Drv->InterruptHandlers)
				{
					Interrupts::RemoveHandler((void (*)(CPU::TrapFrame *))rInt.second);
				}
				Drv->InterruptHandlers->clear();
			}

			delete Drv->vma, Drv->vma = nullptr;
			delete Drv->InterruptHandlers, Drv->InterruptHandlers = nullptr;
		}
		Drivers.clear();
	}

	void Manager::Panic()
	{
		Memory::Virtual vmm;
		if (Drivers.size() == 0)
			return;

		foreach (auto Driver in Drivers)
		{
			if (!Driver.second.Initialized)
				continue;

			trace("Panic on driver %s", Driver.second.Name);
			debug("%#lx", Driver.second.Panic);

			/* Crash while probing? */
			if (Driver.second.Panic && vmm.Check((void *)Driver.second.Panic))
				Driver.second.Panic();
			else
				error("No panic function for driver %s",
					  Driver.second.Name);
		}
	}

	int Manager::LoadDriverFile(uintptr_t &EntryPoint, uintptr_t &BaseAddress,
								Memory::VirtualMemoryArea *dVma, FileNode *rDrv)
	{
		Elf64_Ehdr ELFHeader;
		rDrv->Read(&ELFHeader, sizeof(Elf64_Ehdr), 0);
		if (ELFHeader.e_type != ET_DYN)
		{
			error("Driver %s is not a shared object", rDrv->Path.c_str());
			return -ENOEXEC;
		}

		trace("Loading driver %s in memory", rDrv->Name.c_str());

		BaseAddress = 0;
		{
			Elf64_Phdr ProgramBreakHeader{};
			Elf64_Phdr ProgramHeader;

			size_t SegmentsSize = 0;
			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				rDrv->Read(&ProgramHeader, sizeof(Elf64_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)));

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
			void *SegmentsAddress = dVma->RequestPages(TO_PAGES(SegmentsSize) + 1);
			BaseAddress = (uintptr_t)SegmentsAddress;
			debug("BaseAddress: %#lx, End: %#lx (%#lx)", BaseAddress,
				  BaseAddress + FROM_PAGES(TO_PAGES(SegmentsSize)),
				  SegmentsSize);

			for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
			{
				rDrv->Read(&ProgramHeader, sizeof(Elf64_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)));

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
						rDrv->Read(SegmentDestination, ProgramHeader.p_filesz, ProgramHeader.p_offset);
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
						rDrv->Read(DynamicSegmentDestination, ProgramHeader.p_filesz, ProgramHeader.p_offset);
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
		}

		Elf64_Phdr ProgramHeader;
		for (Elf64_Half i = 0; i < ELFHeader.e_phnum; i++)
		{
			rDrv->Read(&ProgramHeader, sizeof(Elf64_Phdr), ELFHeader.e_phoff + (i * sizeof(Elf64_Phdr)));

			if (ProgramHeader.p_type == PT_DYNAMIC)
			{
				Elf64_Dyn *Dynamic = (Elf64_Dyn *)(BaseAddress + ProgramHeader.p_vaddr);
				Elf64_Dyn *RelaSize = nullptr;
				Elf64_Dyn *PltRelSize = nullptr;

				while (Dynamic->d_tag != DT_NULL)
				{
					switch (Dynamic->d_tag)
					{
					case DT_RELASZ:
						RelaSize = Dynamic;
						debug("RELA Size: %d", RelaSize->d_un.d_val / sizeof(Elf64_Rela));

						break;
					case DT_PLTRELSZ:
						PltRelSize = Dynamic;
						debug("PLTRELSZ: %d", PltRelSize->d_un.d_val / sizeof(Elf64_Rela));

						break;
					default:
						break;
					}

					Dynamic++;
				}
				Dynamic = (Elf64_Dyn *)(BaseAddress + ProgramHeader.p_vaddr);

				while (Dynamic->d_tag != DT_NULL)
				{
					switch (Dynamic->d_tag)
					{
					case DT_RELA: /* .rela.dyn */
					{
						if (!RelaSize)
						{
							error("DT_RELASZ is not set");
							break;
						}

						Elf64_Rela *Rela = (Elf64_Rela *)(BaseAddress + Dynamic->d_un.d_ptr);
						for (size_t i = 0; i < (RelaSize->d_un.d_val / sizeof(Elf64_Rela)); i++)
						{
							Elf64_Rela *r = &Rela[i];
							uintptr_t *RelocationAddress = (uintptr_t *)(BaseAddress + r->r_offset);
							uintptr_t RelocationTarget = 0;

							switch (ELF64_R_TYPE(r->r_info))
							{
							case R_X86_64_GLOB_DAT:
							case R_X86_64_JUMP_SLOT:
							{
								RelocationTarget = BaseAddress;
								break;
							}
							case R_X86_64_RELATIVE:
							case R_X86_64_64:
							{
								RelocationTarget = BaseAddress + r->r_addend;
								break;
							}
							default:
							{
								fixme("Unhandled relocation type: %#lx",
									  ELF64_R_TYPE(r->r_info));
								break;
							}
							}

							*RelocationAddress = RelocationTarget;

							debug("Relocated %#lx to %#lx",
								  r->r_offset, *RelocationAddress);
						}
						break;
					}
					case DT_PLTREL:
					{
						if (Dynamic->d_un.d_val != DT_RELA)
							error("DT_PLTREL is not DT_RELA");
						break;
					}
					case DT_JMPREL: /* .rela.plt */
					{
						if (!PltRelSize)
						{
							error("DT_PLTRELSZ is not set");
							break;
						}

						std::vector<Elf64_Dyn> SymTab = Execute::ELFGetDynamicTag_x86_64(rDrv, DT_SYMTAB);
						std::vector<Elf64_Dyn> StrTab = Execute::ELFGetDynamicTag_x86_64(rDrv, DT_STRTAB);
						Elf64_Sym *_SymTab = (Elf64_Sym *)((uintptr_t)BaseAddress + SymTab[0].d_un.d_ptr);
						char *DynStr = (char *)((uintptr_t)BaseAddress + StrTab[0].d_un.d_ptr);
						UNUSED(DynStr);

						Elf64_Rela *Rela = (Elf64_Rela *)(BaseAddress + Dynamic->d_un.d_ptr);
						for (size_t i = 0; i < (PltRelSize->d_un.d_val / sizeof(Elf64_Rela)); i++)
						{
							Elf64_Rela *r = &Rela[i];
							uintptr_t *RelocationAddress = (uintptr_t *)(BaseAddress + r->r_offset);
							uintptr_t RelocationTarget = 0;

							switch (ELF64_R_TYPE(r->r_info))
							{
							case R_X86_64_JUMP_SLOT:
							{
								Elf64_Xword SymIndex = ELF64_R_SYM(r->r_info);
								Elf64_Sym *Sym = _SymTab + SymIndex;

#ifdef DEBUG
								const char *SymbolName = DynStr + Sym->st_name;
								debug("Symbol %s at %#lx", SymbolName, Sym->st_value);
#endif

								RelocationTarget = BaseAddress + Sym->st_value;
								break;
							}
							default:
							{
								fixme("Unhandled relocation type: %#lx",
									  ELF64_R_TYPE(r->r_info));
								break;
							}
							}

							*RelocationAddress = RelocationTarget;

							debug("Relocated %#lx to %#lx",
								  r->r_offset, *RelocationAddress);
						}
						break;
					}
					case DT_SYMTAB:
					{
						fixme("DT_SYMTAB");
						break;

						std::vector<Elf64_Dyn> SymTab = Execute::ELFGetDynamicTag_x86_64(rDrv, DT_SYMTAB);
						std::vector<Elf64_Dyn> StrTab = Execute::ELFGetDynamicTag_x86_64(rDrv, DT_STRTAB);
						Elf64_Sym *_SymTab = (Elf64_Sym *)((uintptr_t)BaseAddress + SymTab[0].d_un.d_ptr);
						char *DynStr = (char *)((uintptr_t)BaseAddress + StrTab[0].d_un.d_ptr);
						UNUSED(DynStr);

						size_t symtabEntrySize = 0;
						Elf64_Dyn *entrySizeDyn = Dynamic;
						while (entrySizeDyn->d_tag != DT_NULL)
						{
							if (entrySizeDyn->d_tag == DT_SYMENT)
							{
								symtabEntrySize = entrySizeDyn->d_un.d_val;
								break;
							}
							entrySizeDyn++;
						}

						if (symtabEntrySize == 0)
						{
							fixme("No information about symbol entry size");
							break;
						}

						size_t numSymbols = Dynamic->d_un.d_val / symtabEntrySize;

						for (size_t i = 0; i < numSymbols; i++)
						{
							Elf64_Sym *s = &_SymTab[i];
							if (s->st_name == 0)
								continue;

#ifdef DEBUG
							const char *SymbolName = (const char *)(DynStr + s->st_name);
							debug("%d: Symbol %s at %#lx", i, SymbolName, s->st_value);
#endif
							/** TODO: search for symbols and link */
							/** good use but it will not work only
							 * if we specify to default visibility but
							 * this will create more issues :/ */
							// if (strcmp(SymbolName, "DriverProbe") == 0)
							// {
							// 	Drivers[DriverIDCounter].Probe = (int (*)())(BaseAddress + s->st_value);
							// 	debug("Found probe function at %#lx", Drivers[DriverIDCounter].Probe);
							// }
						}
						break;
					}
					default:
					{
						fixme("Unhandled dynamic tag: %#lx",
							  Dynamic->d_tag);
						break;
					}
					}
					Dynamic++;
				}
			}
		}

		EntryPoint = ELFHeader.e_entry;
		EntryPoint += BaseAddress;

		debug("Driver %s has entry point %#lx and base %#lx",
			  rDrv->Path.c_str(), EntryPoint, BaseAddress);

		/* FIXME: Do not add to the KernelSymbolTable! */
		// Memory::SmartHeap sh(rDrv->Size);
		// rDrv->seek(0, SEEK_SET);
		// rDrv->read((uint8_t *)sh.Get(), rDrv->Size);
		// KernelSymbolTable->AppendSymbols((uintptr_t)sh.Get(), BaseAddress);
		return 0;
	}

	Manager::Manager()
	{
	}

	Manager::~Manager()
	{
		debug("Unloading drivers");
		UnloadAllDrivers();
	}
}
