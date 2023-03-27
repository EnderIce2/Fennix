#include <memory.hpp>

#include <convert.h>
#include <lock.hpp>
#include <debug.h>
#ifdef DEBUG
#include <uart.hpp>
#endif

#include "HeapAllocators/Xalloc/Xalloc.hpp"
#include "../Library/liballoc_1_1.h"
#include "../../kernel.h"

// #define DEBUG_ALLOCATIONS_SL 1
// #define DEBUG_ALLOCATIONS 1

#ifdef DEBUG_ALLOCATIONS
#define memdbg(m, ...)       \
    debug(m, ##__VA_ARGS__); \
    __sync
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

static MemoryAllocatorType AllocatorType = MemoryAllocatorType::Pages;
Xalloc::V1 *XallocV1Allocator = nullptr;

#ifdef DEBUG
NIF void tracepagetable(PageTable4 *pt)
{
    for (int i = 0; i < 512; i++)
    {
#if defined(a64)
        if (pt->Entries[i].Present)
            debug("Entry %03d: %x %x %x %x %x %x %x %p-%#llx", i,
                  pt->Entries[i].Present, pt->Entries[i].ReadWrite,
                  pt->Entries[i].UserSupervisor, pt->Entries[i].WriteThrough,
                  pt->Entries[i].CacheDisable, pt->Entries[i].Accessed,
                  pt->Entries[i].ExecuteDisable, pt->Entries[i].Address << 12,
                  pt->Entries[i]);
#elif defined(a32)
#elif defined(aa64)
#endif
    }
}
#endif

NIF void MapFromZero(PageTable4 *PT, BootInfo *Info)
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

NIF void MapFramebuffer(PageTable4 *PT, BootInfo *Info)
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

#ifdef DEBUG
        if (EnableExternalMemoryTracer)
        {
            char LockTmpStr[64];
            strcpy_unsafe(LockTmpStr, __FUNCTION__);
            strcat_unsafe(LockTmpStr, "_memTrk");
            mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
            sprintf(mExtTrkLog, "Rsrv( %p %ld )\n\r",
                    Info->Framebuffer[itrfb].BaseAddress,
                    (Info->Framebuffer[itrfb].Pitch * Info->Framebuffer[itrfb].Height) + PAGE_SIZE);
            UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
            for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
            {
                if (mExtTrkLog[i] == '\r')
                    break;
                mTrkUART.Write(mExtTrkLog[i]);
            }
            mExtTrkLock.Unlock();
        }
#endif
    }
}

NIF void MapKernel(PageTable4 *PT, BootInfo *Info)
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
    uintptr_t KernelFileStart = (uintptr_t)Info->Kernel.FileBase;
    uintptr_t KernelFileEnd = KernelFileStart + Info->Kernel.Size;

    uintptr_t BaseKernelMapAddress = (uintptr_t)Info->Kernel.PhysicalBase;
    uintptr_t k;

    /* Text section */
    for (k = KernelStart; k < KernelTextEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    /* Data section */
    for (k = KernelTextEnd; k < KernelDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW | PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    /* Read only data section */
    for (k = KernelDataEnd; k < KernelRoDataEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    /* BSS section */
    for (k = KernelRoDataEnd; k < KernelEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW | PTFlag::G);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    /* Kernel file */
    for (k = KernelFileStart; k < KernelFileEnd; k += PAGE_SIZE)
    {
        va.Map((void *)k, (void *)k, PTFlag::G);
        KernelAllocator.LockPage((void *)k);
    }

    debug("\nStart: %#llx - Text End: %#llx - RoEnd: %#llx - End: %#llx\nStart Physical: %#llx - End Physical: %#llx",
          KernelStart, KernelTextEnd, KernelRoDataEnd, KernelEnd, KernelFileStart, KernelFileEnd);

#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "Rsrv( %p %ld )\n\r",
                Info->Kernel.PhysicalBase,
                Info->Kernel.Size);
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }

        sprintf(mExtTrkLog, "Rsrv( %p %ld )\n\r",
                Info->Kernel.VirtualBase,
                Info->Kernel.Size);
        mExtTrkLock.Unlock();
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
    }
#endif
}

NIF void InitializeMemoryManagement(BootInfo *Info)
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
#if defined(a64) || defined(a32)
    asmv("mov %0, %%cr3" ::"r"(KernelPageTable));
#elif defined(aa64)
    asmv("msr ttbr0_el1, %0" ::"r"(KernelPageTable));
#endif
    debug("Page table updated.");
    if (strstr(Info->Kernel.CommandLine, "xallocv1"))
    {
        XallocV1Allocator = new Xalloc::V1((void *)KERNEL_HEAP_BASE, false, false);
        AllocatorType = MemoryAllocatorType::XallocV1;
        trace("XallocV1 Allocator initialized (%p)", XallocV1Allocator);
    }
    else if (strstr(Info->Kernel.CommandLine, "liballoc11"))
    {
        AllocatorType = MemoryAllocatorType::liballoc11;
    }
}

