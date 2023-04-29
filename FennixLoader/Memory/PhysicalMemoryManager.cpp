#include <memory.hpp>
namespace Memory32
{
    void *Physical::RequestPage()
    {
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        while (1)
            asmv("cli; hlt");
        __builtin_unreachable();
    }

    void *Physical::RequestPages(size_t Count)
    {
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            for (uint64_t Index = PageBitmapIndex; Index < PageBitmap.Size * 8; Index++)
            {
                if (PageBitmap[Index] == true)
                    continue;

                for (size_t i = 0; i < Count; i++)
                {
                    if (PageBitmap[Index + i] == true)
                        goto NextPage;
                }

                this->LockPages((void *)(Index * PAGE_SIZE), Count);
                return (void *)(Index * PAGE_SIZE);

            NextPage:
                Index += Count;
                continue;
            }
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        while (1)
            asmv("cli; hlt");
        __builtin_unreachable();
    }

    void Physical::FreePage(void *Address)
    {
        if (unlikely(Address == nullptr))
        {
            warn("Null pointer passed to FreePage.");
            return;
        }

        size_t Index = (size_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == false))
        {
            warn("Tried to free an already free page. (%p)", Address);
            return;
        }

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            UsedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }
    }

    void Physical::FreePages(void *Address, size_t Count)
    {
        if (unlikely(Address == nullptr || Count == 0))
        {
            warn("%s%s%s passed to FreePages.", Address == nullptr ? "Null pointer " : "", Address == nullptr && Count == 0 ? "and " : "", Count == 0 ? "Zero count" : "");
            return;
        }
        for (size_t t = 0; t < Count; t++)
            this->FreePage((void *)((uintptr_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::LockPage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to lock null address.");

        uintptr_t Index = (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            UsedMemory += PAGE_SIZE;
        }
    }

    void Physical::LockPages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to lock %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t i = 0; i < PageCount; i++)
            this->LockPage((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
    }

    void Physical::ReservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to reserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            ReservedMemory += PAGE_SIZE;
        }
    }

    void Physical::ReservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to reserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

            if (unlikely(PageBitmap[Index] == true))
                return;

            if (PageBitmap.Set(Index, true))
            {
                FreeMemory -= PAGE_SIZE;
                ReservedMemory += PAGE_SIZE;
            }
        }
    }

    void Physical::UnreservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to unreserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

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

    void Physical::UnreservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to unreserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

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
    }

    void Physical::Init(BootInfo *Info)
    {
        uint64_t MemorySize = Info->Memory.Size;
        debug("Memory size: %lld bytes (%ld pages)", MemorySize, TO_PAGES(MemorySize));
        UsedMemory = 0;
        ReservedMemory = 0;
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        void *LargestFreeMemorySegment = nullptr;
        uint64_t LargestFreeMemorySegmentSize = 0;

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable)
            {
                if (Info->Memory.Entry[i].Length > LargestFreeMemorySegmentSize)
                {
                    /* We don't want to use 0 as a memory address. */
                    if (Info->Memory.Entry[i].BaseAddress == 0x0)
                        continue;

                    LargestFreeMemorySegment = (void *)Info->Memory.Entry[i].BaseAddress;
                    LargestFreeMemorySegmentSize = Info->Memory.Entry[i].Length;

                    debug("Largest free memory segment: %llp (%lldMB)",
                          (void *)Info->Memory.Entry[i].BaseAddress,
                          TO_MB(Info->Memory.Entry[i].Length));
                }
            }
        }

        if (LargestFreeMemorySegment == nullptr)
        {
            error("No free memory found!");
            while (1)
                asmv("cli; hlt");
        }

        size_t BitmapSize = (MemorySize / PAGE_SIZE) / 8 + 1;
        void *LargestFreeMemorySegmentNoKernel = nullptr;

        if ((uintptr_t)LargestFreeMemorySegment >= (uintptr_t)&_kernel_start &&
            (uintptr_t)LargestFreeMemorySegment <= (uintptr_t)&_kernel_end)
        {
            LargestFreeMemorySegmentNoKernel = (void *)((uintptr_t)LargestFreeMemorySegment + (uintptr_t)&_kernel_end);
            if ((uintptr_t)LargestFreeMemorySegmentNoKernel > ((uintptr_t)LargestFreeMemorySegment + LargestFreeMemorySegmentSize))
            {
                error("No free memory found!");
                while (1)
                    asmv("cli; hlt");
            }
        }
        else
        {
            LargestFreeMemorySegmentNoKernel = LargestFreeMemorySegment;
        }

        debug("Initializing Bitmap at %llp-%llp (%lld Bytes)",
              LargestFreeMemorySegmentNoKernel,
              (void *)((uintptr_t)LargestFreeMemorySegmentNoKernel + BitmapSize),
              BitmapSize);

        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)LargestFreeMemorySegmentNoKernel;
        for (size_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        debug("Reserving pages...");
        this->ReservePages(0, TO_PAGES(Info->Memory.Size));
        debug("Unreserving usable pages...");

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable)
                this->UnreservePages(Info->Memory.Entry[i].BaseAddress, TO_PAGES(Info->Memory.Entry[i].Length));
        }

        debug("Reserving bitmap pages...");
        this->ReservePages(PageBitmap.Buffer, TO_PAGES(PageBitmap.Size));
        debug("Reserving kernel...");
        this->ReservePages(&_kernel_start, TO_PAGES((uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start));
        debug("Reserving kernel and its modules...");

        BootInfo::ModuleInfo *Module = &Info->Modules[0];
        int ModuleIndex = 0;
        do
        {
            debug("Reserving %s (%p-%p)", Module->CommandLine, Module->Address, (void *)((uintptr_t)Module->Address + Module->Size));
            this->ReservePages(Module->Address, TO_PAGES(Module->Size));
            Module = &Info->Modules[++ModuleIndex];
        } while (Module->Address);

        this->ReservePages(0, 16);
    }

    Physical::Physical() {}
    Physical::~Physical() {}
}

