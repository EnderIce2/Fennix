#include <types.h>
#include <lock.hpp>
#include <memory.hpp>

NewLock(liballocLock);

EXTERNC int liballoc_lock()
{
    return liballocLock.Lock(__FUNCTION__);
}

EXTERNC int liballoc_unlock()
{
    return liballocLock.Unlock();
}

EXTERNC void *liballoc_alloc(size_t Pages)
{
    void *ret = KernelAllocator.RequestPages(Pages);
    debug("(%d) = %#lx", Pages, ret);
    return ret;
}

EXTERNC int liballoc_free(void *Address, size_t Pages)
{
    debug("(%#lx, %d)", Address, Pages);
    KernelAllocator.FreePages(Address, Pages);
    return 0;
}
