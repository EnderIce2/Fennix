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

#include <memory/table.hpp>
#include <memory/va.hpp>
#include <cpu.hpp>
#include <debug.h>
#include <bitset>

#include "../../kernel.h"

namespace Memory
{
	VirtualAllocation::AllocatedPages VirtualAllocation::RequestPages(size_t Count)
	{
		func("%lld", Count);

		void *pAddress = KernelAllocator.RequestPages(Count);
		memset(pAddress, 0, FROM_PAGES(Count));

		Virtual vmm(this->Table);
		SmartLock(MgrLock);
		forItr(itr, AllocatedPagesList)
		{
			if (likely(itr->Free == false))
				continue;

			if (itr->PageCount == Count)
			{
				itr->Free = false;
				vmm.Map(itr->VirtualAddress, pAddress, FROM_PAGES(Count), RW | KRsv | G);
				return *itr;
			}

			if (itr->PageCount > Count)
			{
				/* Split the block */
				void *vAddress = itr->VirtualAddress;
				void *pAddress = itr->PhysicalAddress;
				size_t PageCount = itr->PageCount;

				AllocatedPagesList.erase(itr);

				AllocatedPagesList.push_back({(void *)((uintptr_t)pAddress + FROM_PAGES(Count)),
											  (void *)((uintptr_t)vAddress + FROM_PAGES(Count)),
											  PageCount - Count, true});
				AllocatedPagesList.push_back({pAddress, vAddress, Count, false});

				vmm.Map(vAddress, pAddress, FROM_PAGES(Count), RW | KRsv | G);
				debug("Split region %#lx-%#lx", vAddress, (uintptr_t)vAddress + FROM_PAGES(Count));
				debug("Free region %#lx-%#lx", (uintptr_t)vAddress + FROM_PAGES(Count), (uintptr_t)vAddress + FROM_PAGES(PageCount - Count));
				return AllocatedPagesList.back();
			}
		}

		/* Allocate new region */
		void *vAddress = CurrentBase;
		vmm.Map(vAddress, pAddress, FROM_PAGES(Count), RW | KRsv | G);
		AllocatedPagesList.push_back({pAddress, vAddress, Count, false});
		debug("New region %#lx-%#lx", vAddress, (uintptr_t)vAddress + FROM_PAGES(Count));
		CurrentBase = (void *)((uintptr_t)CurrentBase + FROM_PAGES(Count));
		assert(USER_ALLOC_END > (uintptr_t)CurrentBase);
		return AllocatedPagesList.back();
	}

	void VirtualAllocation::FreePages(void *Address, size_t Count)
	{
		func("%#lx, %lld", Address, Count);

		SmartLock(MgrLock);
		foreach (auto &apl in AllocatedPagesList)
		{
			if (apl.VirtualAddress != Address)
				continue;

			if (apl.PageCount != Count)
			{
				error("Page count mismatch! (Allocated: %lld, Requested: %lld)",
					  apl.PageCount, Count);
				return;
			}

			Virtual vmm(this->Table);
			for (size_t i = 0; i < Count; i++)
			{
				void *AddressToUnmap = (void *)((uintptr_t)Address + FROM_PAGES(i));
				vmm.Unmap(AddressToUnmap);
			}

			KernelAllocator.FreePages(Address, Count);
			apl.Free = true;
			debug("Freed region %#lx-%#lx", Address, (uintptr_t)Address + FROM_PAGES(Count));
			return;
		}
	}

	void VirtualAllocation::MapTo(AllocatedPages ap, PageTable *TargetTable)
	{
		func("%#lx, %#lx", ap.VirtualAddress, TargetTable);

		Virtual vmm(TargetTable);
		vmm.Map(ap.VirtualAddress, ap.PhysicalAddress, FROM_PAGES(ap.PageCount), RW | KRsv | G);
	}

	VirtualAllocation::VirtualAllocation(void *Base)
		: BaseAddress(Base), CurrentBase(Base),
		  Table((PageTable *)CPU::PageTable())
	{
		func("%#lx", Base);
	}

	VirtualAllocation::~VirtualAllocation()
	{
		/* No need to remap pages, the page table will be destroyed */

		Virtual vmm(this->Table);
		foreach (auto ap in AllocatedPagesList)
		{
			KernelAllocator.FreePages(ap.PhysicalAddress, ap.PageCount);

			for (size_t i = 0; i < ap.PageCount; i++)
			{
				void *AddressToUnmap = (void *)((uintptr_t)ap.VirtualAddress + FROM_PAGES(i));
				vmm.Unmap(AddressToUnmap);
			}
		}
	}
}
