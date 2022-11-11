#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(__amd64__)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../../kernel.h"

NewLock(UserInputLock);

namespace CrashHandler
{
    static bool ExceptionOccurred = false;
    int SBIdx = 255;
    __no_stack_protector void printfWrapper(char c, void *unused)
    {
        Display->Print(c, SBIdx, true);
        UNUSED(unused);
    }

    __no_stack_protector void EHPrint(const char *Format, ...)
    {
        va_list args;
        va_start(args, Format);
        vfctprintf(printfWrapper, NULL, Format, args);
        va_end(args);
    }

    __no_stack_protector char *trimwhitespace(char *str)
    {
        char *end;
        while (*str == ' ')
            str++;
        if (*str == 0)
            return str;
        end = str + strlen(str) - 1;
        while (end > str && *end == ' ')
            end--;
        *(end + 1) = 0;
        return str;
    }

    CRData crashdata = {};

    __no_stack_protector void DisplayTopOverlay()
    {
        Video::ScreenBuffer *sb = Display->GetBuffer(SBIdx);
        Video::Font *f = Display->GetCurrentFont();
        Video::FontInfo fi = f->GetInfo();

        for (uint32_t i = 0; i < sb->Width; i++)
            for (uint32_t j = 0; j < fi.Height + 8; j++)
                Display->SetPixel(i, j, 0x282828, SBIdx);

        Display->SetBufferCursor(SBIdx, 8, (fi.Height + 8) / 6);
        switch (SBIdx)
        {
        case 255:
        {
            EHPrint("\eAAAAAAMAIN \e606060DETAILS \e606060FRAMES \e606060TASKS \e606060CONSOLE");
            break;
        }
        case 254:
        {
            EHPrint("\e606060MAIN \eAAAAAADETAILS \e606060FRAMES \e606060TASKS \e606060CONSOLE");
            break;
        }
        case 253:
        {
            EHPrint("\e606060MAIN \e606060DETAILS \eAAAAAAFRAMES \e606060TASKS \e606060CONSOLE");
            break;
        }
        case 252:
        {
            EHPrint("\e606060MAIN \e606060DETAILS \e606060FRAMES \eAAAAAATASKS \e606060CONSOLE");
            break;
        }
        case 251:
        {
            EHPrint("\e606060MAIN \e606060DETAILS \e606060FRAMES \e606060TASKS \eAAAAAACONSOLE");
            break;
        }
        default:
        {
            EHPrint("\e606060MAIN \e606060DETAILS \e606060FRAMES \e606060TASKS \e606060CONSOLE");
            break;
        }
        }
        EHPrint("  \e00AAFF%ldMB / %ldMB (%ldMB Reserved)",
                TO_MB(KernelAllocator.GetUsedMemory()),
                TO_MB(KernelAllocator.GetTotalMemory()),
                TO_MB(KernelAllocator.GetReservedMemory()));
        EHPrint("  \eAA0F0F%s", CPU::Hypervisor());
        EHPrint(" \eAAF00F%s", CPU::Vendor());
        EHPrint(" \eAA00FF%s", CPU::Name());
        Display->SetBufferCursor(SBIdx, 0, fi.Height + 10);
    }

    __no_stack_protector void DisplayBottomOverlay()
    {
        Video::ScreenBuffer *sb = Display->GetBuffer(SBIdx);
        Video::Font *f = Display->GetCurrentFont();
        Video::FontInfo fi = f->GetInfo();

        for (uint32_t i = 0; i < sb->Width; i++)
            for (uint32_t j = sb->Height - fi.Height - 8; j < sb->Height; j++)
                Display->SetPixel(i, j, 0x282828, SBIdx);

        Display->SetBufferCursor(SBIdx, 8, sb->Height - fi.Height - 4);
        EHPrint("\eAAAAAA> \eFAFAFA");
    }

