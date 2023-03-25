#ifndef __FENNIX_KERNEL_RECOVERY_H__
#define __FENNIX_KERNEL_RECOVERY_H__

#include <types.h>
#include <memory.hpp>
#include <task.hpp>

namespace Recovery
{
    class KernelRecovery
    {
    private:
        Memory::MemMgr *mem;
        Tasking::TCB *guiThread;
        Tasking::TCB *recoveryThread;

    public:
        void RecoveryThread();
        KernelRecovery();
        ~KernelRecovery();
    };
}

#endif // !__FENNIX_KERNEL_RECOVERY_H__
