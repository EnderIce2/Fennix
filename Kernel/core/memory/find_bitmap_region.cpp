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
	__no_sanitize("alignment") void Physical::FindBitmapRegion(uintptr_t &BitmapAddress, size_t &BitmapAddressSize)
	{
		size_t BitmapSize = (size_t)(bInfo.Memory.Size / PAGE_SIZE) / 8 + 1;

		uintptr_t KernelStart = (uintptr_t)bInfo.Kernel.PhysicalBase;
		uintptr_t KernelEnd = (uintptr_t)bInfo.Kernel.PhysicalBase + bInfo.Kernel.Size;

		uintptr_t SectionsStart = 0x0;
		uintptr_t SectionsEnd = 0x0;

		uintptr_t Symbols = 0x0;
		uintptr_t StringAddress = 0x0;
		size_t SymbolSize = 0;
		size_t StringSize = 0;

		uintptr_t RSDPStart = 0x0;
		uintptr_t RSDPEnd = 0x0;

		if (bInfo.Kernel.Symbols.Num && bInfo.Kernel.Symbols.EntSize && bInfo.Kernel.Symbols.Shndx)
		{
			char *sections = r_cst(char *, bInfo.Kernel.Symbols.Sections);

			SectionsStart = (uintptr_t)sections;
			SectionsEnd = (uintptr_t)sections + bInfo.Kernel.Symbols.EntSize * bInfo.Kernel.Symbols.Num;

			for (size_t i = 0; i < bInfo.Kernel.Symbols.Num; ++i)
			{
				Elf_Shdr *sym = (Elf_Shdr *)&sections[bInfo.Kernel.Symbols.EntSize * i];
				Elf_Shdr *str = (Elf_Shdr *)&sections[bInfo.Kernel.Symbols.EntSize * sym->sh_link];

				if (sym->sh_type == SHT_SYMTAB && str->sh_type == SHT_STRTAB)
				{
					Symbols = (uintptr_t)sym->sh_addr;
					StringAddress = (uintptr_t)str->sh_addr;
					SymbolSize = (size_t)sym->sh_size;
					StringSize = (size_t)str->sh_size;
					break;
				}
			}
		}

#if defined(__amd64__) || defined(__i386__)
		if (bInfo.RSDP)
		{
			RSDPStart = (uintptr_t)bInfo.RSDP;
			RSDPEnd = (uintptr_t)bInfo.RSDP + sizeof(BootInfo::RSDPInfo);

#ifdef DEBUG
			ACPI::ACPI::ACPIHeader *ACPIPtr;
			bool XSDT = false;

			if (bInfo.RSDP->Revision >= 2 && bInfo.RSDP->XSDTAddress)
			{
				ACPIPtr = (ACPI::ACPI::ACPIHeader *)bInfo.RSDP->XSDTAddress;
				XSDT = true;
			}
			else
				ACPIPtr = (ACPI::ACPI::ACPIHeader *)(uintptr_t)bInfo.RSDP->RSDTAddress;

			if (Memory::Virtual().Check(ACPIPtr))
			{
				size_t TableSize = ((ACPIPtr->Length - sizeof(ACPI::ACPI::ACPIHeader)) / (XSDT ? 8 : 4));
				debug("There are %d ACPI tables", TableSize);
			}
#endif
		}
#elif defined(__aarch64__)
#endif

		for (uint64_t i = 0; i < bInfo.Memory.Entries; i++)
		{
			if (bInfo.Memory.Entry[i].Type == Usable)
			{
				uintptr_t RegionAddress = (uintptr_t)bInfo.Memory.Entry[i].BaseAddress;
				uintptr_t RegionSize = bInfo.Memory.Entry[i].Length;

				/* We don't want to use the first 1MB of memory. */
				if (RegionAddress <= 0xFFFFF)
					continue;

				if ((BitmapSize + 0x100) > RegionSize)
				{
					debug("Region %p-%p (%d MiB) is too small for bitmap.",
						  (void *)RegionAddress,
						  (void *)(RegionAddress + RegionSize),
						  TO_MiB(RegionSize));
					continue;
				}

				BitmapAddress = RegionAddress;
				BitmapAddressSize = RegionSize;

				struct AddrRange
				{
					uintptr_t Start;
					uintptr_t End;
					char Description[16];
				};

				auto SortAddresses = [](AddrRange *Array, size_t n)
				{
					size_t MinimumIndex;
					for (size_t i = 0; i < n - 1; i++)
					{
						MinimumIndex = i;
						for (size_t j = i + 1; j < n; j++)
							if (Array[j].Start < Array[MinimumIndex].Start)
								MinimumIndex = j;

						AddrRange tmp = Array[MinimumIndex];
						Array[MinimumIndex] = Array[i];
						Array[i] = tmp;
					}
				};

				AddrRange PtrArray[] =
					{
						{KernelStart, KernelEnd, "kernel"},
						{SectionsStart, SectionsEnd, "sections"},
						{Symbols, Symbols + SymbolSize, "symbols"},
						{StringAddress, StringAddress + StringSize, "string"},
						{RSDPStart, RSDPEnd, "rsdp"},
						{(uintptr_t)bInfo.Kernel.FileBase, (uintptr_t)bInfo.Kernel.FileBase + bInfo.Kernel.Size, "file"},
						{(uintptr_t)bInfo.Modules[0].Address, (uintptr_t)bInfo.Modules[0].Address + bInfo.Modules[0].Size, "module 0"},
						{(uintptr_t)bInfo.Modules[1].Address, (uintptr_t)bInfo.Modules[1].Address + bInfo.Modules[1].Size, "module 1"},
						{(uintptr_t)bInfo.Modules[2].Address, (uintptr_t)bInfo.Modules[2].Address + bInfo.Modules[2].Size, "module 2"},
						{(uintptr_t)bInfo.Modules[3].Address, (uintptr_t)bInfo.Modules[3].Address + bInfo.Modules[3].Size, "module 3"},
						/* MAX_MODULES == 4 */
					};

				SortAddresses(PtrArray, sizeof(PtrArray) / sizeof(PtrArray[0]));

				uintptr_t MaxEnd = RegionAddress;
				for (size_t i = 0; i < sizeof(PtrArray) / sizeof(PtrArray[0]); i++)
				{
					if (PtrArray[i].Start == 0x0)
					{
						debug("skipping %#lx %zu %s", PtrArray[i].Start, i, PtrArray[i].Description);
						continue;
					}

					uintptr_t Start = PtrArray[i].Start;
					uintptr_t End = PtrArray[i].End;
					debug("[%s] %#lx - %#lx", PtrArray[i].Description, Start, End);

					if ((Start < (RegionAddress + RegionSize)) && (End > RegionAddress))
					{
						if (End > MaxEnd)
							MaxEnd = End;
					}
				}

				if (MaxEnd >= RegionAddress && MaxEnd < (RegionAddress + RegionSize))
				{
					BitmapAddress = MaxEnd;
					BitmapAddressSize = RegionAddress + RegionSize - MaxEnd;

					debug("BitmapAddress = %#lx; Size = %zu", BitmapAddress, BitmapAddressSize);

					if ((BitmapSize + 0x100) > BitmapAddressSize)
					{
						debug("Region %#lx-%#lx (%d MiB) is too small for bitmap.", BitmapAddress, BitmapAddress + BitmapAddressSize, TO_MiB(BitmapAddressSize));
						continue;
					}

					debug("Found free memory for bitmap: %#lx (%d MiB)", BitmapAddress, TO_MiB(BitmapAddressSize));
					break;
				}
			}
		}
	}
}
