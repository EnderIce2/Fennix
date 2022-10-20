#include "kernel.h"

void KernelMainThread()
{
    KPrint("Kernel main thread started!");
    // asm("int $0x1");
    CPU::Stop();
}