namespace Memory64
{
    void *Physical::RequestPage()
    {
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            this->LockPage((void *)(PageBitmapIndex * PAGE_SIZE));
            return (void *)(PageBitmapIndex * PAGE_SIZE);
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        while (1)
            asmv("cli; hlt");
        __builtin_unreachable();
    }

    void *Physical::RequestPages(size_t Count)
    {
        for (; PageBitmapIndex < PageBitmap.Size * 8; PageBitmapIndex++)
        {
            if (PageBitmap[PageBitmapIndex] == true)
                continue;

            for (uint64_t Index = PageBitmapIndex; Index < PageBitmap.Size * 8; Index++)
            {
                if (PageBitmap[Index] == true)
                    continue;

                for (size_t i = 0; i < Count; i++)
                {
                    if (PageBitmap[Index + i] == true)
                        goto NextPage;
                }

                this->LockPages((void *)(Index * PAGE_SIZE), Count);
                return (void *)(Index * PAGE_SIZE);

            NextPage:
                Index += Count;
                continue;
            }
        }

        error("Out of memory! (Free: %ldMB; Used: %ldMB; Reserved: %ldMB)", TO_MB(FreeMemory), TO_MB(UsedMemory), TO_MB(ReservedMemory));
        while (1)
            asmv("cli; hlt");
        __builtin_unreachable();
    }

    void Physical::FreePage(void *Address)
    {
        if (unlikely(Address == nullptr))
        {
            warn("Null pointer passed to FreePage.");
            return;
        }

        size_t Index = (size_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == false))
        {
            warn("Tried to free an already free page. (%p)", Address);
            return;
        }

