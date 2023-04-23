/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <bitmap.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <rand.hpp>
#include <uart.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#include "../Architecture/amd64/cpu/apic.hpp"
#elif defined(a32)
#include "../../Architecture/i386/cpu/gdt.hpp"
#include "../Architecture/i386/cpu/apic.hpp"
#elif defined(aa64)
#endif

#include "../../kernel.h"
#include "../../DAPI.hpp"

NewLock(UserInputLock);

namespace CrashHandler
{
    uintptr_t PageFaultAddress = 0;
    void *EHIntFrames[INT_FRAMES_MAX];
    static bool ExceptionOccurred = false;
    int SBIdx = 255;

    SafeFunction void printfWrapper(char c, void *unused)
    {
        Display->Print(c, SBIdx, true);
        UNUSED(unused);
    }

    SafeFunction void EHPrint(const char *Format, ...)
    {
        va_list args;
        va_start(args, Format);
        vfctprintf(printfWrapper, NULL, Format, args);
        va_end(args);
    }

    SafeFunction void EHDumpData(void *Address, unsigned long Length)
    {
        EHPrint("-------------------------------------------------------------------------\n");
        Display->SetBuffer(SBIdx);
        unsigned char *AddressChar = (unsigned char *)Address;
        unsigned char Buffer[17];
        unsigned long Iterate;
        for (Iterate = 0; Iterate < Length; Iterate++)
        {
            if ((Iterate % 16) == 0)
            {
                if (Iterate != 0)
                    EHPrint("  \e8A78FF%s\eAABBCC\n", Buffer);
                EHPrint("  \e9E9E9E%04x\eAABBCC ", Iterate);
                Display->SetBuffer(SBIdx);
            }
            EHPrint(" \e4287f5%02x\eAABBCC", AddressChar[Iterate]);
            if ((AddressChar[Iterate] < 0x20) || (AddressChar[Iterate] > 0x7e))
                Buffer[Iterate % 16] = '.';
            else
                Buffer[Iterate % 16] = AddressChar[Iterate];
            Buffer[(Iterate % 16) + 1] = '\0';
        }

        while ((Iterate % 16) != 0)
        {
            EHPrint("   ");
            Display->SetBuffer(SBIdx);
            Iterate++;
        }

        EHPrint("  \e8A78FF%s\eAABBCC\n", Buffer);
        EHPrint("-------------------------------------------------------------------------\n\n.");
        Display->SetBuffer(SBIdx);
    }

    SafeFunction char *TrimWhiteSpace(char *str)
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

    SafeFunction void DisplayTopOverlay()
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

