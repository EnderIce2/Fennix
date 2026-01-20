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

#include "uhci.hpp"

#include <cpu.hpp>
#include <pci.hpp>
#include <foward_list>

#include "../../../kernel.h"

extern PCI::Manager *PCIManager;
// EXTERNC void KPrint(const char *Format, ...);

namespace Driver::UniversalHostControllerInterface
{
	static_assert(sizeof(TD) == 32, "TD must be 32 bytes");
	static_assert(sizeof(QH) == 64, "QH must be 64 bytes");

	dev_t DriverID;

	int UHCI_Start(struct USBController *d) { return ((HCD *)d)->Start(false); }
	int UHCI_Stop(struct USBController *d) { return ((HCD *)d)->Stop(); }
	int UHCI_Reset(struct USBController *d) { return ((HCD *)d)->Reset(); }
	int UHCI_Poll(struct USBController *d) { return ((HCD *)d)->Poll(); }

	int UHCI_Port_Control(struct USBDevice *Device, struct USBTransfer *Transfer)
	{
		stub;
		return 0;
	}

	int UHCI_Port_Interrupt(struct USBDevice *Device, struct USBTransfer *Transfer)
	{
		stub;
		return 0;
	}

	std::list<PCI::PCIDevice> Devices;
	std::list<HCD *> Controllers;
	int Entry()
	{
		return ENOSYS;

		for (auto &&dev : Devices)
		{
			PCIManager->MapPCIAddresses(dev, KernelPageTable);
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)dev.Header;
			hdr0->Header.Command |= PCI::PCI_COMMAND_MASTER | PCI::PCI_COMMAND_IO;
			hdr0->Header.Command &= ~PCI::PCI_COMMAND_INTX_DISABLE;

			HCD *hc = new HCD(hdr0->BAR[4], dev);
			// hc->Flags =
			hc->StartHC = UHCI_Start;
			hc->StopHC = UHCI_Stop;
			hc->ResetHC = UHCI_Reset;
			hc->PollHC = UHCI_Poll;

			hc->Reset();
			if (hc->Start(true) != 0)
			{
				error("Failed to start UHCI controller %d:%d:%d", dev.Bus, dev.Device, dev.Function);
				delete hc;
				continue;
			}
			hc->Detect();

			Controllers.push_back(hc);
			v0::AddController(DriverID, hc);
		}

		return 0;
	}

	int Final()
	{
		for (auto &&hc : Controllers)
		{
			v0::RemoveController(DriverID, hc);
			delete hc;
		}
		return 0;
	}

	int Panic()
	{
		return ENOSYS;
	}

	int Probe()
	{
		Devices = PCIManager->FindPCIDevice(0xC, 0x3, 0x00);
		if (Devices.empty())
			return ENODEV;
		debug("there are %d controllers", Devices.size());
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(uhci,
							"Universal Host Controller Interface Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}

#endif
