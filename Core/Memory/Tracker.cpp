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

    void *Tracker::RequestPages(size_t Count)
    {
        void *Address = KernelAllocator.RequestPages(Count);
        for (size_t i = 0; i < Count; i++)
            Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW | Memory::PTFlag::US);
        AllocatedPagesList.push_back({Address, Count});
        return Address;
    }

    void Tracker::FreePages(void *Address, size_t Count)
    {
        KernelAllocator.FreePages(Address, Count);
        for (size_t i = 0; i < Count; i++)
            Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)Address + (i * PAGE_SIZE)), (void *)((uint64_t)Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);

        for (uintptr_t i = 0; i < AllocatedPagesList.size(); i++)
            if (AllocatedPagesList[i].Address == Address)
            {
                AllocatedPagesList.remove(i);
                break;
            }
    }

    Tracker::Tracker(PageTable4 *PageTable)
    {
        if (PageTable)
            this->PageTable = PageTable;
        else
            this->PageTable = (PageTable4 *)CPU::x64::readcr3().raw;
        debug("Tracker initialized.");
    }

    Tracker::~Tracker()
    {
        foreach (auto var in AllocatedPagesList)
        {
            KernelAllocator.FreePages(var.Address, var.PageCount);
            for (size_t i = 0; i < var.PageCount; i++)
                Memory::Virtual(this->PageTable).Remap((void *)((uintptr_t)var.Address + (i * PAGE_SIZE)), (void *)((uintptr_t)var.Address + (i * PAGE_SIZE)), Memory::PTFlag::RW);
        }
        debug("Tracker destroyed.");
    }
}
