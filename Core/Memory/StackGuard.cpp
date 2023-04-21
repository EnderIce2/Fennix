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
    StackGuard::StackGuard(bool User, PageTable4 *Table)
    {
        this->UserMode = User;
        this->Table = Table;

        if (this->UserMode)
        {
            void *AllocatedStack = KernelAllocator.RequestPages(TO_PAGES(USER_STACK_SIZE + 1));
            memset(AllocatedStack, 0, USER_STACK_SIZE);
            debug("AllocatedStack: %p", AllocatedStack);

            Virtual va = Virtual(Table);
            for (size_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
            {
                va.Map((void *)(USER_STACK_BASE + (i * PAGE_SIZE)),
                                   (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE)),
                                   PTFlag::RW | PTFlag::US);

                debug("Mapped %p to %p", (void *)(USER_STACK_BASE + (i * PAGE_SIZE)),
                      (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE)));
            }

            this->StackBottom = (void *)USER_STACK_BASE;
            this->StackTop = (void *)(USER_STACK_BASE + USER_STACK_SIZE);

            this->StackPhyiscalBottom = AllocatedStack;
            this->StackPhyiscalTop = (void *)((uintptr_t)AllocatedStack + USER_STACK_SIZE);

            this->Size = USER_STACK_SIZE;
        }
        else
        {
            this->StackBottom = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE + 1));
            memset(this->StackBottom, 0, STACK_SIZE);
            debug("StackBottom: %p", this->StackBottom);

            this->StackTop = (void *)((uintptr_t)this->StackBottom + STACK_SIZE);

            this->StackPhyiscalBottom = this->StackBottom;
            this->StackPhyiscalTop = this->StackTop;

            this->Size = STACK_SIZE;
        }

        trace("Allocated stack at %p", this->StackBottom);
    }

    StackGuard::~StackGuard()
    {
        fixme("Temporarily disabled stack guard deallocation");
        // KernelAllocator.FreePages(this->StackBottom, TO_PAGES(this->Size + 1));
        // debug("Freed stack at %p", this->StackBottom);
    }

    bool StackGuard::Expand(uintptr_t FaultAddress)
    {
        if (this->UserMode)
        {
            if (FaultAddress < (uintptr_t)this->StackBottom - USER_STACK_SIZE ||
                FaultAddress > (uintptr_t)this->StackTop)
            {
                return false; /* It's not about the stack. */
            }
            else
            {
                void *AllocatedStack = KernelAllocator.RequestPages(TO_PAGES(USER_STACK_SIZE + 1));
                debug("AllocatedStack: %p", AllocatedStack);
                memset(AllocatedStack, 0, USER_STACK_SIZE);
                Virtual va = Virtual(this->Table);
                for (uintptr_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
                {
                    va.Map((void *)((uintptr_t)this->StackBottom - (i * PAGE_SIZE)), (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE)), PTFlag::RW | PTFlag::US);
                    debug("Mapped %p to %p", (void *)((uintptr_t)this->StackBottom - (i * PAGE_SIZE)), (void *)((uintptr_t)AllocatedStack + (i * PAGE_SIZE)));
                }
                this->StackBottom = (void *)((uintptr_t)this->StackBottom - USER_STACK_SIZE);
                this->Size += USER_STACK_SIZE;
                info("Stack expanded to %p", this->StackBottom);
                return true;
            }
        }
        else
        {
            fixme("Not implemented and probably not needed");
            return false;
        }
    }
}
