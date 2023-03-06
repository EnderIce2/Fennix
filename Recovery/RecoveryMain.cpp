#include <recovery.hpp>
#include <task.hpp>
#include <gui.hpp>
#include <debug.h>

#include "../kernel.h"

using Tasking::IP;
using Tasking::PCB;
using Tasking::TaskTrustLevel;
using Tasking::TCB;
using namespace GraphicalUserInterface;

namespace Recovery
{
    WidgetCollection *wdgDbgWin = nullptr;
    Window *DbgWin = nullptr;

    void KernelRecovery::RecoveryThread()
    {
        while (wdgDbgWin == nullptr || DbgWin == nullptr)
            TaskManager->Sleep(100);

        wdgDbgWin->CreateLabel({10, 0, 0, 0}, "Scheduler Ticks  /  Last Task Ticks");
        GraphicalUserInterface::Handle SchedLblHnd = wdgDbgWin->CreateLabel({10, 20, 0, 0}, "0000000000000000 / 0000000000000000");

        wdgDbgWin->CreateLabel({10, 60, 0, 0}, "Memory Usage");
        GraphicalUserInterface::Handle MemLblHnd = wdgDbgWin->CreateLabel({10, 80, 0, 0}, "0MB / 0GB (0MB reserved) 0%");

        DbgWin->AddWidget(wdgDbgWin);

        char TicksText[128];
        uint64_t MemUsed = 0;
        uint64_t MemTotal = 0;
        uint64_t MemReserved = 0;
        while (true)
        {
            sprintf(TicksText, "%016ld / %016ld", TaskManager->GetSchedulerTicks(), TaskManager->GetLastTaskTicks());
            wdgDbgWin->SetText(SchedLblHnd, TicksText);
            static int RefreshCounter = 100;
            if (RefreshCounter-- == 0)
            {
                MemUsed = KernelAllocator.GetUsedMemory();
                MemTotal = KernelAllocator.GetTotalMemory();
                MemReserved = KernelAllocator.GetReservedMemory();
                int MemPercent = (MemUsed * 100) / MemTotal;
                sprintf(TicksText, "%ldMB / %ldGB (%ldMB reserved) %d%%", TO_MB(MemUsed), TO_GB(MemTotal), TO_MB(MemReserved), MemPercent);
                wdgDbgWin->SetText(MemLblHnd, TicksText);
                RefreshCounter = 100;
            }
            sprintf(TicksText, "Debug - %ldx%ld", DbgWin->GetPosition().Width, DbgWin->GetPosition().Height);
            DbgWin->SetTitle(TicksText);
            TaskManager->Sleep(100);
        }
    }

    void RecoveryThreadWrapper() { RecoveryScreen->RecoveryThread(); }

    void RebootCommandThread()
    {
        TaskManager->Sleep(1000);
        PowerManager->Reboot();
    }

    void ShutdownCommandThread()
    {
        TaskManager->Sleep(1000);
        PowerManager->Shutdown();
    }

    void RebootCommandWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RebootCommandThread); }
    void ShutdownCommandWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)ShutdownCommandThread); }

    GraphicalUserInterface::GUI *gui = nullptr;
    void GUIWrapper() { gui->Loop(); }

    KernelRecovery::KernelRecovery()
    {
        // PCB *proc = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), "Recovery", TaskTrustLevel::Kernel, nullptr);

        gui = new GraphicalUserInterface::GUI;

        TCB *guiThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)GUIWrapper);
        guiThread->Rename("GUI Thread");
        guiThread->SetPriority(Tasking::TaskPriority::Critical);

        Rect RecoveryModeWindow;
        RecoveryModeWindow.Width = 460;
        RecoveryModeWindow.Height = 100;
        RecoveryModeWindow.Left = Display->GetBuffer(200)->Width / 2 - RecoveryModeWindow.Width / 2;
        RecoveryModeWindow.Top = Display->GetBuffer(200)->Height / 2 - RecoveryModeWindow.Height / 2;
        Window *RecWin = new Window(gui, RecoveryModeWindow, "Recovery Mode");
        gui->AddWindow(RecWin);

        WidgetCollection *wdgRecWin = new WidgetCollection(RecWin);
        wdgRecWin->CreateLabel({80, 10, 0, 0}, "This is not fully implemented.");
        wdgRecWin->CreateLabel({10, 40, 0, 0}, "All you can do is shutdown/reboot the system.");
        wdgRecWin->CreateButton({10, 70, 90, 20}, "Reboot", (uintptr_t)RebootCommandWrapper);
        wdgRecWin->CreateButton({110, 70, 90, 20}, "Shutdown", (uintptr_t)ShutdownCommandWrapper);
        RecWin->AddWidget(wdgRecWin);

        Rect DebugWindow;
        DebugWindow.Width = 370;
        DebugWindow.Height = 100;
        DebugWindow.Left = 25;
        DebugWindow.Top = 25;
        DbgWin = new Window(gui, DebugWindow, "Debug");
        gui->AddWindow(DbgWin);

        wdgDbgWin = new WidgetCollection(DbgWin);
        TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RecoveryThreadWrapper)->SetPriority(Tasking::TaskPriority::Idle);
    }

    KernelRecovery::~KernelRecovery()
    {
        debug("Destructor called");
        delete gui;
    }
}
