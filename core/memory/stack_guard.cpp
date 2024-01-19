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
		if (this->UserMode)
		{
			if (FaultAddress < (uintptr_t)this->StackBottom - 0x100 ||
				FaultAddress > (uintptr_t)this->StackTop)
			{
				info("Fault address %#lx is not in range of stack %#lx - %#lx", FaultAddress,
					 (uintptr_t)this->StackBottom - 0x100, (uintptr_t)this->StackTop);
				return false; /* It's not about the stack. */
			}
			else
			{
				void *AllocatedStack = this->vma->RequestPages(TO_PAGES(USER_STACK_SIZE) + 1);
				debug("AllocatedStack: %#lx", AllocatedStack);
				memset(AllocatedStack, 0, USER_STACK_SIZE);

				Virtual vmm = Virtual(this->vma->GetTable());
				for (size_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
				{
					void *VirtualPage = (void *)((uintptr_t)this->StackBottom - (i * PAGE_SIZE));
					void *PhysicalPage = (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE));

					vmm.Map(VirtualPage, PhysicalPage, PTFlag::RW | PTFlag::US);
					AllocatedPages ap = {
						.PhysicalAddress = PhysicalPage,
						.VirtualAddress = VirtualPage,
					};
					AllocatedPagesList.push_back(ap);
					debug("Mapped %#lx to %#lx", PhysicalPage, VirtualPage);
				}

				this->StackBottom = (void *)((uintptr_t)this->StackBottom - USER_STACK_SIZE);
				this->Size += USER_STACK_SIZE;
				debug("Stack expanded to %#lx", this->StackBottom);
				this->Expanded = true;
				return true;
			}
		}
		else
		{
			fixme("Not implemented and probably not needed");
			return false;
		}
	}

	void StackGuard::Fork(StackGuard *Parent)
	{
		this->UserMode = Parent->GetUserMode();
		this->StackBottom = Parent->GetStackBottom();
		this->StackTop = Parent->GetStackTop();
		this->StackPhysicalBottom = Parent->GetStackPhysicalBottom();
		this->StackPhysicalTop = Parent->GetStackPhysicalTop();
		this->Size = Parent->GetSize();
		this->Expanded = Parent->IsExpanded();

		if (this->UserMode)
		{
			std::vector<AllocatedPages> ParentAllocatedPages = Parent->GetAllocatedPages();
			Virtual vma(this->vma->GetTable());
			foreach (auto Page in ParentAllocatedPages)
			{
				void *NewPhysical = this->vma->RequestPages(1);
				debug("Forking address %#lx to %#lx", Page.PhysicalAddress, NewPhysical);
				memcpy(NewPhysical, Page.PhysicalAddress, PAGE_SIZE);
				vma.Map(Page.VirtualAddress, NewPhysical, PTFlag::RW | PTFlag::US);

				AllocatedPages ap = {
					.PhysicalAddress = NewPhysical,
					.VirtualAddress = Page.VirtualAddress,
				};
				AllocatedPagesList.push_back(ap);
				debug("Mapped %#lx to %#lx", NewPhysical, Page.VirtualAddress);
			}
		}
		else
		{
			fixme("Kernel mode stack fork not implemented");
		}
	}

	StackGuard::StackGuard(bool User, VirtualMemoryArea *vma)
	{
		this->UserMode = User;
		this->vma = vma;

		if (this->UserMode)
		{
			void *AllocatedStack = vma->RequestPages(TO_PAGES(USER_STACK_SIZE) + 1);
			memset(AllocatedStack, 0, USER_STACK_SIZE);
			debug("AllocatedStack: %#lx", AllocatedStack);

			{
				Virtual vmm = Virtual(vma->GetTable());
				for (size_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
				{
					void *VirtualPage = (void *)(USER_STACK_BASE + (i * PAGE_SIZE));
					void *PhysicalPage = (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE));
					vmm.Map(VirtualPage, PhysicalPage, PTFlag::RW | PTFlag::US);

					AllocatedPages ap = {
						.PhysicalAddress = PhysicalPage,
						.VirtualAddress = VirtualPage,
					};
					AllocatedPagesList.push_back(ap);
					debug("Mapped %#lx to %#lx", PhysicalPage, VirtualPage);
				}
			}

			this->StackBottom = (void *)USER_STACK_BASE;
			this->StackTop = (void *)(USER_STACK_BASE + USER_STACK_SIZE);

			this->StackPhysicalBottom = AllocatedStack;
			this->StackPhysicalTop = (void *)((uintptr_t)AllocatedStack + USER_STACK_SIZE);

			this->Size = USER_STACK_SIZE;
		}
		else
		{
			this->StackBottom = vma->RequestPages(TO_PAGES(STACK_SIZE) + 1);
			memset(this->StackBottom, 0, STACK_SIZE);
			debug("StackBottom: %#lx", this->StackBottom);

			this->StackTop = (void *)((uintptr_t)this->StackBottom + STACK_SIZE);

			this->StackPhysicalBottom = this->StackBottom;
			this->StackPhysicalTop = this->StackTop;

			this->Size = STACK_SIZE;

			for (size_t i = 0; i < TO_PAGES(STACK_SIZE); i++)
			{
				AllocatedPages pa = {
					.PhysicalAddress = (void *)((uintptr_t)this->StackBottom + (i * PAGE_SIZE)),
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
		/* VMA will free the stack */
	}
}
