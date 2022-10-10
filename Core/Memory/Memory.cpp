#include <memory.hpp>

#include <string.h>
#include <debug.h>

#include "HeapAllocators/Xalloc.hpp"
#include "../Library/liballoc_1_1.h"

using namespace Memory;

Physical KernelAllocator;
PageTable *KernelPageTable = nullptr;

enum MemoryAllocatorType
{
    None,
    Pages,
    XallocV1,
    liballoc11
};

static MemoryAllocatorType AllocatorType = MemoryAllocatorType::None;
Xalloc::AllocatorV1 *XallocV1Allocator = nullptr;

#ifdef DEBUG
void tracepagetable(PageTable *pt)
{
    for (int i = 0; i < 512; i++)
    {
        if (pt->Entries[i].Value.Present)
            debug("Entry %03d: %x %x %x %x %x %x %x %x %x %x %x %p-%#lx", i,
                  pt->Entries[i].Value.Present, pt->Entries[i].Value.ReadWrite,
                  pt->Entries[i].Value.UserSupervisor, pt->Entries[i].Value.WriteThrough,
                  pt->Entries[i].Value.CacheDisable, pt->Entries[i].Value.Accessed,
                  pt->Entries[i].Value.Dirty, pt->Entries[i].Value.PageSize,
                  pt->Entries[i].Value.Global, pt->Entries[i].Value.PageAttributeTable,
                  pt->Entries[i].Value.ExecuteDisable, pt->Entries[i].GetAddress(),
                  pt->Entries[i].Value);
    }
}
#endif

void InitializeMemoryManagement(BootInfo *Info)
{
    trace("Initializing Physical Memory Manager");
    KernelAllocator = Physical();
    KernelAllocator.Init(Info);
    debug("Memory Info: %dMB / %dMB (%dMB reserved)",
          TO_MB(KernelAllocator.GetUsedMemory()),
          TO_MB(KernelAllocator.GetTotalMemory()),
          TO_MB(KernelAllocator.GetReservedMemory()));

    AllocatorType = MemoryAllocatorType::Pages;

    trace("Initializing Virtual Memory Manager");
    KernelPageTable = (PageTable *)KernelAllocator.RequestPage();
    memset(KernelPageTable, 0, PAGE_SIZE);
    Virtual kva = Virtual(KernelPageTable);

    uint64_t KernelStart = (uint64_t)&_kernel_start;
    uint64_t KernelTextEnd = (uint64_t)&_kernel_text_end;
    uint64_t KernelDataEnd = (uint64_t)&_kernel_data_end;
    uint64_t KernelRoDataEnd = (uint64_t)&_kernel_rodata_end;
    uint64_t KernelEnd = (uint64_t)&_kernel_end;

    uint64_t VirtualOffsetNormalVMA = NORMAL_VMA_OFFSET;
    uint64_t BaseKernelMapAddress = (uint64_t)Info->Kernel.PhysicalBase;

    for (uint64_t t = 0; t < Info->Memory.Size; t += PAGE_SIZE)
    {
        kva.Map((void *)t, (void *)t, PTFlag::RW);
        kva.Map((void *)VirtualOffsetNormalVMA, (void *)t, PTFlag::RW);
        VirtualOffsetNormalVMA += PAGE_SIZE;
    }

    /* Mapping Framebuffer address */
    int itrfb = 0;
    while (1)
    {
        if (!Info->Framebuffer[itrfb].BaseAddress)
            break;

        for (uint64_t fb_base = (uint64_t)Info->Framebuffer[itrfb].BaseAddress;
             fb_base < ((uint64_t)Info->Framebuffer[itrfb].BaseAddress + ((Info->Framebuffer[itrfb].Pitch * Info->Framebuffer[itrfb].Height) + PAGE_SIZE));
             fb_base += PAGE_SIZE)
            kva.Map((void *)(fb_base + NORMAL_VMA_OFFSET), (void *)fb_base, PTFlag::RW | PTFlag::US);
        itrfb++;
    }

    /* Kernel mapping */
    for (uint64_t k = KernelStart; k < KernelTextEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelTextEnd; k < KernelDataEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelDataEnd; k < KernelRoDataEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::P);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelRoDataEnd; k < KernelEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    debug("\nStart: %#llx - Text End: %#llx - RoEnd: %#llx - End: %#llx\nStart Physical: %#llx - End Physical: %#llx",
          KernelStart, KernelTextEnd, KernelRoDataEnd, KernelEnd, Info->Kernel.PhysicalBase, BaseKernelMapAddress - PAGE_SIZE);

    /*    KernelStart             KernelTextEnd       KernelRoDataEnd                  KernelEnd
    Kernel Start & Text Start ------ Text End ------ Kernel Rodata End ------ Kernel Data End & Kernel End
    */
    trace("Applying new page table from address %p", KernelPageTable);
#ifdef DEBUG
    tracepagetable(KernelPageTable);
#endif
#if defined(__amd64__) || defined(__i386__)
    asmv("mov %0, %%cr3" ::"r"(KernelPageTable));
#elif defined(__aarch64__)
    asmv("msr ttbr0_el1, %0" ::"r"(KernelPageTable));
#endif
    if (strstr(Info->Kernel.CommandLine, "xallocv1"))
    {
        XallocV1Allocator = new Xalloc::AllocatorV1((void *)KERNEL_HEAP_BASE, false, false);
        AllocatorType = MemoryAllocatorType::XallocV1;
        trace("XallocV1 Allocator initialized (%p)", XallocV1Allocator);
    }
    else if (strstr(Info->Kernel.CommandLine, "liballoc11"))
    {
        AllocatorType = MemoryAllocatorType::liballoc11;
    }
}

void *HeapMalloc(uint64_t Size)
{
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
        return KernelAllocator.RequestPages(TO_PAGES(Size));
    case MemoryAllocatorType::XallocV1:
        return XallocV1Allocator->Malloc(Size);
    case MemoryAllocatorType::liballoc11:
        return PREFIX(malloc)(Size);
    default:
        throw;
    }
}

void *HeapCalloc(uint64_t n, uint64_t Size)
{
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
        return KernelAllocator.RequestPages(TO_PAGES(n * Size));
    case MemoryAllocatorType::XallocV1:
        return XallocV1Allocator->Calloc(n, Size);
    case MemoryAllocatorType::liballoc11:
        return PREFIX(calloc)(n, Size);
    default:
        throw;
    }
}

void *HeapRealloc(void *Address, uint64_t Size)
{
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
        return KernelAllocator.RequestPages(TO_PAGES(Size)); // WARNING: Potential memory leak
    case MemoryAllocatorType::XallocV1:
        return XallocV1Allocator->Realloc(Address, Size);
    case MemoryAllocatorType::liballoc11:
        return PREFIX(realloc)(Address, Size);
    default:
        throw;
    }
}

void HeapFree(void *Address)
{
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
        KernelAllocator.FreePage(Address); // WARNING: Potential memory leak
        break;
    case MemoryAllocatorType::XallocV1:
        XallocV1Allocator->Free(Address);
        break;
    case MemoryAllocatorType::liballoc11:
        PREFIX(free)
        (Address);
        break;
    default:
        throw;
    }
}

void *operator new(uint64_t Size) { return HeapMalloc(Size); }
void *operator new[](uint64_t Size) { return HeapMalloc(Size); }
void operator delete(void *Pointer) { HeapFree(Pointer); }
void operator delete[](void *Pointer) { HeapFree(Pointer); }
void operator delete(void *Pointer, long unsigned int Size) { HeapFree(Pointer); }
void operator delete[](void *Pointer, long unsigned int Size) { HeapFree(Pointer); }