        /* https://imgflip.com/i/77slbl */
        if ((Random::rand32() % 100) >= 98)
        {
            debug("Easter egg activated!");
            int BaseXOffset = sb->Width - 14;
            int BaseYOffset = 8;
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 0, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 0, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 0, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 7, BaseYOffset + 0, 0x21852E, SBIdx);

            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 7, BaseYOffset + 1, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 8, BaseYOffset + 1, 0x21852E, SBIdx);

            Display->SetPixel(BaseXOffset + 1, BaseYOffset + 2, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 2, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 2, 0xFFFFFF, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 2, 0x000000, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 2, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 2, 0xFFFFFF, SBIdx);
            Display->SetPixel(BaseXOffset + 7, BaseYOffset + 2, 0x000000, SBIdx);
            Display->SetPixel(BaseXOffset + 8, BaseYOffset + 2, 0x21852E, SBIdx);

            Display->SetPixel(BaseXOffset + 1, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 3, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 7, BaseYOffset + 3, 0x21852E, SBIdx);

            Display->SetPixel(BaseXOffset + 0, BaseYOffset + 4, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 1, BaseYOffset + 4, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 4, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 4, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 4, 0xA84832, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 4, 0xA84832, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 4, 0xA84832, SBIdx);
            Display->SetPixel(BaseXOffset + 7, BaseYOffset + 4, 0xA84832, SBIdx);

            Display->SetPixel(BaseXOffset + 0, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 1, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 5, 0x21852E, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 5, 0x21852E, SBIdx);

            Display->SetPixel(BaseXOffset + 0, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 1, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 2, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 3, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 4, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 5, BaseYOffset + 6, 0x1216FF, SBIdx);
            Display->SetPixel(BaseXOffset + 6, BaseYOffset + 6, 0x1216FF, SBIdx);

            Display->SetBuffer(SBIdx);
        }
    }

    SafeFunction void DisplayBottomOverlay()
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

    SafeFunction void ArrowInput(uint8_t key)
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
            if (SBIdx > 251)
                SBIdx--;
            else
                return;
            break;
        case KEY_D_DOWN:
            if (SBIdx > 251)
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

    SafeFunction void UserInput(char *Input)
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
            EHPrint("showbuf,sb <INDEX> - Display the contents of a screen buffer.\n");
            EHPrint("       - A sleep timer will be enabled. This will cause the OS to sleep for an unknown amount of time.\n");
            EHPrint("       - \eFF4400WARNING: This can crash the system if a wrong buffer is selected.\eFAFAFA\n");
            EHPrint("ifr <COUNT> - Show interrupt frames.\n");
            EHPrint("tlb <ADDRESS> - Print the page table entries\n");
            EHPrint("bitmap - Print the memory bitmap\n");
            EHPrint("mem - Print the memory allocation\n");
            EHPrint("cr<INDEX> - Print the CPU control register\n");
            EHPrint("tss <CORE> - Print the CPU task state segment\n");
            EHPrint("dump <ADDRESS HEX> <LENGTH DEC> - Dump memory\n");
            EHPrint("       - \eFF4400WARNING: This can crash the system if you try to read from an unmapped page.\eFAFAFA\n");
            EHPrint("uartmemdmp <INDEX> <SKIP INACCESSIBLE (bool 0,1)> - Dump the memory of a UART.\n");
            EHPrint("main - Show the main screen.\n");
            EHPrint("details - Show the details screen.\n");
            EHPrint("frames - Show the stack frame screen.\n");
            EHPrint("tasks - Show the tasks screen.\n");
            EHPrint("console - Show the console screen.\n");
            EHPrint("Also, you can use the arrow keys to navigate between the screens.\n");
            EHPrint("=========================================================================\n");
            EHPrint("Kernel Compiled at: %s %s with C++ Standard: %d\n", __DATE__, __TIME__, CPP_LANGUAGE_STANDARD);
            EHPrint("C++ Language Version (__cplusplus): %ld\n", __cplusplus);
        }
        else if (strcmp(Input, "exit") == 0)
        {
            PowerManager->Shutdown();
            EHPrint("\eFFFFFFNow it's safe to turn off your computer.");
            Display->SetBuffer(SBIdx);
            CPU::Stop();
        }
        else if (strcmp(Input, "reboot") == 0)
        {
            PowerManager->Reboot();
            EHPrint("\eFFFFFFNow it's safe to reboot your computer.");
            Display->SetBuffer(SBIdx);
            CPU::Stop();
        }
        else if (strncmp(Input, "showbuf", 7) == 0 || strncmp(Input, "sb", 2) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 7);
            int tmpidx = SBIdx;
            SBIdx = atoi(arg);
            Display->SetBuffer(SBIdx);
#if defined(a86)
            for (int i = 0; i < 5000000; i++)
                inb(0x80);
