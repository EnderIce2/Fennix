#include <recovery.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <gui.hpp>
#include <debug.h>

#include "../kernel.h"
#include "../Fex.hpp"
#include "../DAPI.hpp"

using Tasking::IP;
using Tasking::PCB;
using Tasking::TaskTrustLevel;
using Tasking::TCB;
using VirtualFileSystem::File;
using VirtualFileSystem::FileStatus;
using namespace GraphicalUserInterface;

#ifdef DEBUG
extern uint64_t FIi, PDi, PWi, PWWi, PCi, mmi;
#endif

extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start;
extern uintptr_t _binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end;

NewLock(PlayAudioLock);

namespace Recovery
{
    WidgetCollection *wdgDbgWin = nullptr;
    Window *DbgWin = nullptr;

    char *AudioFile = (char *)"/home/default/Music/pcm/FurElise48000.pcm";

    void PlayAudio()
    {
        SmartLock(PlayAudioLock);
        Driver::DriverFile *AudioDrv = nullptr;

        foreach (auto Driver in DriverManager->GetDrivers())
        {
            if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Audio)
            {
                AudioDrv = Driver;
                break;
            }
        }

        if (AudioDrv == nullptr)
        {
            error("No audio drivers found! Cannot play audio!");
            return;
        }

        shared_ptr<VirtualFileSystem::File> pcm = vfs->Open(AudioFile);

        if (pcm->Status != FileStatus::OK)
        {
            error("Cannot open audio file! Cannot play audio!");
            return;
        }

        void *PCMRaw = KernelAllocator.RequestPages(TO_PAGES(pcm->node->Length));
        memcpy(PCMRaw, (void *)pcm->node->Address, pcm->node->Length);

