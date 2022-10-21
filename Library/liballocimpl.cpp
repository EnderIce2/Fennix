#include <types.h>
#include <lock.hpp>
#include <memory.hpp>

NewLock(liballocLock);

EXTERNC int liballoc_lock() { return liballocLock.Lock(); }
EXTERNC int liballoc_unlock() { return liballocLock.Unlock(); }
EXTERNC void *liballoc_alloc(size_t Pages) { return KernelAllocator.RequestPages(Pages); }
EXTERNC int liballoc_free(void *Address, size_t Pages)
{
    KernelAllocator.FreePages(Address, Pages);
    return 0;
}
