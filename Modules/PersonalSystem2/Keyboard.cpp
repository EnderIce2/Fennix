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

#include "../../DAPI.hpp"
#include "../drv.hpp"
#include "../../kernel.h"

namespace PS2Keyboard
{
	KernelAPI KAPI;

	uint8_t ScanCode = 0;
	bool InputReceived = false;

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
			fixme("Driver stopped.");
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
