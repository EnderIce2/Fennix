#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
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

static inline void backspace(char s[])
{
    int len = strlen(s);
    s[len - 1] = '\0';
}

static inline void append(char s[], char n)
{
    int len = strlen(s);
    s[len] = n;
    s[len + 1] = '\0';
}

namespace CrashHandler
{
    CrashKeyboardDriver::CrashKeyboardDriver() : Interrupts::Handler(CPU::x64::IRQ1)
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

#if defined(__amd64__)
    __no_stack_protector void CrashKeyboardDriver::OnInterruptReceived(CPU::x64::TrapFrame *Frame)
#elif defined(__i386__)
    __no_stack_protector void CrashKeyboardDriver::OnInterruptReceived(void *Frame)
#elif defined(__aarch64__)
    __no_stack_protector void CrashKeyboardDriver::OnInterruptReceived(void *Frame)
#endif
    {
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
                append(UserInputBuffer, key);
                Display->Print(key, SBIdx);
                BackSpaceLimit++;
            }
            Display->SetBuffer(SBIdx); // Update as we type.
        }
    }

    __no_stack_protector void HookKeyboard()
    {
        CrashKeyboardDriver kbd; // We don't want to allocate memory.
        asmv("Loop: nop; jmp Loop;");
        // CPU::Halt(true); // This is an infinite loop.
    }
}
