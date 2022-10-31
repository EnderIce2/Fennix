#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>

Driver::Driver *DriverManager = nullptr;

void StartFilesystem()
{
    KPrint("Initializing Filesystem...");
    vfs = new FileSystem::Virtual;
    new FileSystem::USTAR((uint64_t)bInfo->Modules[0].Address, vfs); // TODO: Detect initrd
    /* ... */
    TEXIT(0);
}

void LoadDrivers()
{
    KPrint("Loading Drivers...");
    DriverManager = new Driver::Driver;
    TEXIT(0);
}

void KernelMainThread()
{
    Tasking::TCB *CurrentWorker = nullptr;
    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus) :%ld", __cplusplus);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)StartFilesystem);
    CurrentWorker->Rename("Filesystem");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)LoadDrivers);
    CurrentWorker->Rename("Drivers");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);
    KPrint("Waiting for userspace process to start...");
    /* Load init file */
    CPU::Halt(true);
}

void KernelShutdownThread(bool Reboot)
{
    debug("Shutting down...");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();

    CPU::Stop();
}
