#include <types.h>
#include "../lock.hpp"

#include "../../../../Kernel/syscalls.h"

NewLock(liballocLock);

extern "C" int liballoc_lock()
{
	return liballocLock.Lock(__FUNCTION__);
}

extern "C" int liballoc_unlock()
{
	return liballocLock.Unlock();
}

extern "C" void *liballoc_alloc(size_t Pages)
{
	return (void *)syscall6(sc_mmap, NULL,
							Pages * 0x1000,
							sc_PROT_READ | sc_PROT_WRITE,
							sc_MAP_ANONYMOUS | sc_MAP_PRIVATE,
							-1, 0);
}

extern "C" int liballoc_free(void *Address, size_t Pages)
{
	return syscall2(sc_munmap, (uintptr_t)Address, Pages * 0x1000);
}
