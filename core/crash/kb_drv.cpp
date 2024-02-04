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
#include "../../arch/amd64/cpu/gdt.hpp"
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
	void CrashKeyboardDriver::PS2Wait(bool Read)
	{
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
	}

	CrashKeyboardDriver::CrashKeyboardDriver() : Interrupts::Handler(1) /* IRQ1 */
	{
#define WaitRead PS2Wait(true)
#define WaitWrite PS2Wait(false)
		CPU::Interrupts(CPU::Disable);
#if defined(a86)

		/* Disable Port 1 */
		WaitWrite;
		outb(0x64, 0xAD);

		/* Disable Port 2 */
		WaitWrite;
		outb(0x64, 0xA7);

		/* Flush */
		WaitRead;
		inb(0x60);

		/* Test PS/2 controller */
		WaitWrite;
		outb(0x64, 0xAA);
		WaitRead;
		uint8_t test = inb(0x60);
		if (test != 0x55)
		{
			if (test == 0xFA)
				warn("PS/2 controller acknowledged? (expected TEST_PASSED = 0x55)");
			else
			{
				error("PS/2 controller self test failed (%#x)", test);
				// CPU::Stop();
			}
		}

		/* Enable Port 1 */
		WaitWrite;
		outb(0x64, 0xAE);

		/* Reset Port 1 */
		WaitWrite;
		outb(0x64, 0xFF); /* This may break some keyboards? */

		/* Test Port 1 */
		WaitWrite;
		outb(0x64, 0xAB);
		WaitRead;
		test = inb(0x60);

		if (test != 0x00)
		{
			if (test == 0xFA)
				warn("PS/2 keyboard acknowledged? (expected TEST_PASSED = 0x00)");
			else
			{
				error("PS/2 keyboard self test failed (%#x)", test);
				// CPU::Stop();
			}
		}

		/* Disable Port 1 */
		WaitWrite;
		outb(0x64, 0xAD);

		/* Disable Port 2 */
		WaitWrite;
		outb(0x64, 0xA7);

		/* Flush Port 1 */
		WaitRead;
		inb(0x60);

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
		if (scs != 0x41)
		{
			warn("PS/2 keyboard scan code set 1 not supported (%#x)", scs);
		}
#endif // defined(a86)

		CPU::Interrupts(CPU::Enable);
	}

	CrashKeyboardDriver::~CrashKeyboardDriver()
	{
		error("CrashKeyboardDriver::~CrashKeyboardDriver() called");
	}

	int BackSpaceLimit = 0;
	static char UserInputBuffer[1024];

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
			Display->SetBuffer(SBIdx); /* Update as we type. */
		}
#endif // a64 || a32
	}
}
