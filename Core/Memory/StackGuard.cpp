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
            void *AllocatedStack = KernelAllocator.RequestPages(TO_PAGES(USER_STACK_SIZE));
            debug("AllocatedStack: %p", AllocatedStack);
            memset(AllocatedStack, 0, USER_STACK_SIZE);
            for (uint64_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
            {
                Virtual(Table).Map((void *)(USER_STACK_BASE + (i * PAGE_SIZE)),
                                   (void *)((uint64_t)AllocatedStack + (i * PAGE_SIZE)),
                                   PTFlag::RW | PTFlag::US);
                debug("Mapped %p to %p", (void *)(USER_STACK_BASE + (i * PAGE_SIZE)),
                      (void *)((uint64_t)AllocatedStack + (i * PAGE_SIZE)));
            }
            this->StackBottom = (void *)USER_STACK_BASE;
            this->StackTop = (void *)(USER_STACK_BASE + USER_STACK_SIZE);
            this->Size = USER_STACK_SIZE;
        }
        else
        {
            this->StackBottom = KernelAllocator.RequestPages(TO_PAGES(STACK_SIZE));
            debug("StackBottom: %p", this->StackBottom);
            memset(this->StackBottom, 0, STACK_SIZE);
            this->StackTop = (void *)((uint64_t)this->StackBottom + STACK_SIZE);
            this->Size = STACK_SIZE;
        }
        trace("Allocated stack at %p", this->StackBottom);
    }

    StackGuard::~StackGuard()
    {
        fixme("Temporarily disabled stack guard deallocation");
        // KernelAllocator.FreePages(this->StackBottom, TO_PAGES(this->Size));
        // debug("Freed stack at %p", this->StackBottom);
    }

    bool StackGuard::Expand(uint64_t FaultAddress)
    {
        if (this->UserMode)
        {
            if (FaultAddress < (uint64_t)this->StackBottom - USER_STACK_SIZE ||
                FaultAddress > (uint64_t)this->StackTop)
            {
                return false; // It's not about the stack.
            }
            else
            {
                void *AllocatedStack = KernelAllocator.RequestPages(TO_PAGES(USER_STACK_SIZE));
                debug("AllocatedStack: %p", AllocatedStack);
                memset(AllocatedStack, 0, USER_STACK_SIZE);
                for (uint64_t i = 0; i < TO_PAGES(USER_STACK_SIZE); i++)
                {
                    Virtual(this->Table).Map((void *)((uint64_t)this->StackBottom - (i * PAGE_SIZE)), (void *)((uint64_t)AllocatedStack + (i * PAGE_SIZE)), PTFlag::RW | PTFlag::US);
                    debug("Mapped %p to %p", (void *)((uint64_t)this->StackBottom - (i * PAGE_SIZE)), (void *)((uint64_t)AllocatedStack + (i * PAGE_SIZE)));
                }
                this->StackBottom = (void *)((uint64_t)this->StackBottom - USER_STACK_SIZE);
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
        error("Reached end of function! How?");
        return false;
    }
}
