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

#include "mod.hpp"

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

#include "../core/module/api.hpp"
#include "../kernel.h"
#include "../mapi.hpp"
#include "../Fex.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

FexExtended AHCIExtendedHeader = {
	.Module = {
		.Name = "Advanced Host Controller Interface",
		.Type = FexModuleType_Storage,
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
	.Module = {
		.Name = "VMware Virtual Mouse",
		.Type = FexModuleType_Input,
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
	.Module = {
		.Name = "PS/2 Mouse",
		.Type = FexModuleType_Input,
		.TypeFlags = FexDriverInputTypes_Mouse,
		.Callback = PS2Mouse::CallbackHandler,
		.InterruptCallback = PS2Mouse::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {12}, // IRQ12
			}}}};

FexExtended PS2KeyboardExtendedHeader = {
	.Module = {
		.Name = "PS/2 Keyboard",
		.Type = FexModuleType_Input,
		.TypeFlags = FexDriverInputTypes_Keyboard,
		.Callback = PS2Keyboard::CallbackHandler,
		.InterruptCallback = PS2Keyboard::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {1}, // IRQ1
			}}}};

FexExtended ATAExtendedHeader = {
	.Module = {
		.Name = "Advanced Technology Attachment",
		.Type = FexModuleType_Storage,
		.Callback = AdvancedTechnologyAttachment::CallbackHandler,
		.InterruptCallback = AdvancedTechnologyAttachment::InterruptCallback,
		.Bind = {
			.Type = BIND_INTERRUPT,
			.Interrupt = {
				.Vector = {14, 15}, // IRQ14, IRQ15
			}}}};

FexExtended AC97ExtendedHeader = {
	.Module = {
		.Name = "Audio Codec '97 Module",
		.Type = FexModuleType_Audio,
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
	.Module = {
		.Name = "Realtek RTL8139",
		.Type = FexModuleType_Network,
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
	.Module = {
		.Name = "Advanced Micro Devices PCNET",
		.Type = FexModuleType_Network,
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
	.Module = {
		.Name = "Intel Gigabit Ethernet Controller",
		.Type = FexModuleType_Network,
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

std::vector<PCI::PCIDevice> FindPCI(uint16_t VendorID[16], uint16_t DeviceID[16])
{
	for (uint16_t Vidx = 0; Vidx < 16; Vidx++)
	{
		for (uint16_t Didx = 0; Didx < 16; Didx++)
		{
			if (VendorID[Vidx] == 0 || DeviceID[Didx] == 0)
				break;

			std::vector<PCI::PCIDevice> devices = PCIManager->FindPCIDevice(VendorID[Vidx], DeviceID[Didx]);
			if (devices.size() == 0)
				continue;

			return devices;
		}
	}

	warn("No PCI device found");
	return std::vector<PCI::PCIDevice>();
}

bool StartAHCI()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = AdvancedHostControllerInterface::DriverEntry,
		.ExtendedHeader = &AHCIExtendedHeader};

	if (ModuleManager->ModuleLoadBindPCI((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartVMwareMouse()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = VMwareMouse::DriverEntry,
		.ExtendedHeader = &VMwareVirtualMouseExtendedHeader};

	if (ModuleManager->ModuleLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartPS2Mouse()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = PS2Mouse::DriverEntry,
		.ExtendedHeader = &PS2MouseExtendedHeader};

	if (ModuleManager->ModuleLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartPS2Keyboard()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = PS2Keyboard::DriverEntry,
		.ExtendedHeader = &PS2KeyboardExtendedHeader};

	if (ModuleManager->ModuleLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartATA()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = AdvancedTechnologyAttachment::DriverEntry,
		.ExtendedHeader = &ATAExtendedHeader};

	if (ModuleManager->ModuleLoadBindInterrupt((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartAC97()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = AudioCodec97::DriverEntry,
		.ExtendedHeader = &AC97ExtendedHeader};

	if (ModuleManager->ModuleLoadBindPCI((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartRTL8139()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = RTL8139::DriverEntry,
		.ExtendedHeader = &RTL8139ExtendedHeader};

	if (ModuleManager->ModuleLoadBindPCI((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartPCNET()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = PCNET::DriverEntry,
		.ExtendedHeader = &AMDPCNETExtendedHeader};

	if (ModuleManager->ModuleLoadBindPCI((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

bool StartGigabit()
{
	Module::BuiltInModuleInfo BIDI = {
		.EntryPoint = Gigabit::DriverEntry,
		.ExtendedHeader = &IntelGigabitExtendedHeader};

	if (ModuleManager->ModuleLoadBindPCI((uintptr_t)&BIDI, 0, true) == Module::ModuleCode::OK)
		return true;

	return false;
}

void StartBuiltInModules()
{
	StartAHCI();
	StartVMwareMouse();
	StartPS2Mouse();
	StartPS2Keyboard();
	StartATA();
	StartAC97();
	StartRTL8139();
	StartPCNET();
	StartGigabit();
}
