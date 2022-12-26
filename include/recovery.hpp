#ifndef __FENNIX_KERNEL_RECOVERY_H__
#define __FENNIX_KERNEL_RECOVERY_H__

#include <types.h>
#include <memory.hpp>

namespace Recovery
{
    class KernelRecovery
    {
    private:
        Memory::MemMgr *mem;

    public:
        void RecoveryThread();
        KernelRecovery();
        ~KernelRecovery();
    };
}

#endif // !__FENNIX_KERNEL_RECOVERY_H__
