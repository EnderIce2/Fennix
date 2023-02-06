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

using VirtualFileSystem::File;
using VirtualFileSystem::FileStatus;
using VirtualFileSystem::Node;
using VirtualFileSystem::NodeFlags;

Driver::Driver *DriverManager = nullptr;
Disk::Manager *DiskManager = nullptr;
NetworkInterfaceManager::NetworkInterface *NIManager = nullptr;
Recovery::KernelRecovery *RecoveryScreen = nullptr;
VirtualFileSystem::Node *DevFS = nullptr;
VirtualFileSystem::Node *MntFS = nullptr;
VirtualFileSystem::Node *ProcFS = nullptr;

#ifdef DEBUG
void TreeFS(Node *node, int Depth)
{
    return;
    foreach (auto Chld in node->Children)
    {
        printf("%*c %s\eFFFFFF\n", Depth, ' ', Chld->Name);
        Display->SetBuffer(0);
        TreeFS(Chld, Depth + 1);
    }
}
#endif

Execute::SpawnData SpawnInit()
{
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

    return Execute::Spawn(Config.InitPath, argv, envp);
}

void KernelMainThread()
{
    TaskManager->GetCurrentThread()->SetPriority(Tasking::Critical);

    KPrint("Kernel Compiled at: %s %s with C++ Standard: %d", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
    KPrint("C++ Language Version (__cplusplus): %ld", __cplusplus);

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
    NIManager->StartService();

    KPrint("Setting up userspace");

#ifdef DEBUG
    TreeFS(vfs->GetRootNode(), 0);
#endif

    const char *USpace_msg = "Setting up userspace";
    for (size_t i = 0; i < strlen(USpace_msg); i++)
        Display->Print(USpace_msg[i], 0);

    Display->SetBuffer(0);

    Execute::SpawnData ret = {Execute::ExStatus::Unknown, nullptr, nullptr};
    Tasking::TCB *ExecuteThread = nullptr;
    int ExitCode = -1;

    Display->Print('.', 0);
    Display->SetBuffer(0);

    ExecuteThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)Execute::StartExecuteService);
    ExecuteThread->Rename("Library Manager");
    ExecuteThread->SetCritical(true);
    ExecuteThread->SetPriority(Tasking::Idle);

    Display->Print('.', 0);
    Display->SetBuffer(0);

    CPU::Interrupts(CPU::Disable);
    ret = SpawnInit();

    Display->Print('.', 0);
    Display->Print('\n', 0);
    Display->SetBuffer(0);

    if (ret.Status != Execute::ExStatus::OK)
    {
        KPrint("\eE85230Failed to start %s! Code: %d", Config.InitPath, ret.Status);
        goto Exit;
    }
    TaskManager->GetSecurityManager()->TrustToken(ret.Process->Security.UniqueToken, Tasking::TTL::FullTrust);
    TaskManager->GetSecurityManager()->TrustToken(ret.Thread->Security.UniqueToken, Tasking::TTL::FullTrust);
    ret.Thread->SetCritical(true);
    KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...", Config.InitPath);
    CPU::Interrupts(CPU::Enable);
    TaskManager->GetCurrentThread()->SetPriority(Tasking::Idle);
    TaskManager->WaitForThread(ret.Thread);
    ExitCode = ret.Thread->GetExitCode();
    if (ExitCode != 0)
        KPrint("\eE85230Userspace process exited with code %d", ExitCode);
    error("Userspace process exited with code %d (%#x)", ExitCode, ExitCode);
Exit:
    if (ExitCode != 0)
    {
        KPrint("Dropping to recovery screen...", ExitCode);
        TaskManager->Sleep(5000);
        RecoveryScreen = new Recovery::KernelRecovery;
    }
    else
    {
        KPrint("\eFF7900%s process exited with code %d and it didn't invoked the shutdown function.",
               Config.InitPath, ExitCode);
        KPrint("System Halted");
    }
    CPU::Halt(true);
}

void KernelShutdownThread(bool Reboot)
{
    BeforeShutdown();

    trace("%s...", Reboot ? "Rebooting" : "Shutting down");
    if (Reboot)
        PowerManager->Reboot();
    else
        PowerManager->Shutdown();
    CPU::Stop();
}
