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

#include <interface/aip.h>
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

nsa void CrashKeyboardDriver::PS2Wait(bool Output)
{
#if defined(a86)
	TimeoutCallNumber++;
	int timeout = 100000;
	PS2_STATUSES status = {.Raw = inb(PS2_STATUS)};
	while (timeout--)
	{
		if (!Output)
		{
			if (status.OutputBufferFull == 0)
				return;
		}
		else
		{
			if (status.InputBufferFull == 0)
				return;
		}
		status.Raw = inb(PS2_STATUS);
	}
	ExPrint(WARN_COLOR "PS/2 controller timeout (%s;%d)\n" DEFAULT_COLOR,
			Output ? "output" : "input", TimeoutCallNumber);
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
		outb(PS2_CMD, PS2_CMD_DISABLE_PORT_1);

		/* Disable Port 2 */
		WaitWrite;
		outb(PS2_CMD, PS2_CMD_DISABLE_PORT_2);
	}
	ExPrint(".");

	/* Flush */
	{
		PS2_STATUSES status;
		int timeout = 0x500;
		while (timeout--)
		{
			status.Raw = inb(PS2_STATUS);
			if (status.OutputBufferFull == 0)
				break;
			inb(PS2_DATA);
		}

		if (timeout <= 0)
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
		outb(PS2_CMD, PS2_CMD_READ_CONFIG);
		WaitRead;
		PS2_CONFIGURATION cfg = {.Raw = inb(PS2_DATA)};
		cfg.Port1Interrupt = 1;
		cfg.Port2Interrupt = 1;
		cfg.Port1Translation = 1;

		/* Update config */
		WaitWrite;
		outb(PS2_CMD, PS2_DATA);
		WaitWrite;
		outb(PS2_DATA, cfg.Raw);

		/* Test PS/2 controller */
		WaitWrite;
		outb(PS2_CMD, PS2_CMD_TEST_CONTROLLER);
		WaitRead;
		uint8_t test = inb(PS2_DATA);
		if (test != PS2_TEST_PASSED)
		{
			if (test == PS2_ACK)
			{
				trace("PS/2 controller sent ACK to test request.");

				WaitRead;
				test = inb(PS2_DATA);
			}

			if (test != PS2_TEST_PASSED)
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
		outb(PS2_CMD, PS2_DATA);
		WaitWrite;
		outb(PS2_DATA, cfg.Raw);
	}
	ExPrint(".");

	/* Disable scanning; Enable port 1; Set default settings */
	{
		/* Disable scanning */
		outb(PS2_DATA, PS2_KBD_CMD_DISABLE_SCANNING);

		/* Enable Port 1 */
		WaitWrite;
		outb(PS2_CMD, PS2_CMD_ENABLE_PORT_1);

		/* Set default settings */
		outb(PS2_DATA, PS2_KBD_CMD_DEFAULTS);
	}
	ExPrint(".");

	/* Test port 1 */
	{
		WaitWrite;
		outb(PS2_CMD, PS2_CMD_TEST_PORT_1);
		WaitRead;
		uint8_t test = inb(PS2_DATA);

		if (test != 0x00)
		{
			if (test == PS2_KBD_RESP_ACK)
			{
				trace("PS/2 keyboard sent ACK to test request.");

				WaitRead;
				test = inb(PS2_DATA);
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
		// /* Read Controller Configuration */
		// WaitWrite;
		// outb(PS2_CMD, PS2_CMD_READ_CONFIG);
		// WaitRead;
		// uint8_t cfg = inb(PS2_DATA);

		// /* Enable Port 1 & Port 1 translation */
		// cfg |= 0b01000001;

		// /* Write Controller Configuration */
		// WaitWrite;
		// outb(PS2_CMD, PS2_CMD_WRITE_CONFIG);
		// WaitWrite;
		// outb(PS2_DATA, cfg);
	}
	ExPrint(".");

	/* Enable port 1; Set scan code; Enable scanning */
	{
		/* Enable Port 1 */
		outb(PS2_CMD, PS2_CMD_ENABLE_PORT_1);

		/* Set scan code set 1 */
		WaitWrite;
		outb(PS2_DATA, PS2_KBD_CMD_SCAN_CODE_SET);
		WaitWrite;
		outb(PS2_DATA, PS2_KBD_SCAN_CODE_SET_2);

		/* Check if we have scan code set 1 */
		WaitWrite;
		outb(PS2_DATA, PS2_KBD_CMD_SCAN_CODE_SET);
		WaitWrite;
		outb(PS2_DATA, PS2_KBD_SCAN_CODE_GET_CURRENT);

		/* Read scan code set */
		WaitRead;
		uint8_t scs = inb(PS2_DATA);
		if (scs == PS2_KBD_RESP_ACK || scs == PS2_KBD_RESP_RESEND)
		{
			if (scs == PS2_KBD_RESP_ACK)
				trace("PS/2 keyboard sent ACK to scan code set request.");
			if (scs == PS2_KBD_RESP_RESEND)
				trace("PS/2 keyboard sent RESEND to scan code set request.");

			WaitRead;
			scs = inb(PS2_DATA);
		}

		if (scs != PS2_KBD_SC_SET_2)
		{
			SetMessageLocation;
			ExPrint(WARN_COLOR
					"PS/2 keyboard scan code set 1 not supported (%#x)\n" DEFAULT_COLOR,
					scs);
		}

		/* Enable scanning */
		outb(PS2_DATA, PS2_KBD_CMD_ENABLE_SCANNING);
	}

#ifdef DEBUG
	WaitWrite;
	outb(PS2_CMD, PS2_CMD_READ_CONFIG);
	WaitRead;
	PS2_CONFIGURATION cfg = {.Raw = inb(PS2_DATA)};
	debug("PS2 CONFIG:\nPort1int: %d\nPort2int: %d\nSysFlg: %d\nZ: %d\nP1clk: %d\nP2clk: %d\nP1trans: %d\nz: %d",
		  cfg.Port1Interrupt, cfg.Port2Interrupt, cfg.SystemFlag, cfg.Zero0, cfg.Port1Clock, cfg.Port2Clock, cfg.Port1Translation, cfg.Zero1);
#endif

	ExPrint(".");

#endif // defined(a86)

	CPU::Interrupts(CPU::Enable);
}

nsa void CrashKeyboardDriver::OnInterruptReceived(CPU::TrapFrame *Frame)
{
#if defined(a86)
	UNUSED(Frame);
	uint8_t scanCode = inb(PS2_DATA);

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
