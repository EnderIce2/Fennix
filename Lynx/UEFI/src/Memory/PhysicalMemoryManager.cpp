#include "memory.hpp"

extern "C" void printf(const char *format, ...);

namespace Memory
{
    uint64_t Physical::GetTotalMemory()
    {
        return this->TotalMemory;
    }

    uint64_t Physical::GetFreeMemory()
    {
        return this->FreeMemory;
    }

    uint64_t Physical::GetReservedMemory()
    {
        return this->ReservedMemory;
    }

    uint64_t Physical::GetUsedMemory()
    {
        return this->UsedMemory;
    }

    bool Physical::SwapPage(void *Address)
    {
        printf("%p", Address);
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
        printf("%p", Address);
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

        printf("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", (FreeMemory / 1024 / 1024), (UsedMemory / 1024 / 1024), (ReservedMemory / 1024 / 1024));
        while (1)
            __asm__("hlt");
        return nullptr;
    }

    void *Physical::RequestPages(uint64_t Count)
    {
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

        printf("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", (FreeMemory / 1024 / 1024), (UsedMemory / 1024 / 1024), (ReservedMemory / 1024 / 1024));
        while (1)
            __asm__("hlt");
        return nullptr;
    }

    void Physical::FreePage(void *Address)
    {
        if (Address == nullptr)
        {
            printf("Null pointer passed to FreePage.");
            return;
        }
        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (PageBitmap[Index] == false)
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
        if (Address == nullptr || Count == 0)
        {
            printf("%s%s passed to FreePages.", Address == nullptr ? "Null pointer" : "", Count == 0 ? "Zero count" : "");
            return;
        }

        for (uint64_t t = 0; t < Count; t++)
            this->FreePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::LockPage(void *Address)
    {
        if (Address == nullptr)
            printf("Trying to lock null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (PageBitmap[Index] == true)
            return;
        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            UsedMemory += PAGE_SIZE;
        }
    }

    void Physical::LockPages(void *Address, uint64_t PageCount)
    {
        if (Address == nullptr || PageCount == 0)
            printf("Trying to lock %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t i = 0; i < PageCount; i++)
            this->LockPage((void *)((uint64_t)Address + (i * PAGE_SIZE)));
    }

    void Physical::ReservePage(void *Address)
    {
        if (Address == nullptr)
            printf("Trying to reserve null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (PageBitmap[Index] == true)
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            ReservedMemory += PAGE_SIZE;
        }
    }

    void Physical::ReservePages(void *Address, uint64_t PageCount)
    {
        if (Address == nullptr || PageCount == 0)
            printf("Trying to reserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t t = 0; t < PageCount; t++)
            this->ReservePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::UnreservePage(void *Address)
    {
        if (Address == nullptr)
            printf("Trying to unreserve null address.");

        uint64_t Index = (uint64_t)Address / PAGE_SIZE;
        if (PageBitmap[Index] == false)
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
        if (Address == nullptr || PageCount == 0)
            printf("Trying to unreserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (uint64_t t = 0; t < PageCount; t++)
            this->UnreservePage((void *)((uint64_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::Init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
    {
        printf("Initializing physical memory manager...\n");
        EFI_MEMORY_DESCRIPTOR *memDesc = nullptr;
        UINTN MapSize, MapKey;
        UINTN DescriptorSize;
        UINT32 DescriptorVersion;
        {
            SystemTable->BootServices->GetMemoryMap(&MapSize, memDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
            SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void **)&memDesc);
            SystemTable->BootServices->GetMemoryMap(&MapSize, memDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
        }

        uint64_t MemoryMapSize = MapSize / DescriptorSize;
        static uint64_t MemorySizeBytes = 0;

        void *LargestFreeMemorySegment = nullptr;
        uint64_t LargestFreeMemorySegmentSize = 0;
        uint64_t MemorySize = MapSize;
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        for (int i = 0; i < MemoryMapSize; i++)
        {
            EFI_MEMORY_DESCRIPTOR *Descriptor = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)memDesc + (i * DescriptorSize));
            MemorySizeBytes += Descriptor->NumberOfPages * 4096;
            switch (Descriptor->Type)
            {
            case EfiConventionalMemory:
                if ((Descriptor->NumberOfPages * 4096) > LargestFreeMemorySegmentSize)
                {
                    LargestFreeMemorySegment = (void *)Descriptor->PhysicalStart;
                    LargestFreeMemorySegmentSize = Descriptor->NumberOfPages * 4096;
                    printf("Largest free memory segment: %p (%dKB)",
                           (void *)Descriptor->PhysicalStart,
                           ((Descriptor->NumberOfPages * 4096) / 1024));
                }
                break;
            }
        }

        uint64_t BitmapSize = ALIGN_UP((MemorySize / 0x1000) / 8, 0x1000);
        printf("Initializing Bitmap (%p %dKB)", LargestFreeMemorySegment, (BitmapSize / 1024));

        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)LargestFreeMemorySegment;
        for (uint64_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        this->ReservePages(0, MemorySize / PAGE_SIZE + 1);
        for (uint64_t i = 0; i < MemoryMapSize; i++)
        {
            EFI_MEMORY_DESCRIPTOR *Descriptor = (EFI_MEMORY_DESCRIPTOR *)((uint64_t)memDesc + (i * DescriptorSize));
            if (Descriptor->Type == EfiConventionalMemory)
                this->UnreservePages((void *)Descriptor->PhysicalStart, (Descriptor->NumberOfPages * 4096) / PAGE_SIZE + 1);
        }
        this->ReservePages(0, 0x100); // Reserve between 0 and 0x100000
        this->LockPages(PageBitmap.Buffer, PageBitmap.Size / PAGE_SIZE + 1);
    }

    Physical::Physical() {}
    Physical::~Physical() {}
}
