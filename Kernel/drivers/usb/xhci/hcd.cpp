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

#include "xhci.hpp"

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	bool HCD::TakeOwnership()
	{
		ExtendedCapabilityPointer *ext = (ExtendedCapabilityPointer *)Ext;
		while (ext->CapabilityID != EXTCAP_USBLegacySupport)
		{
			if (ext->NextExtendedCapabilityPointer == 0)
			{
				debug("xHCI Legacy Support capability not found");
				return true;
			}
			ext = (ExtendedCapabilityPointer *)((uintptr_t)ext + (ext->NextExtendedCapabilityPointer << 2));
		}
		LegacySupportCapability *legacy = (LegacySupportCapability *)ext;

		/* Figure 4-37: OS Ownership State Machine */
		if (legacy->USBLEGSUP.OSOwnedSemaphore != 1)
			legacy->USBLEGSUP.OSOwnedSemaphore = 1;

		int timeout = 0;
		whileto(legacy->USBLEGSUP.BIOSOwnedSemaphore == 1, 1000, timeout);
		if (timeout)
		{
			warn("Timeout waiting for BIOS to release xHCI ownership!");
			return false;
		}
		return true;
	}

	void HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
	}

	int HCD::Reset()
	{
		if (TakeOwnership() == false)
		{
			error("Unable to take ownership from BIOS!");
			return EADDRNOTAVAIL;
		}
		return 0;
	}

	int HCD::Start(bool WaitForStart)
	{
		return 0;
	}

	int HCD::Stop()
	{
		return 0;
	}

	int HCD::Detect()
	{
		return 0;
	}

	int HCD::Poll()
	{
		return 0;
	}

	HCD::HCD(PCI::PCIDevice &pciHeader)
		: Interrupts::Handler(pciHeader),
		  Header(pciHeader)
	{
		PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)pciHeader.Header;
		MMIOBase = pciHeader.GetBAR(0);

		Cap = (Capability *)MMIOBase;
		Op = (Operational *)(MMIOBase + Cap->CAPLENGTH);
		Ports = (PortRegister *)(MMIOBase + Cap->CAPLENGTH + 0x400);
		Rt = (Runtime *)(MMIOBase + (Cap->RTSOFF & ~0x1F));
		Db = (Doorbell *)(MMIOBase + (Cap->DBOFF & ~0x3));
		Ext = (ExtendedCapabilityPointer *)(MMIOBase + (Cap->HCCPARAMS1.xHCIExtendedCapacitiesPointer << 2));
		stub;
	}

	HCD::~HCD()
	{
		this->Stop();
	}
}
