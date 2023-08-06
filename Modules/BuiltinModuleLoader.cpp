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

#include "drv.hpp"

#include "AHCI/ahci.hpp"
#include "VMware/mouse.hpp"
#include "PersonalSystem2/mouse.hpp"
#include "PersonalSystem2/keyboard.hpp"
#include "ATA/ata.hpp"
#include "AudioCodec97/ac97.hpp"
#include "Realtek/rtl8139.hpp"
#include "AdvancedMicroDevices/pcnet.hpp"
#include "Intel/gigabit.hpp"

#include <pci.hpp>
#include <vector>

#include "../Core/Driver/api.hpp"
#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

FexExtended AHCIExtendedHeader = {
	.Driver = {
		.Name = "Advanced Host Controller Interface",
		.Type = FexDriverType_Storage,
		.Callback = AdvancedHostControllerInterface::CallbackHandler,
		.InterruptCallback = AdvancedHostControllerInterface::InterruptCallback,
		.Bind = {
			.Type = BIND_PCI,
			.PCI = {
				.VendorID = {0x8086, 0x15AD},
				.DeviceID = {0x2922, 0x2829, 0x07E0},
				.Class = 0x1,
				.SubClass = 0x6,
				.ProgIF = 0x1,
			}}}};

FexExtended VMwareVirtualMouseExtendedHeader = {
	.Driver = {
		.Name = "VMware Virtual Mouse",
		.Type = FexDriverType_Input,
		.TypeFlags = FexDriverInputTypes_Mouse,
		.OverrideOnConflict = true,
		.Callback = VMwareMouse::CallbackHandler,
		.InterruptCallback = VMwareMouse::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {12}, // IRQ12
			}}}};

FexExtended PS2MouseExtendedHeader = {
	.Driver = {
		.Name = "PS/2 Mouse",
		.Type = FexDriverType_Input,
		.TypeFlags = FexDriverInputTypes_Mouse,
		.Callback = PS2Mouse::CallbackHandler,
		.InterruptCallback = PS2Mouse::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {12}, // IRQ12
			}}}};

FexExtended PS2KeyboardExtendedHeader = {
	.Driver = {
		.Name = "PS/2 Keyboard",
		.Type = FexDriverType_Input,
		.TypeFlags = FexDriverInputTypes_Keyboard,
		.Callback = PS2Keyboard::CallbackHandler,
		.InterruptCallback = PS2Keyboard::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {1}, // IRQ1
			}}}};

FexExtended ATAExtendedHeader = {
	.Driver = {
		.Name = "Advanced Technology Attachment",
		.Type = FexDriverType_Storage,
		.Callback = AdvancedTechnologyAttachment::CallbackHandler,
		.InterruptCallback = AdvancedTechnologyAttachment::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {14, 15}, // IRQ14, IRQ15
			}}}};

FexExtended AC97ExtendedHeader = {
	.Driver = {
		.Name = "Audio Codec '97 Driver",
		.Type = FexDriverType_Audio,
		.TypeFlags = FexDriverInputTypes_None,
		.OverrideOnConflict = false,
		.Callback = AudioCodec97::CallbackHandler,
		.InterruptCallback = AudioCodec97::InterruptCallback,
		.Bind = {
			.Type = BIND_PCI,
			.PCI = {
				.VendorID = {0x8086},
				.DeviceID = {0x2415},
				.Class = 0x4,
				.SubClass = 0x3,
				.ProgIF = 0x0,
			}}}};

FexExtended RTL8139ExtendedHeader = {
	.Driver = {
		.Name = "Realtek RTL8139",
		.Type = FexDriverType_Network,
		.Callback = RTL8139::CallbackHandler,
		.InterruptCallback = RTL8139::InterruptCallback,
		.Bind = {
			.Type = BIND_PCI,
			.PCI = {
				.VendorID = {0x10EC},
				.DeviceID = {0x8139},
				.Class = 0x2,
				.SubClass = 0x0,
				.ProgIF = 0x0,
			}}}};

