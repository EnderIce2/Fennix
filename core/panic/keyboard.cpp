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

#include "keyboard.hpp"

#include <display.hpp>
#include <convert.h>
#include <printf.h>
#include <kcon.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

using namespace KernelConsole;

#define ERROR_COLOR "\x1b[31m"
#define WARN_COLOR "\x1b[33m"
#define DEFAULT_COLOR "\x1b[0m"

extern void ExPrint(const char *Format, ...);
extern void ArrowInput(uint8_t key);
extern void UserInput(char *Input);
extern FontRenderer CrashFontRenderer;

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

nsa static inline int GetLetterFromScanCode(uint8_t ScanCode)
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

nsa void CrashKeyboardDriver::PS2Wait(bool Read)
{
	TimeoutCallNumber++;
#if defined(a86)
	int Timeout = 100000;
	uint8_t Status = 0;
	while (Timeout--)
	{
		Status = inb(0x64);
		if (Read)
		{
			if ((Status & 1) == 1)
				return;
		}
		else
		{
			if ((Status & 2) == 0)
				return;
		}
	}
	ExPrint(WARN_COLOR "PS/2 controller timeout (%s;%d)\n" DEFAULT_COLOR,
			Read ? "read" : "write", TimeoutCallNumber - 1);
#endif // a86
}

/*
	This simple driver relies on the PS/2 controller to handle the keyboard.

	Maybe is not the most efficient way but most x86 devices out there
	still has PS/2 support (emulated or not).

	We even have to make sure IRQ1 is enabled in the PIC but on x64 we use
	the I/O APIC... "outb(0x21, 0b11111101);" can be used but the EOI is
	automatically sent to I/O APIC if enabled/supported which is bad.

	FIXME: On some real devices, the PS/2 keyboard doesn't send interrupts.

	TODO: Implement a way to handle USB keyboards in the future.
*/
CrashKeyboardDriver::CrashKeyboardDriver() : Interrupts::Handler(1) /* IRQ1 */
{
#define WaitRead PS2Wait(true)
#define WaitWrite PS2Wait(false)
#define SetMessageLocation \
	ExPrint("\x1b[%d;%dH", (Display->GetWidth / CrashFontRenderer.CurrentFont->GetInfo().Width) - 1, 0);

	CPU::Interrupts(CPU::Disable);

	/* Dots will be printed at the bottom of the screen as a progress bar. */
	ExPrint("\x1b[%d;%dH", (Display->GetWidth / CrashFontRenderer.CurrentFont->GetInfo().Width) - 2, 0);
#if defined(a86)

	/* Disable port 1 & 2 */
	{
		/* Disable Port 1 */
		WaitWrite;
		outb(0x64, 0xAD);

		/* Disable Port 2 */
		WaitWrite;
		outb(0x64, 0xA7);
	}
	ExPrint(".");

	/* Flush */
	{
		int Timeout = 100000;
		while ((inb(0x64) & 1) && Timeout-- > 0)
			inb(0x60);

		if (Timeout <= 0)
		{
			SetMessageLocation;
			ExPrint(ERROR_COLOR
					"PS/2 controller timeout (flush;0)\n" DEFAULT_COLOR);
			CPU::Stop();
		}
	}

	ExPrint(".");
	/* Test controller */
	{
		/* Save config */
		WaitWrite;
		outb(0x64, 0x20);
		WaitRead;
		uint8_t cfg = inb(0x60);

		/* Test PS/2 controller */
		WaitWrite;
		outb(0x64, 0xAA);
		WaitRead;
		uint8_t test = inb(0x60);
		if (test != 0x55)
		{
			if (test == 0xFA)
			{
				trace("PS/2 controller sent ACK to test request.");

				WaitRead;
				test = inb(0x60);
			}

			if (test != 0x55)
			{
				SetMessageLocation;
				ExPrint(ERROR_COLOR
						"PS/2 controller self test failed (%#x)\n" DEFAULT_COLOR,
						test);
				CPU::Stop();
			}
		}

		/* Restore config */
		WaitWrite;
		outb(0x64, 0x60);
		WaitWrite;
		outb(0x60, cfg);
	}
	ExPrint(".");

	/* Disable scanning; Enable port 1; Set default settings */
	{
		/* Disable scanning */
		outb(0x60, 0xF5);

		/* Enable Port 1 */
		WaitWrite;
		outb(0x64, 0xAE);

		/* Set default settings */
		outb(0x60, 0xF6);
	}
	ExPrint(".");

	/* Test port 1 */
	{
		WaitWrite;
		outb(0x64, 0xAB);
		WaitRead;
		uint8_t test = inb(0x60);

		if (test != 0x00)
		{
			if (test == 0xFA)
			{
				trace("PS/2 keyboard sent ACK to test request.");

				WaitRead;
				test = inb(0x60);
			}

			if (test != 0x00)
			{
				SetMessageLocation;
				ExPrint(ERROR_COLOR
						"PS/2 keyboard self test failed (%#x)\n" DEFAULT_COLOR,
						test);
				CPU::Stop();
			}
		}
	}
	ExPrint(".");

	/* Configure the controller */
	{
		/* Read Controller Configuration */
		WaitWrite;
		outb(0x64, 0x20);
		WaitRead;
		uint8_t cfg = inb(0x60);

		/* Enable Port 1 & Port 1 translation */
		cfg |= 0b01000001;

		/* Write Controller Configuration */
		WaitWrite;
		outb(0x64, 0x60);
		WaitWrite;
		outb(0x60, cfg);
	}
	ExPrint(".");

	/* Enable port 1; Set scan code; Enable scanning */
	{
		/* Enable Port 1 */
		outb(0x64, 0xAE);

		/* Set scan code set 1 */
		WaitWrite;
		outb(0x60, 0xF0);
		WaitWrite;
		outb(0x60, 0x02);

		/* Check if we have scan code set 1 */
		WaitWrite;
		outb(0x60, 0xF0);
		WaitWrite;
		outb(0x60, 0x00);

		/* Read scan code set */
		WaitRead;
		uint8_t scs = inb(0x60);
		if (scs == 0xFA || scs == 0xFE)
		{
			if (scs == 0xFA)
				trace("PS/2 keyboard sent ACK to scan code set request.");
			if (scs == 0xFE)
				trace("PS/2 keyboard sent RESEND to scan code set request.");

			WaitRead;
			scs = inb(0x60);
		}

		if (scs != 0x41)
		{
			SetMessageLocation;
			ExPrint(WARN_COLOR
					"PS/2 keyboard scan code set 1 not supported (%#x)\n" DEFAULT_COLOR,
					scs);
		}

		/* Enable scanning */
		outb(0x60, 0xF4);
	}
	ExPrint(".");

#endif // defined(a86)

	CPU::Interrupts(CPU::Enable);
}

nsa void CrashKeyboardDriver::OnInterruptReceived(CPU::TrapFrame *Frame)
{
#if defined(a86)
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
				char keyBuf[5] = {'\b', '\x1b', '[', 'K', '\0'};
				ExPrint(keyBuf);
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
			char keyBuf[2] = {(char)key, '\0'};
			ExPrint(keyBuf);
			BackSpaceLimit++;
		}
		Display->UpdateBuffer(); /* Update as we type. */
	}
#endif // a64 || a32
}