void *malloc(size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("malloc(%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    void *ret = nullptr;
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
    {
        ret = KernelAllocator.RequestPages(TO_PAGES(Size));
        memset(ret, 0, Size);
        break;
    }
    case MemoryAllocatorType::XallocV1:
    {
        ret = XallocV1Allocator->malloc(Size);
        break;
    }
    case MemoryAllocatorType::liballoc11:
    {
        ret = PREFIX(malloc)(Size);
        memset(ret, 0, Size);
        break;
    }
    default:
        throw;
    }
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "malloc( %ld )=%p~%p\n\r",
                Size,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void *calloc(size_t n, size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("calloc(%d, %d)->[%s]", n, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    void *ret = nullptr;
    switch (AllocatorType)
    {
    case MemoryAllocatorType::Pages:
    {
        ret = KernelAllocator.RequestPages(TO_PAGES(n * Size));
        memset(ret, 0, n * Size);
        break;
    }
    case MemoryAllocatorType::XallocV1:
    {
        ret = XallocV1Allocator->calloc(n, Size);
        break;
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
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "calloc( %ld %ld )=%p~%p\n\r",
                n, Size,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void *realloc(void *Address, size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("realloc(%#lx, %d)->[%s]", Address, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    void *ret = nullptr;
    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
    {
        ret = KernelAllocator.RequestPages(TO_PAGES(Size)); // WARNING: Potential memory leak
        memset(ret, 0, Size);
        break;
    }
    case MemoryAllocatorType::XallocV1:
    {
        ret = XallocV1Allocator->realloc(Address, Size);
        break;
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
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "realloc( %p %ld )=%p~%p\n\r",
                Address, Size,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void free(void *Address)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(AllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("free(%#lx)->[%s]", Address, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    switch (AllocatorType)
    {
    case unlikely(MemoryAllocatorType::Pages):
    {
        KernelAllocator.FreePage(Address); // WARNING: Potential memory leak
        break;
    }
    case MemoryAllocatorType::XallocV1:
    {
        XallocV1Allocator->free(Address);
        break;
    }
    case MemoryAllocatorType::liballoc11:
    {
        PREFIX(free)
        (Address);
        break;
    }
    default:
        throw;
    }
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "free( %p )~%p\n\r",
                Address,
                __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
}

void *operator new(size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new(%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    void *ret = malloc(Size);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "new( %ld )=%p~%p\n\r",
                Size,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void *operator new[](size_t Size)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new[](%d)->[%s]", Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    void *ret = malloc(Size);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "new[]( %ld )=%p~%p\n\r",
                Size,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void *operator new(unsigned long Size, std::align_val_t Alignment)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("new(%d, %d)->[%s]", Size, Alignment, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");
    fixme("operator new with alignment(%#lx) is not implemented", Alignment);

    void *ret = malloc(Size);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "new( %ld %#lx )=%p~%p\n\r",
                Size, (uintptr_t)Alignment,
                ret, __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
    return ret;
}

void operator delete(void *Pointer)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete(%#lx)->[%s]", Pointer, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    free(Pointer);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "delete( %p )~%p\n\r",
                Pointer,
                __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
}

void operator delete[](void *Pointer)
{
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete[](%#lx)->[%s]", Pointer, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    free(Pointer);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "delete[]( %p )~%p\n\r",
                Pointer,
                __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
}

void operator delete(void *Pointer, long unsigned int Size)
{
    UNUSED(Size);
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete(%#lx, %d)->[%s]", Pointer, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    free(Pointer);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "delete( %p %ld )~%p\n\r",
                Pointer, Size,
                __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
}

void operator delete[](void *Pointer, long unsigned int Size)
{
    UNUSED(Size);
#ifdef DEBUG_ALLOCATIONS_SL
    SmartLockClass lock___COUNTER__(OperatorAllocatorLock, (KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown"));
#endif
    memdbg("delete[](%#lx, %d)->[%s]", Pointer, Size, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_return_address(0)) : "Unknown");

    free(Pointer);
#ifdef DEBUG
    if (EnableExternalMemoryTracer)
    {
        char LockTmpStr[64];
        strcpy_unsafe(LockTmpStr, __FUNCTION__);
        strcat_unsafe(LockTmpStr, "_memTrk");
        mExtTrkLock.TimeoutLock(LockTmpStr, 10000);
        sprintf(mExtTrkLog, "delete[]( %p %ld )~%p\n\r",
                Pointer, Size,
                __builtin_return_address(0));
        UniversalAsynchronousReceiverTransmitter::UART mTrkUART = UniversalAsynchronousReceiverTransmitter::UART(UniversalAsynchronousReceiverTransmitter::COM3);
        for (short i = 0; i < MEM_TRK_MAX_SIZE; i++)
        {
            if (mExtTrkLog[i] == '\r')
                break;
            mTrkUART.Write(mExtTrkLog[i]);
        }
        mExtTrkLock.Unlock();
    }
#endif
}
