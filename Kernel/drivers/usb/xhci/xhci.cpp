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

#if defined(__amd64__) || defined(__i386__)

#include <driver.hpp>
#include <interface/usb.h>
#include <cpu.hpp>
#include <pci.hpp>

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
EXTERNC void KPrint(const char *Format, ...);

namespace Driver::ExtensibleHostControllerInterface
{
	dev_t DriverID;

	std::list<PCI::PCIDevice> Devices;
	int Entry()
	{
		for (auto &&dev : Devices)
		{
			PCIManager->InitializeDevice(dev, KernelPageTable);
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)dev.Header;
		}

		return 0;
	}

	int Final()
	{
		return ENOSYS;
	}

	int Panic()
	{
		return ENOSYS;
	}

	int Probe()
	{
		Devices = PCIManager->FindPCIDevice(0xC, 0x3, 0x30);

		if (Devices.empty())
		{
			trace("No device found.");
			return -ENODEV;
		}
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(xhci,
							"Extensible Host Controller Interface Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}

#endif