#endif // a64 || a32
            SBIdx = tmpidx;
            Display->SetBuffer(SBIdx);
        }
        else if (strncmp(Input, "ifr", 3) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 3);
            int CountI = atoi(arg);
            int TotalCount = sizeof(EHIntFrames) / sizeof(EHIntFrames[0]);

            debug("Printing %ld interrupt frames.", CountI);

            if (CountI > TotalCount)
            {
                EHPrint("\eFF4400Count too big! Maximum allowed is %ld\eFAFAFA\n", TotalCount);
                Display->SetBuffer(SBIdx);
            }
            else
            {
                for (int i = 0; i < CountI; i++)
                {
                    if (EHIntFrames[i])
                    {
                        if (!Memory::Virtual().Check(EHIntFrames[i]))
                            continue;
                        EHPrint("\n\e2565CC%p", EHIntFrames[i]);
                        EHPrint("\e7925CC-");
#if defined(a64)
                        if ((uintptr_t)EHIntFrames[i] >= 0xFFFFFFFF80000000 && (uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#elif defined(a32)
                        if ((uintptr_t)EHIntFrames[i] >= 0xC0000000 && (uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#elif defined(aa64)
                        if ((uintptr_t)EHIntFrames[i] >= 0xFFFFFFFF80000000 && (uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#endif
                            EHPrint("\e25CCC9%s", KernelSymbolTable->GetSymbolFromAddress((uintptr_t)EHIntFrames[i]));
                        else
                            EHPrint("\eFF4CA9Outside Kernel");
#if defined(a86)
                        for (int i = 0; i < 20000; i++)
                            inb(0x80);
#endif // a64 || a32
                        Display->SetBuffer(SBIdx);
                    }
                }
            }
        }
        else if (strncmp(Input, "tlb", 3) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 3);
            uintptr_t Address = NULL;
            Address = strtol(arg, NULL, 16);
            debug("Converted %s to %#lx", arg, Address);
            Memory::PageTable4 *BasePageTable = (Memory::PageTable4 *)Address;
            if (Memory::Virtual().Check(BasePageTable))
            {
                for (int PMLIndex = 0; PMLIndex < 512; PMLIndex++)
                {
                    Memory::PageMapLevel4 PML4 = BasePageTable->Entries[PMLIndex];
                    EHPrint("\e888888# \eAABBCC%03d-%03d-%03d-%03d\e4500F5: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:\e888888%#lx\n",
                            PMLIndex, 0, 0, 0,
                            PML4.Present ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.ReadWrite ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.UserSupervisor ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.WriteThrough ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.CacheDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.Accessed ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.ExecuteDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                            PML4.GetAddress() << 12);
                    Display->SetBuffer(SBIdx);
                    if (PML4.Present)
                    {
                        Memory::PageDirectoryPointerTableEntryPtr *PDPTE = (Memory::PageDirectoryPointerTableEntryPtr *)((uintptr_t)PML4.GetAddress() << 12);
                        if (PDPTE)
                        {
                            for (int PDPTEIndex = 0; PDPTEIndex < 512; PDPTEIndex++)
                            {
                                EHPrint("\e888888# \eAABBCC%03d-%03d-%03d-%03d\e4500F5: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:\e888888%#lx\n",
                                        PMLIndex, PDPTEIndex, 0, 0,
                                        PDPTE->Entries[PDPTEIndex].Present ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].ReadWrite ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].UserSupervisor ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].WriteThrough ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].CacheDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].Accessed ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].ExecuteDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                        PDPTE->Entries[PDPTEIndex].GetAddress() << 12);
                                Display->SetBuffer(SBIdx);
                                if ((PDPTE->Entries[PDPTEIndex].Present))
                                {
                                    Memory::PageDirectoryEntryPtr *PDE = (Memory::PageDirectoryEntryPtr *)((uintptr_t)PDPTE->Entries[PDPTEIndex].GetAddress() << 12);
                                    if (PDE)
                                    {
                                        for (int PDEIndex = 0; PDEIndex < 512; PDEIndex++)
                                        {
                                            EHPrint("\e888888# \eAABBCC%03d-%03d-%03d-%03d\e4500F5: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s NX:%s Address:\e888888%#lx\n",
                                                    PMLIndex, PDPTEIndex, PDEIndex, 0,
                                                    PDE->Entries[PDEIndex].Present ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].ReadWrite ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].UserSupervisor ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].WriteThrough ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].CacheDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].Accessed ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].ExecuteDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                    PDE->Entries[PDEIndex].GetAddress() << 12);
                                            Display->SetBuffer(SBIdx);
                                            if ((PDE->Entries[PDEIndex].Present))
                                            {
                                                Memory::PageTableEntryPtr *PTE = (Memory::PageTableEntryPtr *)((uintptr_t)PDE->Entries[PDEIndex].GetAddress() << 12);
                                                if (PTE)
                                                {
                                                    for (int PTEIndex = 0; PTEIndex < 512; PTEIndex++)
                                                    {
                                                        EHPrint("\e888888# \eAABBCC%03d-%03d-%03d-%03d\e4500F5: P:%s RW:%s US:%s PWT:%s PCB:%s A:%s D:%s PAT:%s G:%s PK:%d NX:%s Address:\e888888%#lx\n",
                                                                PMLIndex, PDPTEIndex, PDEIndex, PTEIndex,
                                                                PTE->Entries[PTEIndex].Present ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].ReadWrite ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].UserSupervisor ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].WriteThrough ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].CacheDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].Accessed ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].Dirty ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].PageAttributeTable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].Global ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].ProtectionKey,
                                                                PTE->Entries[PTEIndex].ExecuteDisable ? "\e00FF001\e4500F5" : "\eFF00000\e4500F5",
                                                                PTE->Entries[PTEIndex].GetAddress() << 12);
                                                        Display->SetBuffer(SBIdx);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (strncmp(Input, "bitmap", 6) == 0)
        {
            Bitmap bm = KernelAllocator.GetPageBitmap();

            EHPrint("\n\eFAFAFA[0%%] %08ld: ", 0);
            for (size_t i = 0; i < bm.Size; i++)
            {
                if (bm.Get(i))
                    EHPrint("\eFF00001");
                else
                    EHPrint("\e00FF000");
                if (i % 128 == 127)
                {
                    short Percentage = s_cst(short, (i * 100) / bm.Size);
                    EHPrint("\n\eFAFAFA[%03ld%%] %08ld: ", Percentage, i);
                    Display->SetBuffer(SBIdx);
                }
            }
            EHPrint("\n\e22AA44--- END OF BITMAP ---\nBitmap size: %ld\n\n.", bm.Size);
            Display->SetBuffer(SBIdx);
        }
        else if (strcmp(Input, "mem") == 0)
        {
            uint64_t Total = KernelAllocator.GetTotalMemory();
            uint64_t Used = KernelAllocator.GetUsedMemory();
            uint64_t Free = KernelAllocator.GetFreeMemory();
            uint64_t Reserved = KernelAllocator.GetReservedMemory();

            EHPrint("\e22AA44Total: %ld bytes\n\eFF0000Used: %ld bytes\n\e00FF00Free: %ld bytes\n\eFF00FFReserved: %ld bytes\n", Total, Used, Free, Reserved);
            int Progress = s_cst(int, (Used * 100) / Total);
            int ReservedProgress = s_cst(int, (Reserved * 100) / Total);
            EHPrint("\e22AA44%3d%% \eCCCCCC[", Progress);
            for (int i = 0; i < Progress; i++)
                EHPrint("\eFF0000|");
            for (int i = 0; i < 100 - Progress; i++)
                EHPrint("\e00FF00|");
            for (int i = 0; i < ReservedProgress; i++)
                EHPrint("\eFF00FF|");
            EHPrint("\eCCCCCC]\n");

            Display->SetBuffer(SBIdx);
        }
        else if (strncmp(Input, "cr", 2) == 0)
        {
            char *cr = TrimWhiteSpace(Input + 2);
            switch (cr[0])
            {
            case '0':
            {
#if defined(a64)
                EHPrint("\e44AA000: %#lx\n", CPU::x64::readcr0());
#elif defined(a32)
                EHPrint("\e44AA000: %#lx\n", CPU::x32::readcr0());
#endif
                break;
            }
            case '2':
            {
#if defined(a64)
                EHPrint("\e44AA002: %#lx\n", PageFaultAddress);
#elif defined(a32)
                EHPrint("\e44AA002: %#lx\n", CPU::x32::readcr2());
#endif
                break;
            }
            case '3':
            {
#if defined(a64)
                EHPrint("\e44AA003: %#lx\n", CPU::x64::readcr3());
#elif defined(a32)
                EHPrint("\e44AA003: %#lx\n", CPU::x32::readcr3());
#endif
                break;
            }
            case '4':
            {
#if defined(a64)
                EHPrint("\e44AA004: %#lx\n", CPU::x64::readcr4());
#elif defined(a32)
                EHPrint("\e44AA004: %#lx\n", CPU::x32::readcr4());
#endif
                break;
            }
            case '8':
            {
#if defined(a64)
                EHPrint("\e44AA008: %#lx\n", CPU::x64::readcr8());
#elif defined(a32)
                EHPrint("\e44AA008: %#lx\n", CPU::x32::readcr8());
#endif
                break;
            }
            default:
                EHPrint("\eFF0000Invalid CR\n");
                break;
            }
        }
        else if (strncmp(Input, "tss", 3) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 3);
            int TSSIndex = atoi(arg);
            if (TSSIndex > SMP::CPUCores)
            {
                EHPrint("\eFF0000Invalid TSS index\n");
            }
            else
            {
#if defined(a86)
                GlobalDescriptorTable::TaskStateSegment tss = GlobalDescriptorTable::tss[TSSIndex];
                EHPrint("\eFAFAFAStack Pointer 0: \eAABB22%#lx\n", tss.StackPointer[0]);
                EHPrint("\eFAFAFAStack Pointer 1: \eAABB22%#lx\n", tss.StackPointer[1]);
                EHPrint("\eFAFAFAStack Pointer 2: \eAABB22%#lx\n", tss.StackPointer[2]);

                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[0]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[1]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[2]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[3]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[4]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[5]);
                EHPrint("\eFAFAFAInterrupt Stack Table: \eAABB22%#lx\n", tss.InterruptStackTable[6]);

                EHPrint("\eFAFAFAI/O Map Base Address Offset: \eAABB22%#lx\n", tss.IOMapBaseAddressOffset);

                EHPrint("\eFAFAFAReserved 0: \eAABB22%#lx\n", tss.Reserved0);
                EHPrint("\eFAFAFAReserved 1: \eAABB22%#lx\n", tss.Reserved1);
                EHPrint("\eFAFAFAReserved 2: \eAABB22%#lx\n", tss.Reserved2);
