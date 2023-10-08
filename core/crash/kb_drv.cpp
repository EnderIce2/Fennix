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
		// Disable devices
		WaitWrite;
		outb(0x64, 0xAD);
		WaitWrite;
		outb(0x64, 0xA7);

		// Flush buffer
		WaitRead;
		inb(0x60);

		// outb(0x64, 0xAE);

		// Configure devices
		WaitWrite;
		outb(0x64, 0x20);
		WaitRead;
		uint8_t cfg = inb(0x60);
		bool DualChannel = cfg & 0b00100000;
		if (DualChannel)
			trace("Dual channel PS/2 controller detected.");
		cfg |= 0b01000011;
		WaitWrite;
		outb(0x64, 0x60);
		WaitWrite;
		outb(0x60, cfg);

		WaitWrite;
		outb(0x64, 0xAA);
		WaitRead;
		uint8_t test = inb(0x60);
		if (test != 0x55)
		{
			error("PS/2 controller self test failed! (%#x)", test);
			printf("PS/2 controller self test failed! (%#x)\n", test);
			CPU::Stop();
		}

		WaitWrite;
		outb(0x64, 0x60);
		WaitWrite;
		outb(0x60, cfg);

		bool DCExists = false;
		if (DualChannel)
		{
			WaitWrite;
			outb(0x64, 0xAE);
			WaitWrite;
			outb(0x64, 0x20);
			WaitRead;
			cfg = inb(0x60);
			DCExists = !(cfg & 0b00100000);
			WaitWrite;
			outb(0x64, 0xAD);
			debug("DCExists: %d", DCExists);
		}

		WaitWrite;
		outb(0x64, 0xAB);
		WaitRead;
		test = inb(0x60);
		if (test != 0x00)
		{
			error("PS/2 keyboard self test failed! (%#x)", test);
			printf("PS/2 keyboard self test failed! (%#x)\n", test);
			CPU::Stop();
		}

		if (DCExists)
		{
			WaitWrite;
			outb(0x64, 0xA9);
			WaitRead;
			test = inb(0x60);
			if (test != 0x00)
			{
				error("PS/2 mouse self test failed! (%#x)", test);
				printf("PS/2 mouse self test failed! (%#x)\n", test);
				CPU::Stop();
			}
		}

		WaitWrite;
		outb(0x64, 0xAE);

		if (DCExists)
		{
			WaitWrite;
			outb(0x64, 0xA8);
		}

		WaitWrite;
		outb(0x60, 0xFF);
		WaitRead;
		test = inb(0x60);
		if (test == 0xFC)
		{
			error("PS/2 keyboard reset failed! (%#x)", test);
			printf("PS/2 keyboard reset failed! (%#x)\n", test);
			CPU::Stop();
		}

		WaitWrite;
		outb(0x60, 0xD4);
		WaitWrite;
		outb(0x60, 0xFF);
		WaitRead;
		test = inb(0x60);
		if (test == 0xFC)
		{
			error("PS/2 mouse reset failed! (%#x)", test);
			printf("PS/2 mouse reset failed! (%#x)\n", test);
			CPU::Stop();
		}

		// outb(0x60, 0xF4);

		// outb(0x21, 0xFD);
		// outb(0xA1, 0xFF);
#endif // defined(a86)

		CPU::Interrupts(CPU::Enable);
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
	SafeFunction void CrashKeyboardDriver::OnInterruptReceived(CPU::aarch64::TrapFrame *Frame)
#endif
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
