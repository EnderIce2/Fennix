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
NetworkInterfaceManager::NetworkInterface *NIManager = nullptr;
Recovery::KernelRecovery *RecoveryScreen = nullptr;

void KernelMainThread()
{
    TaskManager->InitIPC();
    TaskManager->GetCurrentThread()->SetPriority(100);
    CPU::Interrupts(CPU::Disable);

    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);

    KPrint("Initializing Filesystem...");
    vfs = new FileSystem::Virtual;
    new FileSystem::USTAR((uintptr_t)bInfo->Modules[0].Address, vfs); // TODO: Detect initrd
    KPrint("Initializing Disk Manager...");
    DiskManager = new Disk::Manager;

    KPrint("Loading Drivers...");
    DriverManager = new Driver::Driver;

    KPrint("Fetching Disks...");
    if (DriverManager->GetDrivers().size() > 0)
    {
        foreach (auto Driver in DriverManager->GetDrivers())
            if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Storage)
                DiskManager->FetchDisks(Driver->DriverUID);
    }
    else
        KPrint("\eE85230No disk drivers found! Cannot fetch disks!");

    KPrint("Initializing Network Interface Manager...");
    NIManager = new NetworkInterfaceManager::NetworkInterface;
    KPrint("Starting Network Interface Manager...");
    // NIManager->StartService();

    KPrint("Setting up userspace...");

    const char *envp[9] = {
        "PATH=/system:/system/bin",
        "TERM=tty",
        "HOME=/",
        "USER=root",
        "SHELL=/system/sh",
        "PWD=/",
        "LANG=en_US.UTF-8",
        "TZ=UTC",
        nullptr};

    const char *argv[4] = {
        Config.InitPath,
        "--init",
        "--critical",
        nullptr};

    Execute::SpawnData ret = Execute::Spawn(Config.InitPath, argv, envp);
    if (ret.Status != Execute::ExStatus::OK)
    {
        KPrint("\eE85230Failed to start %s! Code: %d", Config.InitPath, ret.Status);
        CPU::Interrupts(CPU::Enable);
        goto Exit;
    }
    ret.Thread->SetCritical(true);
    KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...", Config.InitPath);
    CPU::Interrupts(CPU::Enable);
    TaskManager->GetCurrentThread()->SetPriority(1);
    TaskManager->WaitForThread(ret.Thread);
    KPrint("\eE85230Userspace process exited with code %d", ret.Thread->GetExitCode());
    error("Userspace process exited with code %d (%#x)", ret.Thread->GetExitCode(), ret.Thread->GetExitCode());
Exit:
    KPrint("%s exited with code %d! Dropping to recovery screen...", Config.InitPath, ret.Thread->GetExitCode());
    TaskManager->Sleep(1000);
    RecoveryScreen = new Recovery::KernelRecovery;
    CPU::Halt(true);
}

void KernelShutdownThread(bool Reboot)
{
    delete NIManager;

    if (DriverManager)
        DriverManager->UnloadAllDrivers();

    trace("Shutting Down/Rebooting...");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();
    CPU::Stop();
}
