#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>

void StartFilesystem()
{
    KPrint("Initializing Filesystem...");
    TaskManager->GetCurrentThread()->SetPriority(100);
    vfs = new FileSystem::Virtual;
    new FileSystem::USTAR((uint64_t)bInfo->Modules[0].Address, vfs); // TODO: Detect initrd
    TaskManager->GetCurrentThread()->SetPriority(1);
    debug("Filesystem service idling...");
    CPU::Halt(true);
}

void KernelMainThread()
{
    KPrint("Kernel main thread started!");

    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus) :%ld", __cplusplus);

    CPU::Interrupts(CPU::Disable);

    TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)StartFilesystem)->Rename("Filesystem Service");

    CPU::Interrupts(CPU::Enable);
    TaskManager->GetCurrentThread()->SetPriority(1);
    CPU::Halt(true);
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
