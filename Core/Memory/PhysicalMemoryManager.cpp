#include <memory.hpp>

#include <debug.h>

namespace Memory
{
    uint64_t Physical::GetTotalMemory()
    {
        SmartLock(this->MemoryLock);
        return this->TotalMemory;
    }

    uint64_t Physical::GetFreeMemory()
    {
        SmartLock(this->MemoryLock);
        return this->FreeMemory;
    }

    uint64_t Physical::GetReservedMemory()
    {
        SmartLock(this->MemoryLock);
        return this->ReservedMemory;
    }

    uint64_t Physical::GetUsedMemory()
    {
        SmartLock(this->MemoryLock);
        return this->UsedMemory;
    }

    bool Physical::SwapPage(void *Address)
    {
        fixme("%p", Address);
        return false;
    }

    bool Physical::SwapPages(void *Address, uint64_t PageCount)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            if (!this->SwapPage((void *)((uint64_t)Address + (i * PAGE_SIZE))))
                return false;
        return false;
    }

    bool Physical::UnswapPage(void *Address)
    {
        fixme("%p", Address);
        return false;
    }

    bool Physical::UnswapPages(void *Address, uint64_t PageCount)
    {
        for (uint64_t i = 0; i < PageCount; i++)
            if (!this->UnswapPage((void *)((uint64_t)Address + (i * PAGE_SIZE))))
                return false;
        return false;
    }

    void *Physical::RequestPage()
    {
        SmartLock(this->MemoryLock);
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;
            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        if (this->SwapPage((void *)(PageBitmapIndex * PAGE_SIZE)))
        {
            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        CPU::Halt(true);
        return nullptr;
    }

    void *Physical::RequestPages(uint64_t Count)
    {
        SmartLock(this->MemoryLock);
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            for (uint64_t Index = PageBitmapIndex; Index < PageBitmap.Size * 8; Index++)
            {
                if (PageBitmap[Index] == true)
                    continue;

                for (uint64_t i = 0; i < Count; i++)
                    if (PageBitmap[Index + i] == true)
                        goto NextPage;

                this->LockPages((void *)(Index * PAGE_SIZE), Count);
                return (void *)(Index * PAGE_SIZE);

            NextPage:
                Index += Count;
                continue;
            }
        }

        if (this->SwapPages((void *)(PageBitmapIndex * PAGE_SIZE), Count))
        {
            this->LockPages((void *)(PageBitmapIndex * PAGE_SIZE), Count);
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        CPU::Halt(true);
        return nullptr;
    }

    void Physical::FreePage(void *Address)
    {
        SmartLock(this->MemoryLock);
        if (unlikely(Address == nullptr))
        {
            warn("Null pointer passed to FreePage.");
            return;
        }
        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (unlikely(PageBitmap[Index] == false))
            return;

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            UsedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }
    }

    void Physical::FreePages(void *Address, uint64_t Count)
    {
        if (unlikely(Address == nullptr || Count == 0))
        {
            warn("%s%s passed to FreePages.", Address == nullptr ? "Null pointer" : "", Count == 0 ? "Zero count" : "");
            return;
        }

        for (uint64_t t = 0; t < Count; t++)
            this->FreePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::LockPage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to lock null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (unlikely(PageBitmap[Index] == true))
            return;
        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            UsedMemory += PAGE_SIZE;
        }
    }

    void Physical::LockPages(void *Address, uint64_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to lock %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t i = 0; i < PageCount; i++)
            this->LockPage((void *)((uint64_t)Address + (i * PAGE_SIZE)));
    }

    void Physical::ReservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to reserve null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            ReservedMemory += PAGE_SIZE;
        }
    }

    void Physical::ReservePages(void *Address, uint64_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to reserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t t = 0; t < PageCount; t++)
            this->ReservePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::UnreservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to unreserve null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (unlikely(PageBitmap[Index] == false))
            return;

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            ReservedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }
    }

    void Physical::UnreservePages(void *Address, uint64_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to unreserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t t = 0; t < PageCount; t++)
            this->UnreservePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::Init(BootInfo *Info)
    {
        SmartLock(this->MemoryLock);
        void *LargestFreeMemorySegment = nullptr;
        uint64_t LargestFreeMemorySegmentSize = 0;
        uint64_t MemorySize = Info->Memory.Size;

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
            if (Info->Memory.Entry[i].Type == Usable)
                if (Info->Memory.Entry[i].Length > LargestFreeMemorySegmentSize)
                {
                    // We don't want to use 0 as a memory address.
                    if (Info->Memory.Entry[i].BaseAddress == nullptr)
                        continue;
                    LargestFreeMemorySegment = (void *)Info->Memory.Entry[i].BaseAddress;
                    LargestFreeMemorySegmentSize = Info->Memory.Entry[i].Length;
                    debug("Largest free memory segment: %llp (%lldMB)",
                          (void *)Info->Memory.Entry[i].BaseAddress,
                          TO_MB(Info->Memory.Entry[i].Length));
                }
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        if (LargestFreeMemorySegment == nullptr)
        {
            error("No free memory found!");
            CPU::Stop();
        }

        uint64_t BitmapSize = (MemorySize / PAGE_SIZE) / 8 + 1;
        trace("Initializing Bitmap at %llp-%llp (%lld Bytes)",
              LargestFreeMemorySegment,
              (void *)((uint64_t)LargestFreeMemorySegment + BitmapSize),
              BitmapSize);
        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)LargestFreeMemorySegment;
        for (uint64_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        trace("Reserving pages...");
        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
            if (Info->Memory.Entry[i].Type != Usable)
                this->ReservePages((void *)Info->Memory.Entry[i].BaseAddress, Info->Memory.Entry[i].Length / PAGE_SIZE + 1);
        trace("Locking bitmap pages...");
        this->ReservePages(0, 0x100);
        this->LockPages(PageBitmap.Buffer, PageBitmap.Size / PAGE_SIZE + 1);
    }

    Physical::Physical() {}
    Physical::~Physical() {}
}
