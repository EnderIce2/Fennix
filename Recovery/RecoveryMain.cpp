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
    void KernelRecovery::RecoveryThread()
    {
        while (true)
        {
        }
    }

    void RecoveryThreadWrapper() { RecoveryScreen->RecoveryThread(); }
    void RebootCommandWrapper() { PowerManager->Reboot(); }
    void ShutdownCommandWrapper() { PowerManager->Shutdown(); }

    GraphicalUserInterface::GUI *gui = nullptr;
    void GUIWrapper() { gui->Loop(); }

    KernelRecovery::KernelRecovery()
    {
        // PCB *proc = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), "Recovery", TaskTrustLevel::Kernel, nullptr);

        gui = new GraphicalUserInterface::GUI;

        // TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RecoveryThreadWrapper);
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
        GraphicalUserInterface::Handle SchedLblHnd = wdgRecWin->CreateLabel({10, 0, 0, 0}, "Scheduler Ticks: 0");
        wdgRecWin->CreateLabel({10, 20, 0, 0}, "This is not fully implemented.");
        wdgRecWin->CreateLabel({10, 40, 0, 0}, "All you can do is shutdown/reboot the system.");
        wdgRecWin->CreateButton({10, 70, 90, 20}, "Reboot", (uintptr_t)RebootCommandWrapper);
        wdgRecWin->CreateButton({110, 70, 90, 20}, "Shutdown", (uintptr_t)ShutdownCommandWrapper);
        RecWin->AddWidget(wdgRecWin);

        char TicksText[128];
        while (true)
        {
            sprintf(TicksText, "Scheduler Ticks: %ld", TaskManager->GetSchedulerTicks());
            wdgRecWin->SetText(SchedLblHnd, TicksText);
        }
    }

    KernelRecovery::~KernelRecovery()
    {
        debug("Destructor called");
        delete gui;
    }
}
