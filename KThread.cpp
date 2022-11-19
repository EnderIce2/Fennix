#include "kernel.h"

#include <filesystem/ustar.hpp>
#include <vector.hpp>
#include <power.hpp>
#include <lock.hpp>
#include <printf.h>
#include <exec.hpp>
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
    TaskManager->InitIPC();
    Vector<AuxiliaryVector> auxv;

    Tasking::TCB *CurrentWorker = nullptr;
    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);
    TaskManager->GetCurrentThread()->SetPriority(1);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)StartFilesystem, nullptr, nullptr, auxv);
    CurrentWorker->Rename("Filesystems");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)LoadDrivers, nullptr, nullptr, auxv);
    CurrentWorker->Rename("Drivers");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    CurrentWorker = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)FetchDisks, nullptr, nullptr, auxv);
    CurrentWorker->Rename("Disks");
    CurrentWorker->SetPriority(100);
    TaskManager->WaitForThread(CurrentWorker);

    KPrint("Setting up userspace...");

    const char *envp[] = {
        "PATH=/system:/system/bin",
        "TERM=tty",
        "HOME=/",
        "USER=root",
        "SHELL=/system/bin/sh",
        "PWD=/",
        "LANG=en_US.UTF-8",
        "TZ=UTC",
        nullptr};

    const char *argv[] = {
        "--init",
        "--critical",
        nullptr};

    // TODO: Untested!
    bool ien = CPU::Interrupts(CPU::Check);
    CPU::Interrupts(CPU::Disable);
    Execute::SpawnData ret = Execute::Spawn(Config.InitPath, argv, envp);
    if (ret.Status != Execute::ExStatus::OK)
    {
        KPrint("\eE85230Failed to start %s! Code: %d", Config.InitPath, ret.Status);
        if (ien)
            CPU::Interrupts(CPU::Enable);
        goto Exit;
    }
    ret.Thread->SetCritical(true);
    if (ien)
        CPU::Interrupts(CPU::Enable);
    KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...", Config.InitPath);
    TaskManager->WaitForThread(ret.Thread);
    KPrint("\eE85230Userspace process exited with code %d", ret.Thread->GetExitCode());
    error("Userspace process exited with code %d (%#x)", ret.Thread->GetExitCode(), ret.Thread->GetExitCode());
Exit:
    KPrint("Well, this is awkward. I guess you'll have to reboot.");
    CPU::Halt(true);
}

void KernelShutdownThread(bool Reboot)
{
    trace("Shutting Down/Rebooting...");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();
    CPU::Stop();
}
