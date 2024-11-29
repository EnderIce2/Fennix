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

#include <memory.hpp>

#include <acpi.hpp>
#include <debug.h>
#include <elf.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "../../kernel.h"

namespace Memory
{
	__no_sanitize("alignment") void Physical::ReserveEssentials()
	{
		debug("Reserving pages...");
		/* The bootloader won't give us the entire mapping, so we
		   reserve everything and then unreserve the usable pages. */
		this->ReservePages(0, TO_PAGES(bInfo.Memory.Size));
		debug("Unreserving usable pages...");

		for (uint64_t i = 0; i < bInfo.Memory.Entries; i++)
		{
			if (bInfo.Memory.Entry[i].Type == Usable)
			{
				if (uintptr_t(bInfo.Memory.Entry[i].BaseAddress) <= 0xFFFFF)
					continue;

				this->UnreservePages(bInfo.Memory.Entry[i].BaseAddress,
									 TO_PAGES(bInfo.Memory.Entry[i].Length));
			}
		}

		debug("Reserving 0x0-0xFFFFF range...");
		// this->ReservePage((void *)0x0);						/* Trampoline stack, gdt, idt, etc... */
		// this->ReservePages((void *)0x2000, 4);				/* TRAMPOLINE_START */

		/* Reserve the lower part of memory. (0x0-0xFFFFF)
		   This includes: BIOS, EBDA, VGA, SMP, etc...
		   https://wiki.osdev.org/Memory_Map_(x86)
		*/
		this->ReservePages((void *)0x0, TO_PAGES(0xFFFFF));

		debug("Reserving bitmap region %#lx-%#lx...",
			  PageBitmap.Buffer,
			  (void *)((uintptr_t)PageBitmap.Buffer + PageBitmap.Size));

		this->ReservePages(PageBitmap.Buffer, TO_PAGES(PageBitmap.Size));

		debug("Reserving kernel physical region %#lx-%#lx...",
			  bInfo.Kernel.PhysicalBase,
			  (void *)((uintptr_t)bInfo.Kernel.PhysicalBase + bInfo.Kernel.Size));

		this->ReservePages(bInfo.Kernel.PhysicalBase, TO_PAGES(bInfo.Kernel.Size));

		debug("Reserving kernel file and symbols...");
		if (bInfo.Kernel.FileBase)
			this->ReservePages(bInfo.Kernel.FileBase, TO_PAGES(bInfo.Kernel.Size));

		if (bInfo.Kernel.Symbols.Num &&
			bInfo.Kernel.Symbols.EntSize &&
			bInfo.Kernel.Symbols.Shndx)
		{
			char *sections = r_cst(char *, bInfo.Kernel.Symbols.Sections);
			debug("Reserving sections region %#lx-%#lx...",
				  sections,
				  (void *)((uintptr_t)sections + bInfo.Kernel.Symbols.EntSize *
													 bInfo.Kernel.Symbols.Num));

			this->ReservePages(sections, TO_PAGES(bInfo.Kernel.Symbols.EntSize *
												  bInfo.Kernel.Symbols.Num));

			Elf_Sym *Symbols = nullptr;
			uint8_t *StringAddress = nullptr;

#if defined(__amd64__) || defined(__aarch64__)
			Elf64_Xword SymbolSize = 0;
			Elf64_Xword StringSize = 0;
#elif defined(__i386__)
			Elf32_Word SymbolSize = 0;
			Elf32_Word StringSize = 0;
#endif

			for (size_t i = 0; i < bInfo.Kernel.Symbols.Num; ++i)
			{
				Elf_Shdr *sym = (Elf_Shdr *)&sections[bInfo.Kernel.Symbols.EntSize * i];
				Elf_Shdr *str = (Elf_Shdr *)&sections[bInfo.Kernel.Symbols.EntSize *
													  sym->sh_link];

				if (sym->sh_type == SHT_SYMTAB &&
					str->sh_type == SHT_STRTAB)
				{
					Symbols = (Elf_Sym *)sym->sh_addr;
					StringAddress = (uint8_t *)str->sh_addr;
					SymbolSize = (int)sym->sh_size;
					StringSize = (int)str->sh_size;
					debug("Symbol table found, %d entries (%ld KiB)",
						  SymbolSize / sym->sh_entsize,
						  TO_KiB(SymbolSize));
					this->ReservePages(Symbols, TO_PAGES(SymbolSize));
					break;
				}
			}

			if (Symbols)
			{
				debug("Reserving symbol table region %#lx-%#lx...",
					  Symbols, (void *)((uintptr_t)Symbols + SymbolSize));
				this->ReservePages(Symbols, TO_PAGES(SymbolSize));
			}

			if (StringAddress)
			{
				debug("Reserving string table region %#lx-%#lx...",
					  StringAddress, (void *)((uintptr_t)StringAddress + StringSize));
				this->ReservePages(StringAddress, TO_PAGES(StringSize));
			}
		}

		debug("Reserving kernel modules...");

		for (uint64_t i = 0; i < MAX_MODULES; i++)
		{
			if (bInfo.Modules[i].Address == 0x0)
				continue;

			debug("Reserving module %s (%#lx-%#lx)...", bInfo.Modules[i].CommandLine,
				  bInfo.Modules[i].Address,
				  (void *)((uintptr_t)bInfo.Modules[i].Address + bInfo.Modules[i].Size));

			this->ReservePages((void *)bInfo.Modules[i].Address,
							   TO_PAGES(bInfo.Modules[i].Size));
		}

#if defined(__amd64__) || defined(__i386__)
		if (bInfo.RSDP)
		{
			debug("Reserving RSDT region %#lx-%#lx...", bInfo.RSDP,
				  (void *)((uintptr_t)bInfo.RSDP + sizeof(BootInfo::RSDPInfo)));

			this->ReservePages(bInfo.RSDP, TO_PAGES(sizeof(BootInfo::RSDPInfo)));

			ACPI::ACPI::ACPIHeader *ACPIPtr;
			bool XSDT = false;

			if (bInfo.RSDP->Revision >= 2 && bInfo.RSDP->XSDTAddress)
			{
				ACPIPtr = (ACPI::ACPI::ACPIHeader *)bInfo.RSDP->XSDTAddress;
				XSDT = true;
			}
			else
				ACPIPtr = (ACPI::ACPI::ACPIHeader *)(uintptr_t)bInfo.RSDP->RSDTAddress;

			debug("Reserving RSDT...");
			this->ReservePages((void *)bInfo.RSDP, TO_PAGES(sizeof(BootInfo::RSDPInfo)));

			if (!Memory::Virtual().Check(ACPIPtr))
			{
				error("ACPI table is located in an unmapped region.");
				return;
			}

			size_t TableSize = ((ACPIPtr->Length - sizeof(ACPI::ACPI::ACPIHeader)) /
								(XSDT ? 8 : 4));
			debug("Reserving %d ACPI tables...", TableSize);

			for (size_t t = 0; t < TableSize; t++)
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
				// TODO: Should I be concerned about unaligned memory access?
				ACPI::ACPI::ACPIHeader *SDTHdr = nullptr;
				if (XSDT)
					SDTHdr =
						(ACPI::ACPI::ACPIHeader *)(*(uint64_t *)((uint64_t)ACPIPtr +
																 sizeof(ACPI::ACPI::ACPIHeader) +
																 (t * 8)));
				else
					SDTHdr =
						(ACPI::ACPI::ACPIHeader *)(*(uint32_t *)((uint64_t)ACPIPtr +
																 sizeof(ACPI::ACPI::ACPIHeader) +
																 (t * 4)));
#pragma GCC diagnostic pop
				this->ReservePages(SDTHdr, TO_PAGES(SDTHdr->Length));
			}
		}
#elif defined(__aarch64__)
#endif
	}
}
