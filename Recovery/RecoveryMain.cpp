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

        Vector<AuxiliaryVector> auxv;
        auxv.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});
        // TaskManager->CreateThread(proc, (IP)RecoveryThreadWrapper, nullptr, nullptr, auxv);
        TCB *guiThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)GUIWrapper, nullptr, nullptr, auxv);
        guiThread->Rename("GUI Thread");
        guiThread->SetPriority(100);

        Rect RecoveryModeWindow;
        RecoveryModeWindow.Width = 460;
        RecoveryModeWindow.Height = 100;
        RecoveryModeWindow.Left = Display->GetBuffer(200)->Width / 2 - RecoveryModeWindow.Width / 2;
        RecoveryModeWindow.Top = Display->GetBuffer(200)->Height / 2 - RecoveryModeWindow.Height / 2;
        Window *win = new Window(gui, RecoveryModeWindow, "Recovery Mode");
        gui->AddWindow(win);

        WidgetCollection *wdg = new WidgetCollection(win);
        wdg->CreateLabel({10, 10, 0, 0}, "This is not fully implemented.");
        wdg->CreateLabel({10, 25, 0, 0}, "All you can do is shutdown/reboot the system.");
        wdg->CreateButton({10, 50, 90, 20}, "Reboot", (uintptr_t)RebootCommandWrapper);
        wdg->CreateButton({110, 50, 90, 20}, "Shutdown", (uintptr_t)ShutdownCommandWrapper);

        win->AddWidget(wdg);
    }

    KernelRecovery::~KernelRecovery()
    {
        delete gui;
    }
}
