#include "Xalloc.hpp"

#include <memory.hpp>

extern "C" void *Xalloc_REQUEST_PAGES(Xsize_t Pages)
{
    return KernelAllocator.RequestPages(Pages);
}

extern "C" void Xalloc_FREE_PAGES(void *Address, Xsize_t Pages)
{
    KernelAllocator.FreePages(Address, Pages);
}

extern "C" void Xalloc_MAP_MEMORY(void *VirtualAddress, void *PhysicalAddress, Xsize_t Flags)
{
    Memory::Virtual(KernelPageTable).Map(VirtualAddress, PhysicalAddress, Flags);
}

extern "C" void Xalloc_UNMAP_MEMORY(void *VirtualAddress)
{
    Memory::Virtual(KernelPageTable).Unmap(VirtualAddress);
}
