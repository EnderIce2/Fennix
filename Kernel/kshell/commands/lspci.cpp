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

#include "../cmds.hpp"

#include <filesystem.hpp>

#include "../../kernel.h"

using namespace vfs;

void cmd_lspci(const char *args)
{
	if (args)
	{
		if (IF_ARG("-i") || IF_ARG("--info"))
		{
			for (auto Device : PCIManager->GetDevices())
			{
				const char *HdrType;
				switch (Device.Header->HeaderType)
				{
				case 0:
					HdrType = "Normal ";
					break;
				case 1:
					HdrType = "PCI-PCI";
					break;
				case 2:
					HdrType = "Cardbus";
					break;
				default:
					HdrType = "Unknown";
					break;
				}

				printf("%04x:%04x | %s:%03d | %02x:%02x.%d | %s:  %s  %s\n",
					   Device.Header->VendorID,
					   Device.Header->DeviceID,
					   HdrType, Device.Header->HeaderType,
					   Device.Bus,
					   Device.Device,
					   Device.Function,
					   PCI::Descriptors::GetSubclassName(Device.Header->Class,
														 Device.Header->Subclass),
					   PCI::Descriptors::GetVendorName(Device.Header->VendorID),
					   PCI::Descriptors::GetDeviceName(Device.Header->VendorID,
													   Device.Header->DeviceID));
			}
			return;
		}
	}

	for (auto Device : PCIManager->GetDevices())
	{
		printf("%02x:%02x.%d: %s:  %s  %s\n",
			   Device.Bus,
			   Device.Device,
			   Device.Function,
			   PCI::Descriptors::GetSubclassName(Device.Header->Class,
												 Device.Header->Subclass),
			   PCI::Descriptors::GetVendorName(Device.Header->VendorID),
			   PCI::Descriptors::GetDeviceName(Device.Header->VendorID,
											   Device.Header->DeviceID));
	}
}
