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
        Driver::DriverFile AudioDrv;

        foreach (auto Driver in DriverManager->GetDrivers())
        {
            if (((FexExtended *)((uintptr_t)Driver.Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Audio)
            {
                AudioDrv = Driver;
                break;
            }
        }

        if (!AudioDrv.Enabled)
        {
            error("No audio drivers found! Cannot play audio!");
            return;
        }

        std::shared_ptr<VirtualFileSystem::File> pcm = vfs->Open(AudioFile);

        if (pcm->Status != FileStatus::OK)
        {
            error("Cannot open audio file! Cannot play audio!");
            return;
        }

        void *PCMRaw = KernelAllocator.RequestPages(TO_PAGES(pcm->node->Length));
        memcpy(PCMRaw, (void *)pcm->node->Address, pcm->node->Length);

        KernelCallback callback;
        memset(&callback, 0, sizeof(KernelCallback));
        callback.Reason = SendReason;
        callback.AudioCallback.Send.Data = (uint8_t *)PCMRaw;
        callback.AudioCallback.Send.Length = pcm->node->Length;
        debug("Playing audio...");
        int status = DriverManager->IOCB(AudioDrv.DriverUID, (void *)&callback);
        debug("Audio played! %d", status);
        KernelAllocator.FreePages((void *)PCMRaw, TO_PAGES(pcm->node->Length));
        vfs->Close(pcm);
    }

    void PlayAudioWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)PlayAudio)->SetPriority(Tasking::TaskPriority::Idle); }

    void ChangeSampleRate(char SR)
    {
        Driver::DriverFile AudioDrv;

        foreach (auto Driver in DriverManager->GetDrivers())
        {
            if (((FexExtended *)((uintptr_t)Driver.Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Audio)
            {
                AudioDrv = Driver;
                break;
            }
        }

        if (!AudioDrv.Enabled)
        {
            error("No audio drivers found! Cannot play audio!");
            return;
        }

        KernelCallback callback;
        memset(&callback, 0, sizeof(KernelCallback));
        callback.Reason = AdjustReason;
        callback.AudioCallback.Adjust._SampleRate = true;
        callback.AudioCallback.Adjust.SampleRate = SR;
        int status = DriverManager->IOCB(AudioDrv.DriverUID, (void *)&callback);
        debug("Sample rate changed! %d", status);
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

    void ChangeVolume(char percentage)
    {
        Driver::DriverFile AudioDrv;

        foreach (auto Driver in DriverManager->GetDrivers())
        {
            if (((FexExtended *)((uintptr_t)Driver.Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Audio)
            {
                AudioDrv = Driver;
                break;
            }
        }

        if (!AudioDrv.Enabled)
        {
            error("No audio drivers found! Cannot play audio!");
            return;
        }

        KernelCallback callback;
        memset(&callback, 0, sizeof(KernelCallback));
        callback.Reason = AdjustReason;
        callback.AudioCallback.Adjust._Volume = true;
        callback.AudioCallback.Adjust.Volume = percentage;
        int status = DriverManager->IOCB(AudioDrv.DriverUID, (void *)&callback);
        debug("Volume changed! %d", status);
    }

    void CV0() { ChangeVolume(0); }
    void CV10() { ChangeVolume(10); }
    void CV20() { ChangeVolume(20); }
    void CV30() { ChangeVolume(30); }
    void CV40() { ChangeVolume(40); }
    void CV50() { ChangeVolume(50); }
    void CV60() { ChangeVolume(60); }
    void CV70() { ChangeVolume(70); }
    void CV80() { ChangeVolume(80); }
    void CV90() { ChangeVolume(90); }
    void CV100() { ChangeVolume(100); }

    void BufBight10() { Display->SetBrightness(10, 200); }
    void BufBight20() { Display->SetBrightness(20, 200); }
    void BufBight30() { Display->SetBrightness(30, 200); }
    void BufBight40() { Display->SetBrightness(40, 200); }
    void BufBight50() { Display->SetBrightness(50, 200); }
    void BufBight60() { Display->SetBrightness(60, 200); }
    void BufBight70() { Display->SetBrightness(70, 200); }
    void BufBight80() { Display->SetBrightness(80, 200); }
    void BufBight90() { Display->SetBrightness(90, 200); }
    void BufBight100() { Display->SetBrightness(100, 200); }

    // void audio_dev_connected() { AudioFile = (char *)"/system/config/audio/media/dev_connected.mp3"; }
    // void audio_dev_disconnected() { AudioFile = (char *)"/system/config/audio/media/dev_disconnected.mp3"; }
    // void audio_dev_error() { AudioFile = (char *)"/system/config/audio/media/dev_error.mp3"; }
    // void audio_error() { AudioFile = (char *)"/system/config/audio/media/error.mp3"; }
    // void audio_notification() { AudioFile = (char *)"/system/config/audio/media/notification.mp3"; }
    // void audio_warning() { AudioFile = (char *)"/system/config/audio/media/warning.mp3"; }

    void KernelRecovery::RecoveryThread()
    {
        while (wdgDbgWin == nullptr || DbgWin == nullptr)
            TaskManager->Sleep(100);

        wdgDbgWin->CreateLabel({5, 0, 0, 0}, "Scheduler Ticks  /  Last Task Ticks");
        GraphicalUserInterface::Handle SchedLblHnd = wdgDbgWin->CreateLabel({5, 15, 0, 0}, "0000000000000000 / 0000000000000000");

        wdgDbgWin->CreateLabel({5, 40, 0, 0}, "Memory Usage");
        GraphicalUserInterface::Handle MemLblHnd = wdgDbgWin->CreateLabel({5, 55, 0, 0}, "0MB / 0GB (0MB reserved) 0% (0000000000000000 bytes allocated)");

        wdgDbgWin->CreateLabel({5, 95, 0, 0}, "GUI Info");
        wdgDbgWin->CreateLabel({5, 110, 0, 0}, "  Fetch Inputs   /  Paint Desktop   / Paint Widgets");
        GraphicalUserInterface::Handle GUI1LblHnd = wdgDbgWin->CreateLabel({5, 125, 0, 0}, "0000000000000000 / 0000000000000000 / 0000000000000000");
        wdgDbgWin->CreateLabel({5, 140, 0, 0}, "  Paint Windows  /   Paint Cursor   / Memset & Update");
        GraphicalUserInterface::Handle GUI2LblHnd = wdgDbgWin->CreateLabel({5, 155, 0, 0}, "0000000000000000 / 0000000000000000 / 0000000000000000");

        wdgDbgWin->CreateLabel({5, 195, 0, 0}, "Audio");
        wdgDbgWin->CreateButton({5, 210, 85, 15}, "Play Audio", (uintptr_t)PlayAudioWrapper);

        wdgDbgWin->CreateLabel({5, 235, 0, 0}, "Sample Rate");
        wdgDbgWin->CreateButton({5, 250, 45, 15}, "8000", (uintptr_t)CSR8000);
        wdgDbgWin->CreateButton({55, 250, 45, 15}, "11025", (uintptr_t)CSR11025);
        wdgDbgWin->CreateButton({105, 250, 45, 15}, "16000", (uintptr_t)CSR16000);
        wdgDbgWin->CreateButton({155, 250, 45, 15}, "22050", (uintptr_t)CSR22050);
        wdgDbgWin->CreateButton({205, 250, 45, 15}, "32000", (uintptr_t)CSR32000);
        wdgDbgWin->CreateButton({255, 250, 45, 15}, "44100", (uintptr_t)CSR44100);
        wdgDbgWin->CreateButton({305, 250, 45, 15}, "48000", (uintptr_t)CSR48000);
        wdgDbgWin->CreateButton({355, 250, 45, 15}, "88200", (uintptr_t)CSR88200);
        wdgDbgWin->CreateButton({405, 250, 45, 15}, "96000", (uintptr_t)CSR96000);

        wdgDbgWin->CreateLabel({5, 265, 0, 0}, "Volume");
        wdgDbgWin->CreateButton({5, 280, 25, 15}, "0%", (uintptr_t)CV0);
        wdgDbgWin->CreateButton({35, 280, 25, 15}, "10%", (uintptr_t)CV10);
        wdgDbgWin->CreateButton({65, 280, 25, 15}, "20%", (uintptr_t)CV20);
        wdgDbgWin->CreateButton({95, 280, 25, 15}, "30%", (uintptr_t)CV30);
        wdgDbgWin->CreateButton({125, 280, 25, 15}, "40%", (uintptr_t)CV40);
        wdgDbgWin->CreateButton({155, 280, 25, 15}, "50%", (uintptr_t)CV50);
        wdgDbgWin->CreateButton({185, 280, 25, 15}, "60%", (uintptr_t)CV60);
        wdgDbgWin->CreateButton({215, 280, 25, 15}, "70%", (uintptr_t)CV70);
        wdgDbgWin->CreateButton({245, 280, 25, 15}, "80%", (uintptr_t)CV80);
        wdgDbgWin->CreateButton({275, 280, 25, 15}, "90%", (uintptr_t)CV90);
        wdgDbgWin->CreateButton({305, 280, 25, 15}, "100%", (uintptr_t)CV100);

        GraphicalUserInterface::Handle wdgDbgCurrentAudioLbl = wdgDbgWin->CreateLabel({5, 295, 0, 0}, "Current Audio: ");
        // wdgDbgWin->CreateButton({5, 310, 85, 15}, "dev_connected.mp3", (uintptr_t)audio_dev_connected);
        // wdgDbgWin->CreateButton({95, 310, 85, 15}, "dev_disconnected.mp3", (uintptr_t)audio_dev_disconnected);
        // wdgDbgWin->CreateButton({185, 310, 85, 15}, "dev_error.mp3", (uintptr_t)audio_dev_error);
        // wdgDbgWin->CreateButton({275, 310, 85, 15}, "error.mp3", (uintptr_t)audio_error);
        // wdgDbgWin->CreateButton({365, 310, 85, 15}, "notification.mp3", (uintptr_t)audio_notification);
        // wdgDbgWin->CreateButton({455, 310, 85, 15}, "warning.mp3", (uintptr_t)audio_warning);

        wdgDbgWin->CreateLabel({5, 325, 0, 0}, "Display Brightness");
        wdgDbgWin->CreateButton({5, 340, 25, 15}, "10%", (uintptr_t)BufBight10);
        wdgDbgWin->CreateButton({35, 340, 25, 15}, "20%", (uintptr_t)BufBight20);
        wdgDbgWin->CreateButton({65, 340, 25, 15}, "30%", (uintptr_t)BufBight30);
        wdgDbgWin->CreateButton({95, 340, 25, 15}, "40%", (uintptr_t)BufBight40);
        wdgDbgWin->CreateButton({125, 340, 25, 15}, "50%", (uintptr_t)BufBight50);
        wdgDbgWin->CreateButton({155, 340, 25, 15}, "60%", (uintptr_t)BufBight60);
        wdgDbgWin->CreateButton({185, 340, 25, 15}, "70%", (uintptr_t)BufBight70);
        wdgDbgWin->CreateButton({215, 340, 25, 15}, "80%", (uintptr_t)BufBight80);
        wdgDbgWin->CreateButton({245, 340, 25, 15}, "90%", (uintptr_t)BufBight90);
        wdgDbgWin->CreateButton({275, 340, 25, 15}, "100%", (uintptr_t)BufBight100);

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
                uint64_t MemPercent = (MemUsed * 100) / MemTotal;
                sprintf(TicksText, "%ldMB / %ldGB (%ldMB reserved) %ld%% (%ld bytes allocated)", TO_MB(MemUsed), TO_GB(MemTotal), TO_MB(MemReserved), MemPercent, MemUsed);
                wdgDbgWin->SetText(MemLblHnd, TicksText);
                RefreshMemCounter = 25;
            }
            sprintf(TicksText, "Debug - %ldx%ld", DbgWin->GetPosition().Width, DbgWin->GetPosition().Height);
            DbgWin->SetTitle(TicksText);
            sprintf(TicksText, "Current Audio: %s", AudioFile);
            wdgDbgWin->SetText(wdgDbgCurrentAudioLbl, TicksText);

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

    void RebootCommandThread() { KST_Reboot(); }
    void ShutdownCommandThread() { KST_Shutdown(); }

    void RebootCommandWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RebootCommandThread); }
    void ShutdownCommandWrapper() { TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)ShutdownCommandThread); }

    GraphicalUserInterface::GUI *gui = nullptr;
    void GUIWrapper() { gui->Loop(); }

    KernelRecovery::KernelRecovery()
    {
        // PCB *proc = TaskManager->CreateProcess(TaskManager->GetCurrentProcess(), "Recovery", TaskTrustLevel::Kernel, nullptr);

        gui = new GraphicalUserInterface::GUI;

        guiThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)GUIWrapper);
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
        DebugWindow.Height = 380;
        DebugWindow.Left = 5;
        DebugWindow.Top = 25;
        DbgWin = new Window(gui, DebugWindow, "Debug");
        gui->AddWindow(DbgWin);

        wdgDbgWin = new WidgetCollection(DbgWin);
        Video::Font *NewFont = new Video::Font(&_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_start, &_binary_Files_tamsyn_font_1_11_Tamsyn7x14r_psf_end, Video::FontType::PCScreenFont2);
        wdgDbgWin->ReplaceFont(NewFont);
        recoveryThread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (IP)RecoveryThreadWrapper);
        recoveryThread->Rename("Recovery Thread");
        recoveryThread->SetPriority(Tasking::TaskPriority::Idle);
    }

    KernelRecovery::~KernelRecovery()
    {
        debug("Destructor called");
        TaskManager->KillThread(guiThread, 0);
        TaskManager->KillThread(recoveryThread, 0);
        delete gui, gui = nullptr;
    }
}