    __no_stack_protector void ArrowInput(uint8_t key)
    {
        switch (key)
        {
        case KEY_D_UP:
            if (SBIdx < 255)
                SBIdx++;
            else
                return;
            break;
        case KEY_D_LEFT:
            if (SBIdx < 255)
                SBIdx++;
            else
                return;
            break;
        case KEY_D_RIGHT:
            if (SBIdx > 250)
                SBIdx--;
            else
                return;
            break;
        case KEY_D_DOWN:
            if (SBIdx > 250)
                SBIdx--;
            else
                return;
            break;
        default:
            break;
        }
        Display->ClearBuffer(SBIdx);
        DisplayTopOverlay();
        EHPrint("\eFAFAFA");

        switch (SBIdx)
        {
        case 255:
        {
            DisplayMainScreen(crashdata);
            break;
        }
        case 254:
        {
            DisplayDetailsScreen(crashdata);
            break;
        }
        case 253:
        {
            DisplayStackFrameScreen(crashdata);
            break;
        }
        case 252:
        {
            DisplayTasksScreen(crashdata);
            break;
        }
        case 251:
        {
            DisplayConsoleScreen(crashdata);
            break;
        }
        default:
        {
            break;
        }
        }
        DisplayBottomOverlay();
        Display->SetBuffer(SBIdx);
    }

