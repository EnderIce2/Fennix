#include "memory.hpp"

#include "liballoc_1_1.h"

extern "C" void printf(const char *format, ...);

extern uint64_t ImageBase, _text, _etext, _data, _edata, _data_size;

using namespace Memory;

Physical KernelAllocator;
PageTable *KernelPageTable = nullptr;

static void *memset(void *s, int c, size_t n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
        ((char *)s)[i] = c;

    return s;
}

extern "C" void InitializeMemoryManagement(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    printf("Initializing Physical Memory Manager\n");
    KernelAllocator = Physical();
    KernelAllocator.Init(ImageHandle, SystemTable);
    printf("Memory Info: %dMB / %dMB (%dMB reserved)",
           (KernelAllocator.GetUsedMemory() / 1024 / 1024),
           (KernelAllocator.GetTotalMemory() / 1024 / 1024),
           (KernelAllocator.GetReservedMemory() / 1024 / 1024));

    KernelPageTable = (PageTable *)KernelAllocator.RequestPage();
    memset(KernelPageTable, 0, PAGE_SIZE);
    Virtual kva = Virtual(KernelPageTable);
    printf("Mapping...\n");

    uint64_t BootloaderStart = (uint64_t)&ImageBase;
    uint64_t BootloaderTextEnd = (uint64_t)&_text;
    uint64_t BootloaderDataEnd = (uint64_t)&_data;
    uint64_t BootloaderEnd = (uint64_t)&ImageBase + (uint64_t)&_etext + (uint64_t)&_edata;

    uint64_t VirtualOffsetNormalVMA = NORMAL_VMA_OFFSET;
    uint64_t BaseKernelMapAddress = (uint64_t)0; // TODO: Info->Kernel.PhysicalBase;

    EFI_MEMORY_DESCRIPTOR *memDesc = nullptr;
    UINTN MapSize, MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    {
        SystemTable->BootServices->GetMemoryMap(&MapSize, memDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
        SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void **)&memDesc);
        SystemTable->BootServices->GetMemoryMap(&MapSize, memDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
    }

    for (uint64_t t = 0; t < MapSize / DescriptorSize; t += PAGE_SIZE)
    {
        kva.Map((void *)t, (void *)t, PTFlag::RW);
        kva.Map((void *)VirtualOffsetNormalVMA, (void *)t, PTFlag::RW);
        VirtualOffsetNormalVMA += PAGE_SIZE;
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes = 0; //, MaximumSupportedMode = 0;
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
    if (EFI_ERROR(status))
    {
        printf("Unable to locate the Graphics Output Protocol.\n");
    }
    status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED)
    {
        printf("The EFI not started!\n");
        status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
    }

    /* Mapping Framebuffer address */
    int itrfb = 0;
    while (1)
    {
        for (uint64_t fb_base = (uint64_t)gop->Mode->FrameBufferBase;
             fb_base < ((uint64_t)gop->Mode->FrameBufferBase + ((gop->Mode->Info->PixelsPerScanLine) + PAGE_SIZE));
             fb_base += PAGE_SIZE)
            kva.Map((void *)fb_base, (void *)fb_base, PTFlag::RW | PTFlag::US);
        itrfb++;
    }

    /* Kernel mapping */
    for (uint64_t k = BootloaderStart; k < BootloaderTextEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = BootloaderTextEnd; k < BootloaderDataEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    for (uint64_t k = BootloaderDataEnd; k < BootloaderEnd; k += PAGE_SIZE)
    {
        kva.Map((void *)k, (void *)BaseKernelMapAddress, PTFlag::RW);
        KernelAllocator.LockPage((void *)BaseKernelMapAddress);
        BaseKernelMapAddress += PAGE_SIZE;
    }

    printf("\nStart: %#llx - Text End: %#llx - End: %#llx\nStart Physical: %#llx - End Physical: %#llx",
           BootloaderStart, BootloaderTextEnd, BootloaderEnd, /* Info->Kernel.PhysicalBase */ 0, BaseKernelMapAddress - PAGE_SIZE);

    /*    BootloaderStart             BootloaderTextEnd       KernelRoDataEnd                  BootloaderEnd
    Kernel Start & Text Start ------ Text End ------ Kernel Rodata End ------ Kernel Data End & Kernel End
    */
    printf("Applying new page table from address %p", KernelPageTable);
    __asm__ volatile("mov %0, %%cr3" ::"r"(KernelPageTable));
}

extern "C" void *HeapMalloc(uint64_t Size) { return PREFIX(malloc)(Size); }
extern "C" void *HeapCalloc(uint64_t n, uint64_t Size) { return PREFIX(calloc)(n, Size); }
extern "C" void *HeapRealloc(void *Address, uint64_t Size) { return PREFIX(realloc)(Address, Size); }
extern "C" void HeapFree(void *Address)
{
    PREFIX(free)
    (Address);
}

void *operator new(uint64_t Size) { return HeapMalloc(Size); }
void *operator new[](uint64_t Size) { return HeapMalloc(Size); }
void operator delete(void *Pointer) { HeapFree(Pointer); }
void operator delete[](void *Pointer) { HeapFree(Pointer); }
void operator delete(void *Pointer, long unsigned int Size) { HeapFree(Pointer); }
void operator delete[](void *Pointer, long unsigned int Size) { HeapFree(Pointer); }

EXTERNC int liballoc_lock() {}
EXTERNC int liballoc_unlock() {}
EXTERNC void *liballoc_alloc(size_t Pages) { return KernelAllocator.RequestPages(Pages); }
EXTERNC int liballoc_free(void *Address, size_t Pages)
{
    KernelAllocator.FreePages(Address, Pages);
    return 0;
}
