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
        TaskManager->Sleep(100);
        TreeFS(Chld, Depth + 1);
    }
}

const char *Statuses[] = {
    "FF0000", /* Unknown */
    "AAFF00", /* Ready */
    "00AA00", /* Running */
    "FFAA00", /* Sleeping */
    "FFAA00", /* Waiting */
    "FF0088", /* Stopped */
    "FF0000", /* Terminated */
};

const char *StatusesSign[] = {
    "Unknown",
    "Ready",
    "Run",
    "Sleep",
    "Wait",
    "Stop",
    "Terminated",
};

const char *SuccessSourceStrings[] = {
    "Unknown",
    "GetNextAvailableThread",
    "GetNextAvailableProcess",
    "SchedulerSearchProcessThread",
};

void TaskMgr()
{
    while (1)
    {
        CPU::Interrupts(CPU::Disable);
        static int sanity = 0;
        Video::ScreenBuffer *sb = Display->GetBuffer(0);
        for (short i = 0; i < 340; i++)
        {
            for (short j = 0; j < 200; j++)
            {
                uint32_t *Pixel = (uint32_t *)((uintptr_t)sb->Buffer + (j * sb->Width + i) * (bInfo->Framebuffer[0].BitsPerPixel / 8));
                *Pixel = 0x222222;
            }
        }

        uint32_t tmpX, tmpY;
        Display->GetBufferCursor(0, &tmpX, &tmpY);
        Display->SetBufferCursor(0, 0, 0);
        printf("\eF02C21Task Manager\n");
        foreach (auto Proc in TaskManager->GetProcessList())
        {
            int Status = Proc->Status;
            printf("\e%s-> \eAABBCC%s \e00AAAA%s\n",
                   Statuses[Status], Proc->Name, StatusesSign[Status]);

            foreach (auto Thd in Proc->Threads)
            {
                Status = Thd->Status;
                printf("  \e%s-> \eAABBCC%s \e00AAAA%s\n\eAABBCC",
                       Statuses[Status], Thd->Name, StatusesSign[Status]);
            }
        }
        register uintptr_t CurrentStackAddress asm("rsp");
        printf("Sanity: %d, Stack: %#lx", sanity++, CurrentStackAddress);
        if (sanity > 1000)
            sanity = 0;
        Display->SetBufferCursor(0, tmpX, tmpY);
        Display->SetBuffer(0);
        CPU::Interrupts(CPU::Enable);
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

#ifdef DEBUG
    /* TODO: This should not be enabled because it may cause a deadlock. Not sure where or how. */
    // Tasking::PCB *tskMgr = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), "Debug Task Manager", Tasking::TaskTrustLevel::Kernel);
    // TaskManager->CreateThread(tskMgr, (Tasking::IP)TaskMgr)->SetPriority(Tasking::High);

    TreeFS(vfs->GetRootNode(), 0);
#endif

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

    Time::Clock tm = Time::ReadClock();
    printf("\eCCCCCC[\e00AEFF%02d:%02d:%02d\eCCCCCC] ", tm.Hour, tm.Minute, tm.Second);
    const char *USpace_msg = "Setting up userspace";
    for (size_t i = 0; i < strlen(USpace_msg); i++)
        Display->Print(USpace_msg[i], 0);
    Display->SetBuffer(0);

    Execute::SpawnData ret = {Execute::ExStatus::Unknown, nullptr, nullptr};
    Tasking::TCB *ExecuteThread = nullptr;
    int ExitCode = -1;
    ExecuteThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)Execute::StartExecuteService);
    ExecuteThread->Rename("Library Manager");
    ExecuteThread->SetCritical(true);
    ExecuteThread->SetPriority(Tasking::Idle);

    Display->Print('.', 0);
    Display->SetBuffer(0);

    ret = SpawnInit();

    Display->Print('.', 0);
    Display->SetBuffer(0);

    if (ret.Status != Execute::ExStatus::OK)
    {
        KPrint("\eE85230Failed to start %s! Code: %d", Config.InitPath, ret.Status);
        goto Exit;
    }
    TaskManager->GetSecurityManager()->TrustToken(ret.Process->Security.UniqueToken, Tasking::TTL::FullTrust);
    TaskManager->GetSecurityManager()->TrustToken(ret.Thread->Security.UniqueToken, Tasking::TTL::FullTrust);
    ret.Thread->SetCritical(true);

    Display->Print('.', 0);
    Display->Print('\n', 0);
    Display->SetBuffer(0);

    KPrint("Waiting for \e22AAFF%s\eCCCCCC to start...", Config.InitPath);
    TaskManager->GetCurrentThread()->SetPriority(Tasking::Idle);

    TaskManager->WaitForThread(ret.Thread);
    ExitCode = ret.Thread->GetExitCode();
Exit:
    if (ExitCode != 0)
    {
        KPrint("\eE85230Userspace process exited with code %d", ExitCode);
        KPrint("Dropping to recovery screen...");
        TaskManager->Sleep(2500);
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