#elif defined(aa64)
                EHPrint("\eFF0000AArch64 does not have TSS\n");
#endif
            }
        }
        else if (strncmp(Input, "dump", 4) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 4);
            char *addr = strtok(arg, " ");
            char *len = strtok(NULL, " ");
            if (addr == NULL || len == NULL)
            {
                EHPrint("\eFF0000Invalid arguments\n");
            }
            else
            {
                uint64_t Address = strtoul(addr, NULL, 16);
                uint64_t Length = strtoul(len, NULL, 10);
                debug("Dumping %ld bytes from %#lx\n", Length, Address);
                EHDumpData((void *)Address, Length);
            }
        }
        else if (strncmp(Input, "uartmemdmp", 10) == 0)
        {
            char *arg = TrimWhiteSpace(Input + 10);
            char *cPort = strtok(arg, " ");
            char *cBoolSkip = strtok(NULL, " ");
            UniversalAsynchronousReceiverTransmitter::SerialPorts port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM1;
            switch (cPort[0])
            {
            case '1':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM1;
                break;
            case '2':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM2;
                break;
            case '3':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM3;
                break;
            case '4':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM4;
                break;
            case '5':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM5;
                break;
            case '6':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM6;
                break;
            case '7':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM7;
                break;
            case '8':
                port = UniversalAsynchronousReceiverTransmitter::SerialPorts::COM8;
                break;
            default:
                EHPrint("\eFF0000Invalid port! Defaulting to 1.\n");
                break;
            }
            EHPrint("\eF8F8F8Dumping memory to UART port %c (%#lx) and %s inaccessible pages.\n", cPort[0], port, cBoolSkip[0] == '1' ? "skipping" : "zeroing");
            Display->SetBuffer(SBIdx);
            uint64_t Length = KernelAllocator.GetTotalMemory();
            uint64_t ProgressLength = Length;
            UniversalAsynchronousReceiverTransmitter::UART uart(port);
            Memory::Virtual vma;
            uint8_t *Address = reinterpret_cast<uint8_t *>(0x0);
            int Progress = 0;
            for (size_t i = 0; i < Length; i++)
            {
                if (vma.Check(Address))
                    uart.Write(*Address);
                else if (cBoolSkip[0] == '0')
                    uart.Write((uint8_t)0);
                else
                    ProgressLength--;
                Address++;

                if (unlikely(i % 0x1000 == 0))
                {
                    int NewProgress = (int)((i * 100) / ProgressLength);
                    if (unlikely(NewProgress != Progress))
                    {
                        Progress = NewProgress;
                        EHPrint("\n%d%%\n", Progress);
                        Display->SetBuffer(SBIdx);
                    }
                    Display->Print('.', SBIdx);
                    if (unlikely(i % 0x500 == 0))
                        Display->SetBuffer(SBIdx);
                }
            }
            EHPrint("\nDone.\n");
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
        else if (strlen(Input) > 0)
            EHPrint("Unknown command: %s", Input);

        DisplayBottomOverlay();
        Display->SetBuffer(SBIdx);
    }

    SafeFunction void StopAllCores()
    {
#if defined(a86)
        /* FIXME: Can't send IPIs to other cores
         * because it causes another exception on
         * the other cores.
         *
         * Also it makes every core to stay at 100% usage for some reason.
         */

        // if (SMP::CPUCores > 1)
        // {
        //     for (int i = 1; i < SMP::CPUCores; i++)
        //     {
        //         APIC::InterruptCommandRegisterLow icr;
        //         icr.Vector = CPU::x86::IRQ29;
        //         icr.Level = APIC::APICLevel::Assert;
        //         ((APIC::APIC *)Interrupts::apic[i])->IPI(i, icr);
        //         __sync;
        //     }
        // }
        // APIC::InterruptCommandRegisterLow icr;
        // icr.Vector = CPU::x86::IRQ29;
        // icr.Level = APIC::APICLevel::Assert;
        // icr.DestinationShorthand = APIC::APICDestinationShorthand::AllExcludingSelf;
        // ((APIC::APIC *)Interrupts::apic[0])->IPI(0, icr);
        // CPU::Interrupts(CPU::Enable);
        __sync;
        CPU::Interrupts(CPU::Disable);
        // }
#elif defined(aa64)
#endif
    }

    SafeFunction void Handle(void *Data)
    {
        // TODO: SUPPORT SMP
        CPU::Interrupts(CPU::Disable);
        SBIdx = 255;
        CHArchTrapFrame *Frame = (CHArchTrapFrame *)Data;
#if defined(a64)
        debug("-----------------------------------------------------------------------------------");
        error("Exception: %#llx", Frame->InterruptNumber);
        for (size_t i = 0; i < INT_FRAMES_MAX; i++)
            EHIntFrames[i] = Interrupts::InterruptFrames[i];
        PageFaultAddress = CPU::x64::readcr2().PFLA;

        if (Frame->cs != GDT_USER_CODE && Frame->cs != GDT_USER_DATA)
        {
            if (PageFaultAddress)
            {
                debug("Exception in kernel mode (ip: %#lx cr2: %#lx (%s))",
                      Frame->rip, PageFaultAddress, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress(Frame->rip) : "No symbol");
            }
            else
            {
                debug("Exception in kernel mode (ip: %#lx (%s))",
                      Frame->rip, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress(Frame->rip) : "No symbol");
            }

            CPUData *data = GetCurrentCPU();
            if (data)
            {
                if (data->CurrentThread)
                {
                    if (!data->CurrentThread->Security.IsCritical)
                    {
                        fixme("Exception in non-critical thread (kernel mode)");
                    }
                }
            }

            if (TaskManager)
                TaskManager->Panic();
            ForceUnlock = true;
            Display->CreateBuffer(0, 0, SBIdx);
            StopAllCores();
        }
        else
        {
            if (PageFaultAddress)
            {
                debug("Exception in user mode (ip: %#lx cr2: %#lx (%s))",
                      Frame->rip, PageFaultAddress, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress(Frame->rip) : "No symbol");
            }
            else
            {
                debug("Exception in user mode (ip: %#lx (%s))",
                      Frame->rip, KernelSymbolTable ? KernelSymbolTable->GetSymbolFromAddress(Frame->rip) : "No symbol");
            }
            CPUData *data = GetCurrentCPU();
            if (!data)
            {
                ForceUnlock = true;
                Display->CreateBuffer(0, 0, SBIdx);
                StopAllCores();
                EHPrint("\eFF0000Cannot get CPU data! This results in a kernel crash!");
                error("Cannot get CPU data! This results in a kernel crash!");
                error("This should never happen!");
            }
            else
            {
                debug("CPU %ld data is valid", data->ID);
                if (data->CurrentThread->Security.IsCritical)
                {
                    debug("Critical thread \"%s\"(%d) died", data->CurrentThread->Name, data->CurrentThread->ID);
                    if (TaskManager)
                        TaskManager->Panic();
                    ForceUnlock = true;
                    Display->CreateBuffer(0, 0, SBIdx);
                    StopAllCores();
                }
                else
                {
                    debug("Current thread is valid %#lx", data->CurrentThread);
                    UserModeExceptionHandler(Frame);
                    return;
                }
            }
        }

        if (ExceptionOccurred)
        {
            SBIdx = 255;
            Display->ClearBuffer(SBIdx);
            Display->SetBufferCursor(SBIdx, 0, 0);

            CPU::x64::CR0 cr0 = CPU::x64::readcr0();
            CPU::x64::CR2 cr2 = CPU::x64::CR2{.PFLA = PageFaultAddress};
            CPU::x64::CR3 cr3 = CPU::x64::readcr3();
            CPU::x64::CR4 cr4 = CPU::x64::readcr4();
            CPU::x64::CR8 cr8 = CPU::x64::readcr8();
            CPU::x64::EFER efer;
            efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);
            uintptr_t ds;
            asmv("mov %%ds, %0"
                 : "=r"(ds));

            EHPrint("\eFF2525FS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx\n",
                    CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                    Frame->ss, Frame->cs, ds);
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
            Interrupts::RemoveAll();
            CPU::Stop();
        }

        ExceptionOccurred = true;

        if (DriverManager)
            DriverManager->Panic();

        debug("Reading control registers...");
        crashdata.Frame = Frame;
        crashdata.cr0 = CPU::x64::readcr0();
        crashdata.cr2 = CPU::x64::CR2{.PFLA = PageFaultAddress};
        crashdata.cr3 = CPU::x64::readcr3();
        crashdata.cr4 = CPU::x64::readcr4();
        crashdata.cr8 = CPU::x64::readcr8();
        crashdata.efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);
        uintptr_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));

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
            crashdata.CPUData = cpudata;
            error("Technical Informations on CPU %lld:", cpudata->ID);
        }

        if (TaskManager && cpudata != nullptr)
        {
            crashdata.Process = cpudata->CurrentProcess.load();
            crashdata.Thread = cpudata->CurrentThread.load();

            error("Current Process: %s(%ld)",
                  cpudata->CurrentProcess->Name,
                  cpudata->CurrentProcess->ID);
            error("Current Thread: %s(%ld)",
                  cpudata->CurrentThread->Name,
                  cpudata->CurrentThread->ID);
        }

        {
            error("FS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx",
                  CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                  Frame->ss, Frame->cs, ds);
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

#elif defined(a32)
        goto CrashEnd;
#elif defined(aa64)
        goto CrashEnd;
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
