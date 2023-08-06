#include "ata.hpp"

#include <debug.h>
#include <pci.hpp>
#include <io.h>

#include "../../DAPI.hpp"
#include "../drv.hpp"

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
			debug("Driver received configuration data.");
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
			debug("Driver stopped.");
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
