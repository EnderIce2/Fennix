#ifndef __FENNIX_KERNEL_CPU_MEMBAR_H__
#define __FENNIX_KERNEL_CPU_MEMBAR_H__

#include <types.h>

namespace CPU
{
    namespace MemBar
    {
        SafeFunction static inline void Barrier()
        {
#if defined(a64) || defined(a32)
            asmv("" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void Fence()
        {
#if defined(a64) || defined(a32)
            asmv("mfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ish" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void StoreFence()
        {
#if defined(a64) || defined(a32)
            asmv("sfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ishst" ::
                     : "memory");
#endif
        }

        SafeFunction static inline void LoadFence()
        {
#if defined(a64) || defined(a32)
            asmv("lfence" ::
                     : "memory");
#elif defined(aa64)
            asmv("dmb ishld" ::
                     : "memory");
#endif
        }
    }
}

#endif // !__FENNIX_KERNEL_CPU_MEMBAR_H__
