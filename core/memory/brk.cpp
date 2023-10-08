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
			return (void *)-ENOMEM;

		Virtual vmm = Virtual(this->Table);

		if ((uintptr_t)Address > Break)
		{
			/* Allocate more memory. */
			size_t Pages = TO_PAGES(uintptr_t(Address) - Break);
			void *Allocated = vma->RequestPages(Pages);
			if (Allocated == nullptr)
				return (void *)-ENOMEM;

			/* Map the allocated pages. */
			for (size_t i = 0; i < Pages; i++)
			{
				void *VirtAddr = (void *)(Break + (i * PAGE_SIZE));
				void *PhysAddr = (void *)(uintptr_t(Allocated) + (i * PAGE_SIZE));
				vmm.Map(VirtAddr, PhysAddr, RW | US);
			}

			Break = (uint64_t)Address;
			return (void *)Break;
		}

		/* Free memory. */
		size_t Pages = TO_PAGES(uintptr_t(Address) - Break);
		vma->FreePages((void *)Break, Pages);

		/* Unmap the freed pages. */
		for (size_t i = 0; i < Pages; i++)
		{
			uint64_t Page = Break - (i * 0x1000);
			vmm.Remap((void *)Page, (void *)Page, PTFlag::P | PTFlag::RW);
		}

		Break = (uint64_t)Address;
		return (void *)Break;
	}

	ProgramBreak::ProgramBreak(PageTable *Table, VirtualMemoryArea *vma)
	{
		assert(Table != nullptr);
		assert(vma != nullptr);

		this->Table = Table;
		this->vma = vma;
	}

	ProgramBreak::~ProgramBreak()
	{
		/* Do nothing because VirtualMemoryArea
			will be destroyed later. */
	}
}
