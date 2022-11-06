#include <memory.hpp>

#include <convert.h>
#include <debug.h>

#include "HeapAllocators/Xalloc.hpp"
#include "../Library/liballoc_1_1.h"

using namespace Memory;

Physical KernelAllocator;
PageTable *KernelPageTable = nullptr;
PageTable *UserspaceKernelOnlyPageTable = nullptr;

static MemoryAllocatorType AllocatorType = MemoryAllocatorType::None;
Xalloc::AllocatorV1 *XallocV1Allocator = nullptr;

#ifdef DEBUG
void tracepagetable(PageTable *pt)
{
    for (int i = 0; i < 512; i++)
    {
#if defined(__amd64__)
        if (pt->Entries[i].Value.Present)
            debug("Entry %03d: %x %x %x %x %x %x %x %x %x %x %x %p-%#llx", i,
                  pt->Entries[i].Value.Present, pt->Entries[i].Value.ReadWrite,
                  pt->Entries[i].Value.UserSupervisor, pt->Entries[i].Value.WriteThrough,
                  pt->Entries[i].Value.CacheDisable, pt->Entries[i].Value.Accessed,
                  pt->Entries[i].Value.Dirty, pt->Entries[i].Value.PageSize,
                  pt->Entries[i].Value.Global, pt->Entries[i].Value.PageAttributeTable,
                  pt->Entries[i].Value.ExecuteDisable, pt->Entries[i].GetAddress(),
                  pt->Entries[i].Value);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }
}
#endif

void MapFromZero(PageTable *PT, BootInfo *Info)
{
    Virtual va = Virtual(PT);
    uint64_t VirtualOffsetNormalVMA = NORMAL_VMA_OFFSET;
    for (uint64_t t = 0; t < Info->Memory.Size; t += PAGE_SIZE)
    {
        va.Map((void *)t, (void *)t, PTFlag::RW | PTFlag::US);
        va.Map((void *)VirtualOffsetNormalVMA, (void *)t, PTFlag::RW | PTFlag::US);
        VirtualOffsetNormalVMA += PAGE_SIZE;
    }
}

void MapFramebuffer(PageTable *PT, BootInfo *Info)
{
    Virtual va = Virtual(PT);
    int itrfb = 0;
    while (1)
    {
        if (!Info->Framebuffer[itrfb].BaseAddress)
            break;

        for (uint64_t fb_base = (uint64_t)Info->Framebuffer[itrfb].BaseAddress;
             fb_base < ((uint64_t)Info->Framebuffer[itrfb].BaseAddress + ((Info->Framebuffer[itrfb].Pitch * Info->Framebuffer[itrfb].Height) + PAGE_SIZE));
             fb_base += PAGE_SIZE)
            va.Map((void *)(fb_base + NORMAL_VMA_OFFSET), (void *)fb_base, PTFlag::RW | PTFlag::US);
        itrfb++;
    }
}

void MapKernel(PageTable *PT, BootInfo *Info)
{
    /*    KernelStart             KernelTextEnd       KernelRoDataEnd                  KernelEnd
    Kernel Start & Text Start ------ Text End ------ Kernel Rodata End ------ Kernel Data End & Kernel End
    */
    Virtual va = Virtual(PT);
    uint64_t KernelStart = (uint64_t)&_kernel_start;
    uint64_t KernelTextEnd = (uint64_t)&_kernel_text_end;
    uint64_t KernelDataEnd = (uint64_t)&_kernel_data_end;
    uint64_t KernelRoDataEnd = (uint64_t)&_kernel_rodata_end;
    uint64_t KernelEnd = (uint64_t)&_kernel_end;

    uint64_t BaseKernelMapAddress = (uint64_t)Info->Kernel.PhysicalBase;
    for (uint64_t k = KernelStart; k < KernelTextEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelTextEnd; k < KernelDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelDataEnd; k < KernelRoDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::P);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = KernelRoDataEnd; k < KernelEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    debug("\nStart: %#llx - Text End: %#llx - RoEnd: %#llx - End: %#llx\nStart Physical: %#llx - End Physical: %#llx",
          KernelStart, KernelTextEnd, KernelRoDataEnd, KernelEnd, Info->Kernel.PhysicalBase, BaseKernelMapAddress - PAGE_SIZE);
}

