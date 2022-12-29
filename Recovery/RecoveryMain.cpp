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
        Window *win = new Window(gui, {50, 50, 400, 250}, "Recovery");
        WidgetCollection *wdg = new WidgetCollection(nullptr);
        wdg->CreatePanel(0, Display->GetBuffer(0)->Height - 25, Display->GetBuffer(0)->Width, 25);
        gui->AddWindow(win);
        win->AddWidget(wdg);
    }

    KernelRecovery::~KernelRecovery()
    {
        delete gui;
    }
}
