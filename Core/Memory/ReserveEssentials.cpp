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

#include <debug.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#if defined(a64)
#include "../../Architecture/amd64/acpi.hpp"
#elif defined(a32)
#include "../../Architecture/i386/acpi.hpp"
#elif defined(aa64)
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
				this->UnreservePages(bInfo.Memory.Entry[i].BaseAddress, TO_PAGES(bInfo.Memory.Entry[i].Length));
		}

		debug("Reserving 0x0-0xFFFFF range...");
		// this->ReservePage((void *)0x0);						/* Trampoline stack, gdt, idt, etc... */
		// this->ReservePages((void *)0x2000, 4);				/* TRAMPOLINE_START */

		/* Reserve the lower part of memory. (0x0-0xFFFFF)
		   This includes: BIOS, EBDA, VGA, SMP, etc...
		   https://wiki.osdev.org/Memory_Map_(x86)
		*/
		this->ReservePages((void *)0x0, TO_PAGES(0xFFFFF));

		debug("Reserving bitmap region %#lx-%#lx...", PageBitmap.Buffer, (void *)((uintptr_t)PageBitmap.Buffer + PageBitmap.Size));
		this->ReservePages(PageBitmap.Buffer, TO_PAGES(PageBitmap.Size));

		debug("Reserving kernel physical region %#lx-%#lx...", bInfo.Kernel.PhysicalBase, (void *)((uintptr_t)bInfo.Kernel.PhysicalBase + bInfo.Kernel.Size));
		this->ReservePages(bInfo.Kernel.PhysicalBase, TO_PAGES(bInfo.Kernel.Size));

		debug("Reserving kernel file and symbols...");
		if (bInfo.Kernel.FileBase)
			this->ReservePages(bInfo.Kernel.FileBase, TO_PAGES(bInfo.Kernel.Size));

		if (bInfo.Kernel.Symbols.Num && bInfo.Kernel.Symbols.EntSize && bInfo.Kernel.Symbols.Shndx)
			this->ReservePages((void *)bInfo.Kernel.Symbols.Sections, TO_PAGES(bInfo.Kernel.Symbols.Num * bInfo.Kernel.Symbols.EntSize));

		debug("Reserving kernel modules...");

		for (uint64_t i = 0; i < MAX_MODULES; i++)
		{
			if (bInfo.Modules[i].Address == 0x0)
				continue;

			debug("Reserving module %s (%#lx-%#lx)...", bInfo.Modules[i].CommandLine,
				  bInfo.Modules[i].Address, (void *)((uintptr_t)bInfo.Modules[i].Address + bInfo.Modules[i].Size));

			this->ReservePages((void *)bInfo.Modules[i].Address, TO_PAGES(bInfo.Modules[i].Size));
		}

#if defined(a86)
		debug("Reserving RSDT region %#lx-%#lx...", bInfo.RSDP, (void *)((uintptr_t)bInfo.RSDP + sizeof(BootInfo::RSDPInfo)));
		this->ReservePages(bInfo.RSDP, TO_PAGES(sizeof(BootInfo::RSDPInfo)));

		ACPI::ACPI::ACPIHeader *ACPIPtr = nullptr;
		bool XSDT = false;

		if (bInfo.RSDP->Revision >= 2 && bInfo.RSDP->XSDTAddress)
		{
			ACPIPtr = (ACPI::ACPI::ACPIHeader *)(bInfo.RSDP->XSDTAddress);
			XSDT = true;
		}
		else
			ACPIPtr = (ACPI::ACPI::ACPIHeader *)(uintptr_t)bInfo.RSDP->RSDTAddress;

		debug("Reserving RSDT...");
		this->ReservePages((void *)bInfo.RSDP, TO_PAGES(sizeof(BootInfo::RSDPInfo)));

#if defined(a64)
		if ((uintptr_t)ACPIPtr > 0x7FE00000) /* FIXME */
		{
			error("ACPI table is located above 0x7FE00000, which is not mapped.");
			return;
		}
#elif defined(a32)
		if ((uintptr_t)ACPIPtr > 0x2800000) /* FIXME */
		{
			error("ACPI table is located above 0x2800000, which is not mapped.");
			return;
		}
#endif

		size_t TableSize = ((ACPIPtr->Length - sizeof(ACPI::ACPI::ACPIHeader)) / (XSDT ? 8 : 4));
		debug("Reserving %d ACPI tables...", TableSize);

		for (size_t t = 0; t < TableSize; t++)
		{
			// TODO: Should I be concerned about unaligned memory access?
			ACPI::ACPI::ACPIHeader *SDTHdr = nullptr;
			if (XSDT)
				SDTHdr = (ACPI::ACPI::ACPIHeader *)(*(uint64_t *)((uint64_t)ACPIPtr + sizeof(ACPI::ACPI::ACPIHeader) + (t * 8)));
			else
				SDTHdr = (ACPI::ACPI::ACPIHeader *)(*(uint32_t *)((uint64_t)ACPIPtr + sizeof(ACPI::ACPI::ACPIHeader) + (t * 4)));

			this->ReservePages(SDTHdr, TO_PAGES(SDTHdr->Length));
		}

#elif defined(aa64)
#endif
	}
}