FexExtended AMDPCNETExtendedHeader = {
	.Driver = {
		.Name = "Advanced Micro Devices PCNET",
		.Type = FexDriverType_Network,
		.Callback = PCNET::CallbackHandler,
		.InterruptCallback = PCNET::InterruptCallback,
		.Bind = {
			.Type = BIND_PCI,
			.PCI = {
				.VendorID = {0x1022},
				.DeviceID = {0x2000},
				.Class = 0x2,
				.SubClass = 0x0,
				.ProgIF = 0x0,
			}}}};

FexExtended IntelGigabitExtendedHeader = {
	.Driver = {
		.Name = "Intel Gigabit Ethernet Controller",
		.Type = FexDriverType_Network,
		.Callback = Gigabit::CallbackHandler,
		.InterruptCallback = Gigabit::InterruptCallback,
		.Bind = {
			.Type = BIND_PCI,
			.PCI = {
				.VendorID = {0x8086},
				.DeviceID = {0x100E, 0x100F, 0x10D3, 0x10EA, 0x153A},
				.Class = 0x2,
				.SubClass = 0x0,
				.ProgIF = 0x0,
			}}}};

#pragma GCC diagnostic pop

std::vector<PCI::PCIDeviceHeader *> FindPCI(uint16_t VendorID[16], uint16_t DeviceID[16])
{
	for (uint16_t Vidx = 0; Vidx < 16; Vidx++)
	{
		for (uint16_t Didx = 0; Didx < 16; Didx++)
		{
			if (VendorID[Vidx] == 0 || DeviceID[Didx] == 0)
				break;

			std::vector<PCI::PCIDeviceHeader *> devices = PCIManager->FindPCIDevice(VendorID[Vidx], DeviceID[Didx]);
			if (devices.size() == 0)
				continue;

			return devices;
		}
	}

	warn("No PCI device found");
	return std::vector<PCI::PCIDeviceHeader *>();
}

bool StartAHCI()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = AdvancedHostControllerInterface::DriverEntry,
		.ExtendedHeader = &AHCIExtendedHeader};

	if (DriverManager->DriverLoadBindPCI((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartVMwareMouse()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = VMwareMouse::DriverEntry,
		.ExtendedHeader = &VMwareVirtualMouseExtendedHeader};

	if (DriverManager->DriverLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartPS2Mouse()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = PS2Mouse::DriverEntry,
		.ExtendedHeader = &PS2MouseExtendedHeader};

	if (DriverManager->DriverLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartPS2Keyboard()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = PS2Keyboard::DriverEntry,
		.ExtendedHeader = &PS2KeyboardExtendedHeader};

	if (DriverManager->DriverLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartATA()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = AdvancedTechnologyAttachment::DriverEntry,
		.ExtendedHeader = &ATAExtendedHeader};

	if (DriverManager->DriverLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartAC97()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = AudioCodec97::DriverEntry,
		.ExtendedHeader = &AC97ExtendedHeader};

	if (DriverManager->DriverLoadBindPCI((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartRTL8139()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = RTL8139::DriverEntry,
		.ExtendedHeader = &RTL8139ExtendedHeader};

	if (DriverManager->DriverLoadBindPCI((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartPCNET()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = PCNET::DriverEntry,
		.ExtendedHeader = &AMDPCNETExtendedHeader};

	if (DriverManager->DriverLoadBindPCI((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}

bool StartGigabit()
{
	Driver::BuiltInDriverInfo BIDI = {
		.EntryPoint = Gigabit::DriverEntry,
		.ExtendedHeader = &IntelGigabitExtendedHeader};

	if (DriverManager->DriverLoadBindPCI((uintptr_t)&BIDI, 0, true) == Driver::DriverCode::OK)
		return true;

	return false;
}
