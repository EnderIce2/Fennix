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

#include "pcnet.hpp"

#include <net/net.hpp>
#include <debug.h>
#include <pci.hpp>
#include <io.h>

#include "../../mapi.hpp"
#include "../mod.hpp"

using namespace PCI;

namespace PCNET
{
	KernelAPI KAPI;

	PCIDeviceHeader *PCIBaseAddress;
	BARData BAR;

	MediaAccessControl MAC;
	InternetProtocol::Version4 IP;

	void WriteRAP32(uint32_t Value) { outportl(BAR.IOBase + 0x14, Value); }
	void WriteRAP16(uint16_t Value) { outportw(BAR.IOBase + 0x12, Value); }

	uint32_t ReadCSR32(uint32_t CSR)
	{
		WriteRAP32(CSR);
		return inportl(BAR.IOBase + 0x10);
	}

	uint16_t ReadCSR16(uint16_t CSR)
	{
		WriteRAP32(CSR);
		return inportw(BAR.IOBase + 0x10);
	}

	void WriteCSR32(uint32_t CSR, uint32_t Value)
	{
		WriteRAP32(CSR);
		outportl(BAR.IOBase + 0x10, Value);
	}

	void WriteCSR16(uint16_t CSR, uint16_t Value)
	{
		WriteRAP16(CSR);
		outportw(BAR.IOBase + 0x10, Value);
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
			debug("Module received configuration data.");
			PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
			if (PCIBaseAddress->VendorID == 0x1022 && PCIBaseAddress->DeviceID == 0x2000)
			{
				trace("Found AMD PCNET.");
				uint32_t PCIBAR = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
				BAR.Type = PCIBAR & 1;
				BAR.IOBase = (uint16_t)(PCIBAR & (~3));
				BAR.MemoryBase = PCIBAR & (~15);
			}
			else
				return DEVICE_NOT_SUPPORTED;
			break;
		}
		case QueryReason:
		{
			memcpy(Data->NetworkCallback.Fetch.Name, (void *)"AMD PCNET", 10);
			Data->NetworkCallback.Fetch.MAC = MAC.ToHex();
			break;
		}
		case SendReason:
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

	int InterruptCallback(CPURegisters *)
	{
		return OK;
	}
}
