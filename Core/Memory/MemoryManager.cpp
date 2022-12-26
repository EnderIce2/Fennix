#include <memory.hpp>
#include <debug.h>

namespace Memory
{
    uint64_t MemMgr::GetAllocatedMemorySize()
    {
        uint64_t Size = 0;
        foreach (auto var in AllocatedPagesList)
            Size += var.PageCount;
        return FROM_PAGES(Size);
    }

    bool MemMgr::Add(void *Address, size_t Count)
    {
        for (size_t i = 0; i < AllocatedPagesList.size(); i++)
        {
            if (AllocatedPagesList[i].Address == Address)
            {
                error("Address already exists!");
                return false;
            }

            if ((uintptr_t)Address < (uintptr_t)AllocatedPagesList[i].Address)
            {
                if ((uintptr_t)Address + (Count * PAGE_SIZE) > (uintptr_t)AllocatedPagesList[i].Address)
                {
                    error("Address intersects with an allocated page!");
                    return false;
                }
            }
            else
            {
                if ((uintptr_t)AllocatedPagesList[i].Address + (AllocatedPagesList[i].PageCount * PAGE_SIZE) > (uintptr_t)Address)
                {
                    error("Address intersects with an allocated page!");
                    return false;
                }
            }
        }

        AllocatedPagesList.push_back({Address, Count});
        return true;
    }

    void *MemMgr::RequestPages(size_t Count)
    {
        void *Address = KernelAllocator.RequestPages(Count);
        for (size_t i = 0; i < Count; i++)
            Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
        AllocatedPagesList.push_back({Address, Count});
        return Address;
    }

    void MemMgr::FreePages(void *Address, size_t Count)
    {
        for (size_t i = 0; i < AllocatedPagesList.size(); i++)
            if (AllocatedPagesList[i].Address == Address)
            {
                // TODO: Advanced checks. Allow if the page count is less than the requested one.
                if (AllocatedPagesList[i].PageCount != Count)
                {
                    error("FreePages: Page count mismatch! (Allocated: %lld, Requested: %lld)", AllocatedPagesList[i].PageCount, Count);
                    return;
                }

                KernelAllocator.FreePages(Address, Count);
                for (size_t i = 0; i < Count; i++)
                    Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);

                AllocatedPagesList.remove(i);
                return;
            }
    }

    MemMgr::MemMgr(PageTable4 *PageTable)
    {
        if (PageTable)
            this->PageTable = PageTable;
        else
            this->PageTable = (PageTable4 *)CPU::x64::readcr3().raw;
        debug("MemMgr initialized.");
    }

    MemMgr::~MemMgr()
    {
        foreach (auto var in AllocatedPagesList)
        {
            KernelAllocator.FreePages(var.Address, var.PageCount);
            for (size_t i = 0; i < var.PageCount; i++)
                Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)var.Address + (i * PAGE_SIZE)), (void *)((uintptr_t)var.Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);
        }
        debug("MemMgr destroyed.");
    }
}
