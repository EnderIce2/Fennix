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

#include <memory/brk.hpp>
#include <memory/virtual.hpp>
#include <memory/vma.hpp>
#include <assert.h>
#include <errno.h>
#include <debug.h>

namespace Memory
{
	void *ProgramBreak::brk(void *Address)
	{
		if (HeapStart == 0x0 || Break == 0x0)
		{
			error("HeapStart or Break is 0x0");
			return (void *)-EAGAIN;
		}

		/* Get the current program break. */
		if (Address == nullptr)
			return (void *)Break;

		/* Check if the address is valid. */
		if ((uintptr_t)Address < HeapStart)
		{
			debug("Address %#lx is less than HeapStart %#lx", Address, HeapStart);
			return (void *)-ENOMEM;
		}

		Virtual vmm(this->Table);

		if ((uintptr_t)Address > Break)
		{
			/* Allocate more memory. */
			ssize_t Pages = TO_PAGES(uintptr_t(Address) - Break);
			void *Allocated = vma->RequestPages(Pages);
			if (Allocated == nullptr)
				return (void *)-ENOMEM;

			/* Map the allocated pages. */
			for (ssize_t i = 0; i < Pages; i++)
			{
				void *VirtAddr = (void *)(Break + (i * PAGE_SIZE));
				void *PhysAddr = (void *)(uintptr_t(Allocated) + (i * PAGE_SIZE));
				debug("Mapping %#lx to %#lx", VirtAddr, PhysAddr);
				vmm.Map(VirtAddr, PhysAddr, RW | US);
			}

			Break = ROUND_UP(uintptr_t(Address), PAGE_SIZE);
			debug("Round up %#lx to %#lx", Address, Break);
			return Address;
		}

		/* Free memory. */
		ssize_t Pages = TO_PAGES(Break - uintptr_t(Address));
		vma->FreePages((void *)Break, Pages);

		/* Unmap the freed pages. */
		for (ssize_t i = 0; i < Pages; i++)
		{
			uint64_t Page = Break - (i * 0x1000);
			vmm.Remap((void *)Page, (void *)Page, RW);
			debug("Unmapping %#lx", Page);
		}

		Break = (uint64_t)Address;
		return (void *)Break;
	}

	ProgramBreak::ProgramBreak(PageTable *Table, VirtualMemoryArea *vma)
	{
		assert(Table != nullptr);
		assert(vma != nullptr);

		debug("+ %#lx", this);

		this->Table = Table;
		this->vma = vma;
	}

	ProgramBreak::~ProgramBreak()
	{
		debug("- %#lx", this);

		/* Do nothing because VirtualMemoryArea
			will be destroyed later. */
	}
}