        if (PageBitmap.Set(Index, false))
        {
            FreeMemory += PAGE_SIZE;
            UsedMemory -= PAGE_SIZE;
            if (PageBitmapIndex > Index)
                PageBitmapIndex = Index;
        }
    }

    void Physical::FreePages(void *Address, size_t Count)
    {
        if (unlikely(Address == nullptr || Count == 0))
        {
            warn("%s%s%s passed to FreePages.", Address == nullptr ? "Null pointer " : "", Address == nullptr && Count == 0 ? "and " : "", Count == 0 ? "Zero count" : "");
            return;
        }
        for (size_t t = 0; t < Count; t++)
            this->FreePage((void *)((uintptr_t)Address + (t * PAGE_SIZE)));
    }

    void Physical::LockPage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to lock null address.");

        uintptr_t Index = (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            UsedMemory += PAGE_SIZE;
        }
    }

    void Physical::LockPages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to lock %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t i = 0; i < PageCount; i++)
            this->LockPage((void *)((uintptr_t)Address + (i * PAGE_SIZE)));
    }

    void Physical::ReservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to reserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

        if (unlikely(PageBitmap[Index] == true))
            return;

        if (PageBitmap.Set(Index, true))
        {
            FreeMemory -= PAGE_SIZE;
            ReservedMemory += PAGE_SIZE;
        }
    }

    void Physical::ReservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to reserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

            if (unlikely(PageBitmap[Index] == true))
                return;

            if (PageBitmap.Set(Index, true))
            {
                FreeMemory -= PAGE_SIZE;
                ReservedMemory += PAGE_SIZE;
            }
        }
    }

    void Physical::UnreservePage(void *Address)
    {
        if (unlikely(Address == nullptr))
            warn("Trying to unreserve null address.");

        uintptr_t Index = (Address == NULL) ? 0 : (uintptr_t)Address / PAGE_SIZE;

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

    void Physical::UnreservePages(void *Address, size_t PageCount)
    {
        if (unlikely(Address == nullptr || PageCount == 0))
            warn("Trying to unreserve %s%s.", Address ? "null address" : "", PageCount ? "0 pages" : "");

        for (size_t t = 0; t < PageCount; t++)
        {
            uintptr_t Index = ((uintptr_t)Address + (t * PAGE_SIZE)) / PAGE_SIZE;

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
    }

    void Physical::Init(BootInfo *Info)
    {
        uint64_t MemorySize = Info->Memory.Size;
        debug("Memory size: %lld bytes (%ld pages)", MemorySize, TO_PAGES(MemorySize));
        UsedMemory = 0;
        ReservedMemory = 0;
        TotalMemory = MemorySize;
        FreeMemory = MemorySize;

        void *LargestFreeMemorySegment = nullptr;
        uint64_t LargestFreeMemorySegmentSize = 0;

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable)
            {
                if (Info->Memory.Entry[i].Length > LargestFreeMemorySegmentSize)
                {
                    /* We don't want to use 0 as a memory address. */
                    if (Info->Memory.Entry[i].BaseAddress == 0x0)
                        continue;

                    LargestFreeMemorySegment = (void *)Info->Memory.Entry[i].BaseAddress;
                    LargestFreeMemorySegmentSize = Info->Memory.Entry[i].Length;

                    debug("Largest free memory segment: %llp (%lldMB)",
                          (void *)Info->Memory.Entry[i].BaseAddress,
                          TO_MB(Info->Memory.Entry[i].Length));
                }
            }
        }

        if (LargestFreeMemorySegment == nullptr)
        {
            error("No free memory found!");
            while (1)
                asmv("cli; hlt");
        }

        size_t BitmapSize = (MemorySize / PAGE_SIZE) / 8 + 1;
        void *LargestFreeMemorySegmentNoKernel = nullptr;

        if ((uintptr_t)LargestFreeMemorySegment >= (uintptr_t)&_kernel_start &&
            (uintptr_t)LargestFreeMemorySegment <= (uintptr_t)&_kernel_end)
        {
            LargestFreeMemorySegmentNoKernel = (void *)((uintptr_t)LargestFreeMemorySegment + (uintptr_t)&_kernel_end);
            if ((uintptr_t)LargestFreeMemorySegmentNoKernel > ((uintptr_t)LargestFreeMemorySegment + LargestFreeMemorySegmentSize))
            {
                error("No free memory found!");
                while (1)
                    asmv("cli; hlt");
            }
        }
        else
        {
            LargestFreeMemorySegmentNoKernel = LargestFreeMemorySegment;
        }

        debug("Initializing Bitmap at %llp-%llp (%lld Bytes)",
              LargestFreeMemorySegmentNoKernel,
              (void *)((uintptr_t)LargestFreeMemorySegmentNoKernel + BitmapSize),
              BitmapSize);

        PageBitmap.Size = BitmapSize;
        PageBitmap.Buffer = (uint8_t *)LargestFreeMemorySegmentNoKernel;
        for (size_t i = 0; i < BitmapSize; i++)
            *(uint8_t *)(PageBitmap.Buffer + i) = 0;

        debug("Reserving pages...");
        this->ReservePages(0, TO_PAGES(Info->Memory.Size));
        debug("Unreserving usable pages...");

        for (uint64_t i = 0; i < Info->Memory.Entries; i++)
        {
            if (Info->Memory.Entry[i].Type == Usable)
                this->UnreservePages(Info->Memory.Entry[i].BaseAddress, TO_PAGES(Info->Memory.Entry[i].Length));
        }

        debug("Reserving bitmap pages...");
        this->ReservePages(PageBitmap.Buffer, TO_PAGES(PageBitmap.Size));
        debug("Reserving fennix loader...");
        this->ReservePages(&_kernel_start, TO_PAGES((uintptr_t)&_kernel_end - (uintptr_t)&_kernel_start));
        debug("Reserving kernel and its modules...");

        BootInfo::ModuleInfo *Module = &Info->Modules[0];
        int ModuleIndex = 0;
        do
        {
            debug("Reserving %s (%p-%p)", Module->CommandLine, Module->Address, (void *)((uintptr_t)Module->Address + Module->Size));
            this->ReservePages(Module->Address, TO_PAGES(Module->Size));
            Module = &Info->Modules[++ModuleIndex];
        } while (Module->Address);
        this->ReservePages(0, 16);
    }

    Physical::Physical() {}
    Physical::~Physical() {}
}