    __no_stack_protector void UserInput(char *Input)
    {
        SmartCriticalSection(UserInputLock);
        Display->ClearBuffer(SBIdx);
        DisplayTopOverlay();
        EHPrint("\eFAFAFA");

        if (strcmp(Input, "help") == 0)
        {
            EHPrint("Available commands are:\n");
            EHPrint("exit - Shutdown the OS.\n");
            EHPrint("reboot - Reboot the OS.\n");
            EHPrint("help - Display this help message.\n");
            EHPrint("showbuf <INDEX> - Display the contents of a screen buffer.\n");
            EHPrint("       - A sleep timer will be enabled. This will cause the OS to sleep for an unknown amount of time.\n");
            EHPrint("       - \eFF4400WARNING: This can crash the system if a wrong buffer is selected.\eFAFAFA\n");
            EHPrint("main - Show the main screen.\n");
            EHPrint("details - Show the details screen.\n");
            EHPrint("frames - Show the stack frame screen.\n");
            EHPrint("tasks - Show the tasks screen.\n");
            EHPrint("console - Show the console screen.\n");
            EHPrint("Also, you can use the arrow keys to navigate the menu.\n");
            EHPrint("=========================================================================\n");
            EHPrint("Kernel Compiled at: %s %s with C++ Standard: %d\n", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
            EHPrint("C++ Language Version (__cplusplus): %ld\n", __cplusplus);
        }
        else if (strcmp(Input, "exit") == 0)
        {
            PowerManager->Shutdown();
            EHPrint("\eFFFFFFNow it's safe to turn off your computer.");
            CPU::Stop();
        }
        else if (strcmp(Input, "reboot") == 0)
        {
            PowerManager->Reboot();
            EHPrint("\eFFFFFFNow it's safe to reboot your computer.");
            CPU::Stop();
        }
        else if (strncmp(Input, "showbuf", 7) == 0)
        {
            char *arg = trimwhitespace(Input + 7);
            int tmpidx = SBIdx;
            SBIdx = atoi(arg);
            Display->SetBuffer(SBIdx);
            for (int i = 0; i < 1000000; i++)
                inb(0x80);
            SBIdx = tmpidx;
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "main") == 0)
        {
            SBIdx = 255;
            DisplayTopOverlay();
            DisplayMainScreen(crashdata);
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "details") == 0)
        {
            SBIdx = 254;
            DisplayTopOverlay();
            DisplayDetailsScreen(crashdata);
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "frames") == 0)
        {
            SBIdx = 253;
            DisplayTopOverlay();
            DisplayStackFrameScreen(crashdata);
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "tasks") == 0)
        {
            SBIdx = 252;
            DisplayTopOverlay();
            DisplayTasksScreen(crashdata);
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "console") == 0)
        {
            SBIdx = 251;
            DisplayTopOverlay();
            DisplayConsoleScreen(crashdata);
            Display->SetBuffer(SBIdx);
        }
        else
        {
            if (strlen(Input) > 0)
                EHPrint("Unknown command: %s", Input);
        }
        DisplayBottomOverlay();
        Display->SetBuffer(SBIdx);
    }

    __no_stack_protector void Handle(void *Data)
    {
        CPU::Interrupts(CPU::Disable);
        error("An exception occurred!");
        if (TaskManager)
            TaskManager->Panic();
        SBIdx = 255;
        CHArchTrapFrame *Frame = (CHArchTrapFrame *)Data;
#if defined(__amd64__)
        error("Exception: %#llx", Frame->InterruptNumber);

        if (Frame->cs != GDT_USER_CODE && Frame->cs != GDT_USER_DATA)
        {
            debug("Exception in kernel mode");
            Display->CreateBuffer(0, 0, SBIdx);
        }
        else
        {
            debug("Exception in user mode");
            CPUData *data = GetCurrentCPU();
            if (!data)
            {
                Display->CreateBuffer(0, 0, SBIdx);
                EHPrint("\eFF0000Cannot get CPU data! This results in a kernel crash!");
                error("Cannot get CPU data! This results in a kernel crash!");
                error("This should never happen!");
            }
            else
            {
                debug("CPU %ld data is valid", data->ID);
                if (data->CurrentThread)
                {
                    debug("Current thread is valid %#lx", data->CurrentThread);
                    if (!data->CurrentThread->Security.IsCritical)
                    {
                        debug("Current thread is not critical");
                        UserModeExceptionHandler(Frame);
                        return;
                    }
                    else
                    {
                        Display->CreateBuffer(0, 0, SBIdx);
                        EHPrint("\eFF0000Init process crashed!");
                    }
                }
            }
        }

        if (ExceptionOccurred)
        {
            SBIdx = 255;
            Display->ClearBuffer(SBIdx);
            Display->SetBufferCursor(SBIdx, 0, 0);

            CPU::x64::CR0 cr0 = CPU::x64::readcr0();
            CPU::x64::CR2 cr2 = CPU::x64::readcr2();
            CPU::x64::CR3 cr3 = CPU::x64::readcr3();
            CPU::x64::CR4 cr4 = CPU::x64::readcr4();
            CPU::x64::CR8 cr8 = CPU::x64::readcr8();
            CPU::x64::EFER efer;
            efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

            EHPrint("\eFF0000FS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx\n",
                    CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                    Frame->ss, Frame->cs, Frame->ds);
            EHPrint("R8=%#llx  R9=%#llx  R10=%#llx  R11=%#llx\n", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
            EHPrint("R12=%#llx  R13=%#llx  R14=%#llx  R15=%#llx\n", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
            EHPrint("RAX=%#llx  RBX=%#llx  RCX=%#llx  RDX=%#llx\n", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
            EHPrint("RSI=%#llx  RDI=%#llx  RBP=%#llx  RSP=%#llx\n", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
            EHPrint("RIP=%#llx  RFL=%#llx  INT=%#llx  ERR=%#llx  EFER=%#llx\n", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
            EHPrint("CR0=%#llx  CR2=%#llx  CR3=%#llx  CR4=%#llx  CR8=%#llx\n", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);
            EHPrint("CR0: PE:%s     MP:%s     EM:%s     TS:%s\n     ET:%s     NE:%s     WP:%s     AM:%s\n     NW:%s     CD:%s     PG:%s\n     R0:%#x R1:%#x R2:%#x\n",
                    cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
                    cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
                    cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
                    cr0.Reserved0, cr0.Reserved1, cr0.Reserved2);
            EHPrint("CR2: PFLA: %#llx\n",
                    cr2.PFLA);
            EHPrint("CR3: PWT:%s     PCD:%s    PDBR:%#llx\n",
                    cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);
            EHPrint("CR4: VME:%s     PVI:%s     TSD:%s      DE:%s\n     PSE:%s     PAE:%s     MCE:%s     PGE:%s\n     PCE:%s    UMIP:%s  OSFXSR:%s OSXMMEXCPT:%s\n    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s\n OSXSAVE:%s    SMEP:%s    SMAP:%s     PKE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                    cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
                    cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
                    cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
                    cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
                    cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
                    cr4.Reserved0, cr4.Reserved1, cr4.Reserved2);
            EHPrint("CR8: TPL:%d\n", cr8.TPL);
            EHPrint("RFL: CF:%s     PF:%s     AF:%s     ZF:%s\n     SF:%s     TF:%s     IF:%s     DF:%s\n     OF:%s   IOPL:%s     NT:%s     RF:%s\n     VM:%s     AC:%s    VIF:%s    VIP:%s\n     ID:%s     AlwaysOne:%d\n     R0:%#x R1:%#x R2:%#x R3:%#x\n",
                    Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
                    Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
                    Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
                    Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
                    Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
                    Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);
            EHPrint("EFER: SCE:%s      LME:%s      LMA:%s      NXE:%s\n     SVME:%s    LMSLE:%s    FFXSR:%s      TCE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                    efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
                    efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
                    efer.Reserved0, efer.Reserved1, efer.Reserved2);

            EHPrint("\nException occurred while handling exception! HALTED!");
            Display->SetBuffer(SBIdx);
            CPU::Stop();
        }

        ExceptionOccurred = true;

        debug("Reading control registers...");
        crashdata.Frame = Frame;
        crashdata.cr0 = CPU::x64::readcr0();
        crashdata.cr2 = CPU::x64::readcr2();
        crashdata.cr3 = CPU::x64::readcr3();
        crashdata.cr4 = CPU::x64::readcr4();
        crashdata.cr8 = CPU::x64::readcr8();
        crashdata.efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        // Get debug registers
        asmv("movq %%dr0, %0"
             : "=r"(crashdata.dr0));
        asmv("movq %%dr1, %0"
             : "=r"(crashdata.dr1));
        asmv("movq %%dr2, %0"
             : "=r"(crashdata.dr2));
        asmv("movq %%dr3, %0"
             : "=r"(crashdata.dr3));
        asmv("movq %%dr6, %0"
             : "=r"(crashdata.dr6));
        asmv("movq %%dr7, %0"
             : "=r"(crashdata.dr7));

        CPUData *cpudata = GetCurrentCPU();

        if (cpudata == nullptr)
        {
            EHPrint("\eFFA500Invalid CPU data!\n");
            for (long i = 0; i < MAX_CPU; i++)
            {
                cpudata = GetCPU(i);
                if (cpudata != nullptr)
                    break;
                if (i == MAX_CPU - 1)
                {
                    EHPrint("\eFF0000No CPU data found!\n");
                    cpudata = nullptr;
                }
            }
            debug("CPU ptr %#lx", cpudata);
        }

        if (cpudata != nullptr)
        {
            crashdata.ID = cpudata->ID;
            error("Technical Informations on CPU %lld:", cpudata->ID);
        }

        if (TaskManager && cpudata != nullptr)
        {
            crashdata.Process = cpudata->CurrentProcess;
            crashdata.Thread = cpudata->CurrentThread;

            error("Current Process: %s(%ld)\n",
                  cpudata->CurrentProcess->Name,
                  cpudata->CurrentProcess->ID);
            error("Current Thread: %s(%ld)\n",
                  cpudata->CurrentThread->Name,
                  cpudata->CurrentThread->ID);
        }

        {
            error("FS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx",
                  CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                  Frame->ss, Frame->cs, Frame->ds);
            error("R8=%#llx  R9=%#llx  R10=%#llx  R11=%#llx", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
            error("R12=%#llx  R13=%#llx  R14=%#llx  R15=%#llx", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
            error("RAX=%#llx  RBX=%#llx  RCX=%#llx  RDX=%#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
            error("RSI=%#llx  RDI=%#llx  RBP=%#llx  RSP=%#llx", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
            error("RIP=%#llx  RFL=%#llx  INT=%#llx  ERR=%#llx  EFER=%#llx", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, crashdata.efer.raw);
            error("CR0=%#llx  CR2=%#llx  CR3=%#llx  CR4=%#llx  CR8=%#llx", crashdata.cr0.raw, crashdata.cr2.raw, crashdata.cr3.raw, crashdata.cr4.raw, crashdata.cr8.raw);
            error("DR0=%#llx  DR1=%#llx  DR2=%#llx  DR3=%#llx  DR6=%#llx  DR7=%#llx", crashdata.dr0, crashdata.dr1, crashdata.dr2, crashdata.dr3, crashdata.dr6, crashdata.dr7.raw);

            error("CR0: PE:%s     MP:%s     EM:%s     TS:%s     ET:%s     NE:%s     WP:%s     AM:%s     NW:%s     CD:%s     PG:%s     R0:%#x R1:%#x R2:%#x",
                  crashdata.cr0.PE ? "True " : "False", crashdata.cr0.MP ? "True " : "False", crashdata.cr0.EM ? "True " : "False", crashdata.cr0.TS ? "True " : "False",
                  crashdata.cr0.ET ? "True " : "False", crashdata.cr0.NE ? "True " : "False", crashdata.cr0.WP ? "True " : "False", crashdata.cr0.AM ? "True " : "False",
                  crashdata.cr0.NW ? "True " : "False", crashdata.cr0.CD ? "True " : "False", crashdata.cr0.PG ? "True " : "False",
                  crashdata.cr0.Reserved0, crashdata.cr0.Reserved1, crashdata.cr0.Reserved2);

            error("CR2: PFLA: %#llx",
                  crashdata.cr2.PFLA);

            error("CR3: PWT:%s     PCD:%s    PDBR:%#llx",
                  crashdata.cr3.PWT ? "True " : "False", crashdata.cr3.PCD ? "True " : "False", crashdata.cr3.PDBR);

            error("CR4: VME:%s     PVI:%s     TSD:%s      DE:%s     PSE:%s     PAE:%s     MCE:%s     PGE:%s     PCE:%s    UMIP:%s  OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s     PKE:%s     R0:%#x R1:%#x R2:%#x",
                  crashdata.cr4.VME ? "True " : "False", crashdata.cr4.PVI ? "True " : "False", crashdata.cr4.TSD ? "True " : "False", crashdata.cr4.DE ? "True " : "False",
                  crashdata.cr4.PSE ? "True " : "False", crashdata.cr4.PAE ? "True " : "False", crashdata.cr4.MCE ? "True " : "False", crashdata.cr4.PGE ? "True " : "False",
                  crashdata.cr4.PCE ? "True " : "False", crashdata.cr4.UMIP ? "True " : "False", crashdata.cr4.OSFXSR ? "True " : "False", crashdata.cr4.OSXMMEXCPT ? "True " : "False",
                  crashdata.cr4.LA57 ? "True " : "False", crashdata.cr4.VMXE ? "True " : "False", crashdata.cr4.SMXE ? "True " : "False", crashdata.cr4.PCIDE ? "True " : "False",
                  crashdata.cr4.OSXSAVE ? "True " : "False", crashdata.cr4.SMEP ? "True " : "False", crashdata.cr4.SMAP ? "True " : "False", crashdata.cr4.PKE ? "True " : "False",
                  crashdata.cr4.Reserved0, crashdata.cr4.Reserved1, crashdata.cr4.Reserved2);

            error("CR8: TPL:%d", crashdata.cr8.TPL);

            error("RFL: CF:%s     PF:%s     AF:%s     ZF:%s     SF:%s     TF:%s     IF:%s     DF:%s     OF:%s   IOPL:%s     NT:%s     RF:%s     VM:%s     AC:%s    VIF:%s    VIP:%s     ID:%s     AlwaysOne:%d     R0:%#x R1:%#x R2:%#x R3:%#x",
                  Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
                  Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
                  Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
                  Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
                  Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
                  Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);

            error("DR7: LDR0:%s     GDR0:%s     LDR1:%s     GDR1:%s     LDR2:%s     GDR2:%s     LDR3:%s     GDR3:%s     CDR0:%s     SDR0:%s     CDR1:%s     SDR1:%s     CDR2:%s     SDR2:%s     CDR3:%s     SDR3:%s     R:%#x",
                  crashdata.dr7.LocalDR0 ? "True " : "False", crashdata.dr7.GlobalDR0 ? "True " : "False", crashdata.dr7.LocalDR1 ? "True " : "False", crashdata.dr7.GlobalDR1 ? "True " : "False",
                  crashdata.dr7.LocalDR2 ? "True " : "False", crashdata.dr7.GlobalDR2 ? "True " : "False", crashdata.dr7.LocalDR3 ? "True " : "False", crashdata.dr7.GlobalDR3 ? "True " : "False",
                  crashdata.dr7.ConditionsDR0 ? "True " : "False", crashdata.dr7.SizeDR0 ? "True " : "False", crashdata.dr7.ConditionsDR1 ? "True " : "False", crashdata.dr7.SizeDR1 ? "True " : "False",
                  crashdata.dr7.ConditionsDR2 ? "True " : "False", crashdata.dr7.SizeDR2 ? "True " : "False", crashdata.dr7.ConditionsDR3 ? "True " : "False", crashdata.dr7.SizeDR3 ? "True " : "False",
                  crashdata.dr7.Reserved);

            error("EFER: SCE:%s      LME:%s      LMA:%s      NXE:%s     SVME:%s    LMSLE:%s    FFXSR:%s      TCE:%s     R0:%#x R1:%#x R2:%#x",
                  crashdata.efer.SCE ? "True " : "False", crashdata.efer.LME ? "True " : "False", crashdata.efer.LMA ? "True " : "False", crashdata.efer.NXE ? "True " : "False",
                  crashdata.efer.SVME ? "True " : "False", crashdata.efer.LMSLE ? "True " : "False", crashdata.efer.FFXSR ? "True " : "False", crashdata.efer.TCE ? "True " : "False",
                  crashdata.efer.Reserved0, crashdata.efer.Reserved1, crashdata.efer.Reserved2);
        }
        goto CrashEnd;

#elif defined(__i386__)
        void *Frame = Data;
#elif defined(__aarch64__)
        void *Frame = Data;
#endif

    CrashEnd:
        if (Config.InterruptsOnCrash)
        {
            // 255 // Main
            Display->CreateBuffer(0, 0, 254); // Details
            Display->CreateBuffer(0, 0, 253); // Frames
            Display->CreateBuffer(0, 0, 252); // Tasks
            Display->CreateBuffer(0, 0, 251); // Console
            Display->CreateBuffer(0, 0, 250); // Empty

            DisplayTopOverlay();
            DisplayMainScreen(crashdata);
            DisplayBottomOverlay();
            Display->SetBuffer(255);
            debug("Interrupts are enabled, waiting for user input");
            CPU::Interrupts(CPU::Enable);
            HookKeyboard();
        }
        else
        {
            /*
            TODO: Stuff that should be done when IOC is disabled.
            */
            Display->SetBuffer(255);
        }

        CPU::Halt(true);
    }
}