        KernelCallback *callback = (KernelCallback *)KernelAllocator.RequestPages(TO_PAGES(sizeof(KernelCallback)));
        memset(callback, 0, sizeof(KernelCallback));
        callback->Reason = SendReason;
        callback->AudioCallback.Send.Data = (uint8_t *)PCMRaw;
        callback->AudioCallback.Send.Length = pcm->node->Length;
        debug("Playing audio...");
        int status = DriverManager->IOCB(AudioDrv->DriverUID, (void *)callback);
        debug("Audio played! %d", status);
        KernelAllocator.FreePages((void *)PCMRaw, TO_PAGES(pcm->node->Length));
        KernelAllocator.FreePages((void *)callback, TO_PAGES(sizeof(KernelCallback)));
    }

    void PlayAudioWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)PlayAudio)->SetPriority(Tasking::TaskPriority::Idle); }

    void ChangeSampleRate(char SR)
    {
        Driver::DriverFile *AudioDrv = nullptr;

        foreach (auto Driver in DriverManager->GetDrivers())
        {
            if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Audio)
            {
                AudioDrv = Driver;
                break;
            }
        }

        if (AudioDrv == nullptr)
        {
            error("No audio drivers found! Cannot play audio!");
            return;
        }

        KernelCallback *callback = (KernelCallback *)KernelAllocator.RequestPages(TO_PAGES(sizeof(KernelCallback)));
        memset(callback, 0, sizeof(KernelCallback));
        callback->Reason = AdjustReason;
        callback->AudioCallback.Adjust._SampleRate = true;
        callback->AudioCallback.Adjust.SampleRate = SR;
        int status = DriverManager->IOCB(AudioDrv->DriverUID, (void *)callback);
        KernelAllocator.FreePages((void *)callback, TO_PAGES(sizeof(KernelCallback)));
    }

    void CSR8000() { ChangeSampleRate(0); }
    void CSR11025() { ChangeSampleRate(1); }
    void CSR16000() { ChangeSampleRate(2); }
    void CSR22050() { ChangeSampleRate(3); }
    void CSR32000() { ChangeSampleRate(4); }
    void CSR44100() { ChangeSampleRate(5); }
    void CSR48000() { ChangeSampleRate(6); }
    void CSR88200() { ChangeSampleRate(7); }
    void CSR96000() { ChangeSampleRate(8); }

    void KernelRecovery::RecoveryThread()
    {
        while (wdgDbgWin == nullptr || DbgWin == nullptr)
            TaskManager->Sleep(100);

        wdgDbgWin->CreateLabel({5, 0, 0, 0}, "Scheduler Ticks  /  Last Task Ticks");
        GraphicalUserInterface::Handle SchedLblHnd = wdgDbgWin->CreateLabel({5, 15, 0, 0}, "0000000000000000 / 0000000000000000");

        wdgDbgWin->CreateLabel({5, 40, 0, 0}, "Memory Usage");
        GraphicalUserInterface::Handle MemLblHnd = wdgDbgWin->CreateLabel({5, 55, 0, 0}, "0MB / 0GB (0MB reserved) 0%");

        wdgDbgWin->CreateLabel({5, 95, 0, 0}, "GUI Info");
        wdgDbgWin->CreateLabel({5, 110, 0, 0}, "  Fetch Inputs   /  Paint Desktop   / Paint Widgets");
        GraphicalUserInterface::Handle GUI1LblHnd = wdgDbgWin->CreateLabel({5, 125, 0, 0}, "0000000000000000 / 0000000000000000 / 0000000000000000");
        wdgDbgWin->CreateLabel({5, 140, 0, 0}, "  Paint Windows  /   Paint Cursor   / Memset & Update");
        GraphicalUserInterface::Handle GUI2LblHnd = wdgDbgWin->CreateLabel({5, 155, 0, 0}, "0000000000000000 / 0000000000000000 / 0000000000000000");

        wdgDbgWin->CreateLabel({5, 195, 0, 0}, "Audio");
        wdgDbgWin->CreateButton({5, 210, 85, 15}, "Play Audio", (uintptr_t)PlayAudioWrapper);
        wdgDbgWin->CreateButton({95, 210, 70, 15}, "8000 Hz", (uintptr_t)CSR8000);
        wdgDbgWin->CreateButton({185, 210, 70, 15}, "11025 Hz", (uintptr_t)CSR11025);
        wdgDbgWin->CreateButton({275, 210, 70, 15}, "16000 Hz", (uintptr_t)CSR16000);
        wdgDbgWin->CreateButton({365, 210, 70, 15}, "22050 Hz", (uintptr_t)CSR22050);
        wdgDbgWin->CreateButton({5, 230, 70, 15}, "32000 Hz", (uintptr_t)CSR32000);
        wdgDbgWin->CreateButton({95, 230, 70, 15}, "44100 Hz", (uintptr_t)CSR44100);
        wdgDbgWin->CreateButton({185, 230, 70, 15}, "48000 Hz", (uintptr_t)CSR48000);
        wdgDbgWin->CreateButton({275, 230, 70, 15}, "88200 Hz", (uintptr_t)CSR88200);
        wdgDbgWin->CreateButton({365, 230, 70, 15}, "96000 Hz", (uintptr_t)CSR96000);

        DbgWin->AddWidget(wdgDbgWin);

        char TicksText[128];
        uint64_t MemUsed = 0;
        uint64_t MemTotal = 0;
        uint64_t MemReserved = 0;
        while (true)
        {
            sprintf(TicksText, "%016ld / %016ld", TaskManager->GetSchedulerTicks(), TaskManager->GetLastTaskTicks());
            wdgDbgWin->SetText(SchedLblHnd, TicksText);
            static int RefreshMemCounter = 0;
            if (RefreshMemCounter-- == 0)
            {
                MemUsed = KernelAllocator.GetUsedMemory();
                MemTotal = KernelAllocator.GetTotalMemory();
                MemReserved = KernelAllocator.GetReservedMemory();
                int MemPercent = (MemUsed * 100) / MemTotal;
                sprintf(TicksText, "%ldMB / %ldGB (%ldMB reserved) %d%%", TO_MB(MemUsed), TO_GB(MemTotal), TO_MB(MemReserved), MemPercent);
                wdgDbgWin->SetText(MemLblHnd, TicksText);
                RefreshMemCounter = 50;
            }
            sprintf(TicksText, "Debug - %ldx%ld", DbgWin->GetPosition().Width, DbgWin->GetPosition().Height);
            DbgWin->SetTitle(TicksText);

#ifdef DEBUG
            static int RefreshGUIDbgCounter = 0;
            if (RefreshGUIDbgCounter-- == 0)
            {
                sprintf(TicksText, "%016ld / %016ld / %016ld", FIi, PDi, PWi);
                wdgDbgWin->SetText(GUI1LblHnd, TicksText);
                sprintf(TicksText, "%016ld / %016ld / %016ld", PWWi, PCi, mmi);
                wdgDbgWin->SetText(GUI2LblHnd, TicksText);
                RefreshGUIDbgCounter = 5;
            }
#endif
            TaskManager->Sleep(100);
        }
    }

    void RecoveryThreadWrapper()
    {
        while (!RecoveryScreen)
            CPU::Pause();
        RecoveryScreen->RecoveryThread();
    }

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
        DebugWindow.Width = 460;
        DebugWindow.Height = 305;
        DebugWindow.Left = 5;
        DebugWindow.Top = 25;
        DbgWin = new Window(gui, DebugWindow, "Debug");
        gui->AddWindow(DbgWin);

        wdgDbgWin = new WidgetCollection(DbgWin);
        Video::Font *NewFont = new Video::Font(&_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start, &_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end, Video::FontType::PCScreenFont2);
        wdgDbgWin->ReplaceFont(NewFont);
        TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RecoveryThreadWrapper)->SetPriority(Tasking::TaskPriority::Idle);
    }

    KernelRecovery::~KernelRecovery()
    {
        debug("Destructor called");
        delete gui;
    }
}
