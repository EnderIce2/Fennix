#include "kernel.h"

#include <power.hpp>

void KernelMainThread()
{
    KPrint("Kernel main thread started!");
    // asm("int $0x1");
    CPU::Stop();
}

void KernelShutdownThread(bool Reboot)
{
    KPrint("Kernel shutdown thread started!");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();

    CPU::Stop();
}
