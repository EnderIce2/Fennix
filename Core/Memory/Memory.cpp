#include <memory.hpp>

#include <convert.h>
#include <lock.hpp>
#include <debug.h>

#include "HeapAllocators/Xalloc.hpp"
#include "../Library/liballoc_1_1.h"
#include "../../kernel.h"

// #define DEBUG_ALLOCATIONS_SL 1
// #define DEBUG_ALLOCATIONS 1

#ifdef DEBUG_ALLOCATIONS
#define memdbg(m, ...)       \
    debug(m, ##__VA_ARGS__); \
    __sync_synchronize()
#else
#define memdbg(m, ...)
#endif

using namespace Memory;

#ifdef DEBUG_ALLOCATIONS_SL
NewLock(AllocatorLock);
NewLock(OperatorAllocatorLock);
#endif

Physical KernelAllocator;
PageTable4 *KernelPageTable = nullptr;
PageTable4 *UserspaceKernelOnlyPageTable = nullptr;
void *KPT = nullptr;

static MemoryAllocatorType AllocatorType = MemoryAllocatorType::None;
Xalloc::AllocatorV1 *XallocV1Allocator = nullptr;

#ifdef DEBUG
__no_instrument_function void tracepagetable(PageTable4 *pt)
{
    for (int i = 0; i < 512; i++)
    {
#if defined(__amd64__)
        if (pt->Entries[i].Present)
            debug("Entry %03d: %x %x %x %x %x %x %x %p-%#llx", i,
                  pt->Entries[i].Present, pt->Entries[i].ReadWrite,
                  pt->Entries[i].UserSupervisor, pt->Entries[i].WriteThrough,
                  pt->Entries[i].CacheDisable, pt->Entries[i].Accessed,
                  pt->Entries[i].ExecuteDisable, pt->Entries[i].Address << 12,
                  pt->Entries[i]);
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif
    }
}
#endif

__no_instrument_function void MapFromZero(PageTable4 *PT, BootInfo *Info)
{
    static int once = 0;
    if (!once++)
    {
        Virtual va = Virtual(PT);
        void *NullAddress = KernelAllocator.RequestPage();
        memset(NullAddress, 0, PAGE_SIZE); // TODO: If the CPU instruction pointer hits this page, there should be function to handle it. (memcpy assembly code?)
        va.Map((void *)0, (void *)NullAddress, PTFlag::RW | PTFlag::US);
        uintptr_t VirtualOffsetNormalVMA = NORMAL_VMA_OFFSET;
        size_t MemSize = Info->Memory.Size;
        for (size_t t = 0; t < MemSize; t += PAGE_SIZE)
        {
            va.Map((void *)t, (void *)t, PTFlag::RW /* | PTFlag::US */);
            va.Map((void *)VirtualOffsetNormalVMA, (void *)t, PTFlag::RW /* | PTFlag::US */);
            VirtualOffsetNormalVMA += PAGE_SIZE;
        }
    }
    else
    {
        error("MapFromZero() called more than once!");
        CPU::Stop();
    }
}

__no_instrument_function void MapFramebuffer(PageTable4 *PT, BootInfo *Info)
{
    Virtual va = Virtual(PT);
    int itrfb = 0;
    while (1)
    {
        if (!Info->Framebuffer[itrfb].BaseAddress)
            break;

        for (uintptr_t fb_base = (uintptr_t)Info->Framebuffer[itrfb].BaseAddress;
             fb_base < ((uintptr_t)Info->Framebuffer[itrfb].BaseAddress + ((Info->Framebuffer[itrfb].Pitch * Info->Framebuffer[itrfb].Height) + PAGE_SIZE));
             fb_base += PAGE_SIZE)
            va.Map((void *)fb_base, (void *)fb_base, PTFlag::RW | PTFlag::US | PTFlag::G);
        itrfb++;
    }
}

__no_instrument_function void MapKernel(PageTable4 *PT, BootInfo *Info)
{
    /*    KernelStart             KernelTextEnd       KernelRoDataEnd                  KernelEnd
    Kernel Start & Text Start ------ Text End ------ Kernel Rodata End ------ Kernel Data End & Kernel End
    */
    Virtual va = Virtual(PT);
    uintptr_t KernelStart = (uintptr_t)&_kernel_start;
    uintptr_t KernelTextEnd = (uintptr_t)&_kernel_text_end;
    uintptr_t KernelDataEnd = (uintptr_t)&_kernel_data_end;
    uintptr_t KernelRoDataEnd = (uintptr_t)&_kernel_rodata_end;
    uintptr_t KernelEnd = (uintptr_t)&_kernel_end;

    uintptr_t BaseKernelMapAddress = (uintptr_t)Info->Kernel.PhysicalBase;
    uintptr_t k;

    for (k = KernelStart; k < KernelTextEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (k = KernelTextEnd; k < KernelDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW | PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (k = KernelDataEnd; k < KernelRoDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::P | PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (k = KernelRoDataEnd; k < KernelEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW | PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    debug("\nStart: %#llx - Text End: %#llx - RoEnd: %#llx - End: %#llx\nStart Physical: %#llx - End Physical: %#llx",
          KernelStart, KernelTextEnd, KernelRoDataEnd, KernelEnd, Info->Kernel.PhysicalBase, BaseKernelMapAddress - PAGE_SIZE);
}

__no_instrument_function void InitializeMemoryManagement(BootInfo *Info)
{
#ifdef DEBUG
    for (uint64_t i = 0; i < Info->Memory.Entries; i++)
    {
        uintptr_t Base = reinterpret_cast<uintptr_t>(Info->Memory.Entry[i].BaseAddress);
        uintptr_t Length = Info->Memory.Entry[i].Length;
        uintptr_t End = Base + Length;
        const char *Type = "Unknown";

        switch (Info->Memory.Entry[i].Type)
        {
        case likely(Usable):
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

        debug("%lld: %#016llx-%#016llx %s",
              i,
              Base,
              End,
              Type);
    }
#endif

    trace("Initializing Physical Memory Manager");
    // KernelAllocator = Physical(); <- Already called in the constructor
    KernelAllocator.Init(Info);
    debug("Memory Info: %lldMB / %lldMB (%lldMB reserved)",
          TO_MB(KernelAllocator.GetUsedMemory()),
          TO_MB(KernelAllocator.GetTotalMemory()),
          TO_MB(KernelAllocator.GetReservedMemory()));

    AllocatorType = MemoryAllocatorType::Pages;

    trace("Initializing Virtual Memory Manager");
    KernelPageTable = (PageTable4 *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
    memset(KernelPageTable, 0, PAGE_SIZE);

    UserspaceKernelOnlyPageTable = (PageTable4 *)KernelAllocator.RequestPages(TO_PAGES(PAGE_SIZE));
    memset(UserspaceKernelOnlyPageTable, 0, PAGE_SIZE);

    debug("Mapping from 0x0 to %#llx", Info->Memory.Size);
    MapFromZero(KernelPageTable, Info);
    debug("Mapping from 0x0 %#llx for Userspace Page Table", Info->Memory.Size);
    UserspaceKernelOnlyPageTable[0] = KernelPageTable[0];

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
    KPT = KernelPageTable;
#if defined(__amd64__) || defined(__i386__)
    asmv("mov %0, %%cr3" ::"r"(KPT));
#elif defined(__aarch64__)
    asmv("msr ttbr0_el1, %0" ::"r"(KPT));
#endif
    debug("Page table updated.");
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

void *HeapMalloc(size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("malloc(%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
        return KernelAllocator.RequestPages(TO_PAGES(Size));
    case MemoryAllocatorType::XallocV1:
    {
        void *ret = XallocV1Allocator->Malloc(Size);
        memset(ret, 0, Size);
        return ret;
    }
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

void *HeapCalloc(size_t n, size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("calloc(%d, %d)->[%s]", n, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
        return KernelAllocator.RequestPages(TO_PAGES(n * Size));
    case MemoryAllocatorType::XallocV1:
    {
        void *ret = XallocV1Allocator->Calloc(n, Size);
        memset(ret, 0, n * Size);
        return ret;
    }
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

void *HeapRealloc(void *Address, size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("realloc(%#lx, %d)->[%s]", Address, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    if (unlikely(!Address))
    {
        error("Attempt to realloc a null pointer");
        return nullptr;
    }

    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
        return KernelAllocator.RequestPages(TO_PAGES(Size)); // WARNING: Potential memory leak
    case MemoryAllocatorType::XallocV1:
    {
        void *ret = XallocV1Allocator->Realloc(Address, Size);
        memset(ret, 0, Size);
        return ret;
    }
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
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("free(%#lx)->[%s]", Address, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    if (unlikely(!Address))
    {
        warn("Attempt to free a null pointer");
        return;
    }

    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
        KernelAllocator.FreePage(Address); // WARNING: Potential memory leak
        break;
    case MemoryAllocatorType::XallocV1:
        if (XallocV1Allocator)
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

void *operator new(size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new(%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    return HeapMalloc(Size);
}

void *operator new[](size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new[](%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    return HeapMalloc(Size);
}

void *operator new(unsigned long Size, std::align_val_t Alignment)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new(%d, %d)->[%s]", Size, Alignment, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    fixme("operator new with alignment(%#lx) is not implemented", Alignment);
    return HeapMalloc(Size);
}

void operator delete(void *Pointer)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete(%#lx)->[%s]", Pointer, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    HeapFree(Pointer);
}

void operator delete[](void *Pointer)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete[](%#lx)->[%s]", Pointer, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    HeapFree(Pointer);
}

void operator delete(void *Pointer, long unsigned int Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete(%#lx, %d)->[%s]", Pointer, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    HeapFree(Pointer);
    UNUSED(Size);
}

void operator delete[](void *Pointer, long unsigned int Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete[](%#lx, %d)->[%s]", Pointer, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    HeapFree(Pointer);
    UNUSED(Size);
}
