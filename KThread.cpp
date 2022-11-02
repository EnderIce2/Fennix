#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>

#include "DAPI.hpp"
#include "Fex.hpp"

Driver::Driver *DriverManager = nullptr;
Disk::Manager *DiskManager = nullptr;

void StartFilesystem()
{
    KPrint("Initializing Filesystem...");
    vfs = new FileSystem::Virtual;
    new FileSystem::USTAR((uint64_t)bInfo->Modules[0].Address, vfs); // TODO: Detect initrd
    KPrint("Initializing Disk Manager...");
    DiskManager = new Disk::Manager;
    /* ... */
    TEXIT(0);
}

void LoadDrivers()
{
    KPrint("Loading Drivers...");
    DriverManager = new Driver::Driver;
    TEXIT(0);
}

void FetchDisks()
{
    KPrint("Fetching Disks...");
    foreach (auto Driver in DriverManager->GetDrivers())
    {
        FexExtended *DrvExtHdr = (FexExtended *)((uint64_t)Driver->Address + EXTENDED_SECTION_ADDRESS);

        if (DrvExtHdr->Driver.Type == FexDriverType::FexDriverType_Storage)
            DiskManager->FetchDisks(Driver->DriverUID);
    }
    TEXIT(0);
}

void KernelMainThread()
{
    Tasking::TCB *CurrentWorker = nullptr;
    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus) :%ld", __cplusplus);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)StartFilesystem);
    CurrentWorker->Rename("Disk");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)LoadDrivers);
    CurrentWorker->Rename("Drivers");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)FetchDisks);
    CurrentWorker->Rename("Fetch Disks");
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
