#include <memory.hpp>
#include <debug.h>

namespace Memory
{
    uint64_t Tracker::GetAllocatedMemorySize()
    {
        uint64_t Size = 0;
        foreach (auto var in AllocatedPagesList)
            Size += var.PageCount;
        return FROM_PAGES(Size);
    }

    void *Tracker::RequestPages(uint64_t Count)
    {
        void *Address = KernelAllocator.RequestPages(Count);
        for (uint64_t i = 0; i < Count; i++)
            Memory::Virtual(this->PageTable).Remap((void *)((uint64_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
        AllocatedPagesList.push_back({Address, Count});
        return Address;
    }

    void Tracker::FreePages(void *Address, uint64_t Count)
    {
        KernelAllocator.FreePages(Address, Count);
        for (uint64_t i = 0; i < Count; i++)
            Memory::Virtual(this->PageTable).Remap((void *)((uint64_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);

        for (uint64_t i = 0; i < AllocatedPagesList.size(); i++)
            if (AllocatedPagesList[i].Address == Address)
            {
                AllocatedPagesList.remove(i);
                break;
            }
    }

    Tracker::Tracker(PageTable4 *PageTable)
    {
        this->PageTable = PageTable;
        debug("Tracker initialized.");
    }

    Tracker::~Tracker()
    {
        foreach (auto var in AllocatedPagesList)
        {
            KernelAllocator.FreePages(var.Address, var.PageCount);
            for (uint64_t i = 0; i < var.PageCount; i++)
                Memory::Virtual(this->PageTable).Remap((void *)((uint64_t)var.Address + (i * PAGE_SIZE)), (void *)((uint64_t)var.Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);
        }
        debug("Tracker destroyed.");
    }
}
