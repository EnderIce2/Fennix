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

namespace Memory
{
	bool StackGuard::Expand(uintptr_t FaultAddress)
	{
		if (!this->UserMode)
			assert(!"Kernel mode stack expansion not implemented");

		if (FaultAddress < USER_STACK_END ||
			FaultAddress > USER_STACK_BASE)
		{
			info("Fault address %#lx is not in range of stack %#lx - %#lx",
				 FaultAddress, USER_STACK_END, USER_STACK_BASE);
			return false; /* It's not about the stack. */
		}

		uintptr_t roundFA = ROUND_DOWN(FaultAddress, PAGE_SIZE);
		uintptr_t diff = (uintptr_t)this->StackBottom - roundFA;
		size_t stackPages = TO_PAGES(diff);
		stackPages = stackPages < 1 ? 1 : stackPages;

		debug("roundFA: %#lx, sb: %#lx, diff: %#lx, stackPages: %d",
			  roundFA, this->StackBottom, diff, stackPages);

		void *AllocatedStack = vma->RequestPages(stackPages);
		debug("AllocatedStack: %#lx", AllocatedStack);

		for (size_t i = 0; i < stackPages; i++)
		{
			void *vAddress = (void *)((uintptr_t)this->StackBottom - (i * PAGE_SIZE));
			void *pAddress = (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE));

			vma->Map(vAddress, pAddress, PAGE_SIZE, PTFlag::RW | PTFlag::US);
			AllocatedPages ap = {
				.PhysicalAddress = pAddress,
				.VirtualAddress = vAddress,
			};
			AllocatedPagesList.push_back(ap);
			debug("Mapped %#lx to %#lx", pAddress, vAddress);
		}

		this->StackBottom = (void *)((uintptr_t)this->StackBottom - (stackPages * PAGE_SIZE));
		this->Size += stackPages * PAGE_SIZE;
		debug("Stack expanded to %#lx", this->StackBottom);
		this->Expanded = true;
		return true;
	}

	void StackGuard::Fork(StackGuard *Parent)
	{
		if (!this->UserMode)
			assert(!"Kernel mode stack fork not implemented");

		this->UserMode = Parent->UserMode;
		this->StackBottom = Parent->StackBottom;
		this->StackTop = Parent->StackTop;
		this->StackPhysicalBottom = Parent->StackPhysicalBottom;
		this->StackPhysicalTop = Parent->StackPhysicalTop;
		this->Size = Parent->Size;
		this->Expanded = Parent->Expanded;

		std::list<AllocatedPages> ParentAllocatedPages = Parent->GetAllocatedPages();
		foreach (auto Page in ParentAllocatedPages)
		{
			void *NewPhysical = vma->RequestPages(1);
			debug("Forking address %#lx to %#lx", Page.PhysicalAddress, NewPhysical);
			memcpy(NewPhysical, Page.PhysicalAddress, PAGE_SIZE);
			vma->Remap(Page.VirtualAddress, NewPhysical, PTFlag::RW | PTFlag::US);

			AllocatedPages ap = {
				.PhysicalAddress = NewPhysical,
				.VirtualAddress = Page.VirtualAddress,
			};
			AllocatedPagesList.push_back(ap);
			debug("Mapped %#lx to %#lx", NewPhysical, Page.VirtualAddress);
		}
	}

	StackGuard::StackGuard(bool User, VirtualMemoryArea *_vma)
	{
		this->UserMode = User;
		this->vma = _vma;

		if (this->UserMode)
		{
			void *AllocatedStack = vma->RequestPages(TO_PAGES(USER_STACK_SIZE));
			this->StackBottom = (void *)USER_STACK_BASE;
			this->StackTop = (void *)(USER_STACK_BASE + USER_STACK_SIZE);
			this->StackPhysicalBottom = AllocatedStack;
			this->StackPhysicalTop = (void *)((uintptr_t)AllocatedStack + USER_STACK_SIZE);
			this->Size = USER_STACK_SIZE;

			debug("AllocatedStack: %#lx", AllocatedStack);

			for (size_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
			{
				void *vAddress = (void *)(USER_STACK_BASE + (i * PAGE_SIZE));
				void *pAddress = (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE));
				vma->Map(vAddress, pAddress, PAGE_SIZE, PTFlag::RW | PTFlag::US);

				AllocatedPages ap = {
					.PhysicalAddress = pAddress,
					.VirtualAddress = vAddress,
				};
				AllocatedPagesList.push_back(ap);
				debug("Mapped %#lx to %#lx", pAddress, vAddress);
			}
		}
		else
		{
			Memory::KernelStackManager::StackAllocation sa = StackManager.DetailedAllocate(LARGE_STACK_SIZE);
			this->StackBottom = sa.VirtualAddress;
			this->StackTop = (void *)((uintptr_t)this->StackBottom + LARGE_STACK_SIZE);
			this->StackPhysicalBottom = sa.PhysicalAddress;
			this->StackPhysicalTop = (void *)((uintptr_t)this->StackPhysicalBottom + LARGE_STACK_SIZE);
			this->Size = LARGE_STACK_SIZE;

			debug("StackBottom: %#lx", this->StackBottom);

			for (size_t i = 0; i < TO_PAGES(LARGE_STACK_SIZE); i++)
			{
				AllocatedPages pa = {
					.PhysicalAddress = (void *)((uintptr_t)this->StackPhysicalBottom + (i * PAGE_SIZE)),
					.VirtualAddress = (void *)((uintptr_t)this->StackBottom + (i * PAGE_SIZE)),
				};
				AllocatedPagesList.push_back(pa);
			}
		}

		debug("Allocated stack at %#lx", this->StackBottom);
		debug("Stack Range: %#lx - %#lx", this->StackBottom, this->StackTop);
	}

	StackGuard::~StackGuard()
	{
		if (!this->UserMode)
		{
			for (auto Page : this->AllocatedPagesList)
				StackManager.Free(Page.VirtualAddress);
		}

		/* VMA will free the stack */
	}
}