void InitializeMemoryManagement(BootInfo *Info)
{
    for (uint64_t i = 0; i < Info->Memory.Entries; i++)
    {
        uint64_t Base = reinterpret_cast<uint64_t>(Info->Memory.Entry[i].BaseAddress);
        uint64_t Length = Info->Memory.Entry[i].Length;
        uint64_t End = Base + Length;
        const char *Type = "Unknown";

        switch (Info->Memory.Entry[i].Type)
        {
        case Usable:
            Type = "Usable";
            break;
        case Reserved:
            Type = "Reserved";
            break;
        case ACPIReclaimable:
            Type = "ACPI Reclaimable";
            break;
        case ACPINVS:
            Type = "ACPI NVS";
            break;
        case BadMemory:
            Type = "Bad Memory";
            break;
        case BootloaderReclaimable:
            Type = "Bootloader Reclaimable";
            break;
        case KernelAndModules:
            Type = "Kernel and Modules";
            break;
        case Framebuffer:
            Type = "Framebuffer";
            break;
        default:
            break;
        }

        trace("%lld: %#016llx-%#016llx %s",
              i,
              Base,
              End,
              Type);
    }

    trace("Initializing Physical Memory Manager");
    KernelAllocator = Physical();
    KernelAllocator.Init(Info);
    debug("Memory Info: %lldMB / %lldMB (%lldMB reserved)",
          TO_MB(KernelAllocator.GetUsedMemory()),
          TO_MB(KernelAllocator.GetTotalMemory()),
          TO_MB(KernelAllocator.GetReservedMemory()));

    AllocatorType = MemoryAllocatorType::Pages;

    trace("Initializing Virtual Memory Manager");
    KernelPageTable = (PageTable *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
    memset(KernelPageTable, 0, PAGE_SIZE);

    UserspaceKernelOnlyPageTable = (PageTable *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
    memset(UserspaceKernelOnlyPageTable, 0, PAGE_SIZE);

    debug("Mapping from %#llx to %#llx", 0, Info->Memory.Size);
    MapFromZero(KernelPageTable, Info);

    /* Mapping Framebuffer address */
    debug("Mapping Framebuffer");
    MapFramebuffer(KernelPageTable, Info);
    debug("Mapping Framebuffer for Userspace Page Table");
    MapFramebuffer(UserspaceKernelOnlyPageTable, Info);

    /* Kernel mapping */
    debug("Mapping Kernel");
    MapKernel(KernelPageTable, Info);
    debug("Mapping Kernel for Userspace Page Table");
    MapKernel(UserspaceKernelOnlyPageTable, Info);

    trace("Applying new page table from address %p", KernelPageTable);
#ifdef DEBUG
    debug("Kernel:");
    tracepagetable(KernelPageTable);
    debug("Userspace:");
    tracepagetable(UserspaceKernelOnlyPageTable);
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
    {
        void *ret = PREFIX(malloc)(Size);
        memset(ret, 0, Size);
        return ret;
    }
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
    {
        void *ret = PREFIX(calloc)(n, Size);
        memset(ret, 0, Size);
        return ret;
    }
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
    {
        void *ret = PREFIX(realloc)(Address, Size);
        memset(ret, 0, Size);
        return ret;
    }
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

void *operator new(size_t Size) { return HeapMalloc(Size); }
void *operator new[](size_t Size) { return HeapMalloc(Size); }
void *operator new(unsigned long Size, std::align_val_t Alignment)
{
    fixme("operator new with alignment(%#lx) is not implemented", Alignment);
    return HeapMalloc(Size);
}
void operator delete(void *Pointer) { HeapFree(Pointer); }
void operator delete[](void *Pointer) { HeapFree(Pointer); }
void operator delete(void *Pointer, long unsigned int Size) { HeapFree(Pointer); }
void operator delete[](void *Pointer, long unsigned int Size) { HeapFree(Pointer); }
