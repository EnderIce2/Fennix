/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#include <pci.h>

#include <driver.h>

extern __driverAPI *API;

static_assert(sizeof(PCIArray) == sizeof(__PCIArray),
			  "PCIArray size mismatch");

PCIArray *FindPCIDevices(uint16_t Vendors[], uint16_t Devices[])
{
	return (PCIArray *)API->GetPCIDevices(API->MajorID,
										  Vendors,
										  Devices);
}

void InitializePCI(PCIDevice *Device)
{
	API->InitializePCI(API->MajorID,
					   (void *)Device->Header);
}

uint32_t GetBAR(uint8_t Index, PCIDevice *Device)
{
	return API->GetBAR(API->MajorID,
					   Index,
					   (void *)Device->Header);
}

uint8_t iLine(PCIDevice *Device)
{
	PCIHeader0 *Header = (PCIHeader0 *)Device->Header;
	return Header->InterruptLine;
}

uint8_t iPin(PCIDevice *Device)
{
	PCIHeader0 *Header = (PCIHeader0 *)Device->Header;
	return Header->InterruptPin;
}
