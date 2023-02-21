#ifndef __FENNIX_KERNEL_CPU_MEMBAR_H__
#define __FENNIX_KERNEL_CPU_MEMBAR_H__

#include <types.h>

namespace CPU
{
    namespace MemBar
    {
        SafeFunction static inline void Barrier()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void Fence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("mfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void StoreFence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("sfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ishst" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void LoadFence()
        {
#if defined(__amd64__) || defined(__i386__)
            asmv("lfence" ::
                     : "memory");
#elif defined(__aarch64__)
            asmv("dmb ishld" ::
                     : "memory");
#endif
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_MEMBAR_H__
