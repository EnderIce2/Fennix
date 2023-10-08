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

#include <limits.h>
#include <debug.h>
#include <io.h>

#include "../../mapi.hpp"
#include "../mod.hpp"
#include "../../kernel.h"

namespace PS2Keyboard
{
	KernelAPI KAPI;

	uint8_t ScanCode = 0;
	bool InputReceived = false;

	void PS2Wait(bool Read)
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

	int DriverEntry(void *Data)
	{
		if (!Data)
			return INVALID_KERNEL_API;
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;

		return OK;
	}

	int CallbackHandler(KernelCallback *Data)
	{
		switch (Data->Reason)
		{
		case AcknowledgeReason:
		{
			debug("Kernel acknowledged the driver.");
			break;
		}
		case ConfigurationReason:
		{
#define WaitRead PS2Wait(true)
#define WaitWrite PS2Wait(false)

			WaitWrite;
			outb(0x64, 0xAD);
			WaitWrite;
			outb(0x64, 0xA7);

			WaitRead;
			inb(0x60);

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

			trace("PS/2 keyboard configured.");
			break;
		}
		case QueryReason:
		{
			Data->InputCallback.Keyboard.Key = ScanCode;
			break;
		}
		case PollWaitReason:
		{
			while (!InputReceived)
				TaskManager->Yield();
			InputReceived = false;

			Data->InputCallback.Keyboard.Key = ScanCode;
			break;
		}
		case StopReason:
		{
			fixme("Module stopped.");
			break;
		}
		default:
		{
			warn("Unknown reason.");
			break;
		}
		}
		return OK;
	}

	int InterruptCallback(CPURegisters *)
	{
		ScanCode = inb(0x60);
		InputReceived = true;
		return OK;
	}
}
