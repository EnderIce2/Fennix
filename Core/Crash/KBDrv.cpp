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
#include <convert.h>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

const char sc_ascii_low[] = {'?', '?', '1', '2', '3', '4', '5', '6',
                             '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
                             'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g',
                             'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
                             'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char sc_ascii_high[] = {'?', '?', '!', '@', '#', '$', '%', '^',
                              '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
                              'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G',
                              'H', 'J', 'K', 'L', ';', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V',
                              'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '};

static int LowerCase = true;

static inline int GetLetterFromScanCode(uint8_t ScanCode)
{
    if (ScanCode & 0x80)
    {
        switch (ScanCode)
        {
        case KEY_U_LSHIFT:
            LowerCase = true;
            return KEY_INVALID;
        case KEY_U_RSHIFT:
            LowerCase = true;
            return KEY_INVALID;
        default:
            return KEY_INVALID;
        }
    }
    else
    {
        switch (ScanCode)
        {
        case KEY_D_RETURN:
            return '\n';
        case KEY_D_LSHIFT:
            LowerCase = false;
            return KEY_INVALID;
        case KEY_D_RSHIFT:
            LowerCase = false;
            return KEY_INVALID;
        case KEY_D_BACKSPACE:
            return ScanCode;
        default:
        {
            if (ScanCode > 0x39)
                break;
            if (LowerCase)
                return sc_ascii_low[ScanCode];
            else
                return sc_ascii_high[ScanCode];
        }
        }
    }
    return KEY_INVALID;
}

namespace CrashHandler
{
    CrashKeyboardDriver::CrashKeyboardDriver() : Interrupts::Handler(1) /* IRQ1 */
    {
        while (inb(0x64) & 0x1)
            inb(0x60);

        outb(0x64, 0xAE);
        outb(0x64, 0x20);
        uint8_t ret = (inb(0x60) | 1) & ~0x10;
        outb(0x64, 0x60);
        outb(0x60, ret);
        outb(0x60, 0xF4);

        outb(0x21, 0xFD);
        outb(0xA1, 0xFF);
        CPU::Interrupts(CPU::Enable); // Just to be sure.
    }

    CrashKeyboardDriver::~CrashKeyboardDriver()
    {
        error("CrashKeyboardDriver::~CrashKeyboardDriver() called!");
    }

    int BackSpaceLimit = 0;
    static char UserInputBuffer[1024];

#if defined(a64)
    SafeFunction void CrashKeyboardDriver::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(a32)
    SafeFunction void CrashKeyboardDriver::OnInterruptReceived(CPU::x32::TrapFrame *Frame)
#elif defined(aa64)
    SafeFunction void CrashKeyboardDriver::OnInterruptReceived(void *Frame)
#endif
    {
        UNUSED(Frame);
        uint8_t scanCode = inb(0x60);
        if (scanCode == KEY_D_TAB ||
            scanCode == KEY_D_LCTRL ||
            scanCode == KEY_D_LALT ||
            scanCode == KEY_U_LCTRL ||
            scanCode == KEY_U_LALT)
            return;

        switch (scanCode)
        {
        case KEY_D_UP:
        case KEY_D_LEFT:
        case KEY_D_RIGHT:
        case KEY_D_DOWN:
            ArrowInput(scanCode);
            break;
        default:
            break;
        }

        int key = GetLetterFromScanCode(scanCode);
        if (key != KEY_INVALID)
        {
            if (key == KEY_D_BACKSPACE)
            {
                if (BackSpaceLimit > 0)
                {
                    Display->Print('\b', SBIdx);
                    backspace(UserInputBuffer);
                    BackSpaceLimit--;
                }
            }
            else if (key == '\n')
            {
                UserInput(UserInputBuffer);
                BackSpaceLimit = 0;
                UserInputBuffer[0] = '\0';
            }
            else
            {
                append(UserInputBuffer, s_cst(char, key));
                Display->Print((char)key, SBIdx);
                BackSpaceLimit++;
            }
            Display->SetBuffer(SBIdx); // Update as we type.
        }
    }

    SafeFunction void HookKeyboard()
    {
        CrashKeyboardDriver kbd; // We don't want to allocate memory.
        asmv("KeyboardHookLoop: nop; jmp KeyboardHookLoop;");
        // CPU::Halt(true); // This is an infinite loop.
    }
}
