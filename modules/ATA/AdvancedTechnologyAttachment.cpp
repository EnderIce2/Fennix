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

#include "ata.hpp"

#include <debug.h>
#include <pci.hpp>
#include <io.h>

#include "../../mapi.hpp"
#include "../mod.hpp"

namespace AdvancedTechnologyAttachment
{
	KernelAPI KAPI;

	bool IsATAPresent()
	{
		outb(0x1F0 + 2, 0);
		outb(0x1F0 + 3, 0);
		outb(0x1F0 + 4, 0);
		outb(0x1F0 + 5, 0);
		outb(0x1F0 + 7, 0xEC);
		if (inb(0x1F0 + 7) == 0 || inb(0x1F0 + 1) != 0)
			return false;
		return true;
	}

	int DriverEntry(void *Data)
	{
		if (!Data)
			return INVALID_KERNEL_API;
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;

		if (!IsATAPresent())
			return NOT_AVAILABLE;
		trace("ATA device found.");

		return NOT_IMPLEMENTED;
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
			debug("Module received configuration data.");
			break;
		}
		case QueryReason:
		{
			break;
		}
		case SendReason:
		case ReceiveReason:
		{
			break;
		}
		case StopReason:
		{
			// TODO: Stop the driver.
			debug("Module stopped.");
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

	int InterruptCallback(CPURegisters *Registers)
	{
		if (Registers->InterruptNumber == 0xE)
		{
			fixme("IRQ14");
		}
		else if (Registers->InterruptNumber == 0xF)
		{
			fixme("IRQ15");
		}
		return OK;
	}
}
