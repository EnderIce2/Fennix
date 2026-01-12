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
		while (ext->CapabilityID() != EXTCAP_USBLegacySupport)
		{
			if (ext->NextExtendedCapabilityPointer() == 0)
			{
				debug("xHCI Legacy Support capability not found");
				return true;
			}
			off_t offset = ext->NextExtendedCapabilityPointer();
			ext = (ExtendedCapabilityPointer *)((uintptr_t)ext + (offset << 2));
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

		Op->USBCMD.RunStop(0);
		int timeout = 0;
		// whileto(Op->USBSTS.HCHalted() == 1, 1000, timeout) v0::Sleep(DriverID, 1);
		// if (timeout)
		// {
		// 	error("Timeout waiting for xHCI to stop!");
		// 	return ETIMEDOUT;
		// }

		Op->USBCMD.HostControllerReset(1);
		whileto(Op->USBCMD.HostControllerReset() == 1, 1000, timeout) v0::Sleep(DriverID, 1);
		if (timeout)
		{
			error("Timeout waiting for xHCI to reset!");
			return ETIMEDOUT;
		}
		v0::Sleep(DriverID, 10);

		whileto(Op->USBSTS.ControllerNotReady() == 1, 1000, timeout) v0::Sleep(DriverID, 1);
		if (timeout)
		{
			error("Timeout waiting for xHCI to become ready!");
			return ETIMEDOUT;
		}

		DeviceSlots = Cap->HCSPARAMS1.MaxDeviceSlots();

		Op->DCBAAP = (uint64_t)Allocations.DCBAAP;
		memset((void *)Allocations.DCBAAP, 0, PAGE_SIZE);

		uint16_t scratchpadCount = Cap->HCSPARAMS2.MaxScratchpadBuffers();
		if (scratchpadCount > 0)
		{
			if (Allocations.ScratchpadBuffers == 0)
			{
				Allocations.ScratchpadBuffers = (uint64_t *)v0::AllocateMemory(DriverID, 1);

				for (uint16_t i = 0; i < Cap->HCSPARAMS2.MaxScratchpadBuffers() && i < PAGE_SIZE / sizeof(uint64_t); i++)
					Allocations.ScratchpadBuffers[i] = (uint64_t)v0::AllocateMemory(DriverID, 1);
			}

			Allocations.DCBAAP[0] = (uint64_t)Allocations.ScratchpadBuffers;
		}

		Op->CRCR.raw = 0;
		Op->CRCR.CommandRingPointer((uint64_t)CmdRing.GetBuffer());

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
		MMIOBase = pciHeader.GetBAR(0);

		Cap = (Capability *)MMIOBase;
		Op = (Operational *)(MMIOBase + Cap->CAPLENGTH);
		Ports = (PortRegister *)(MMIOBase + Cap->CAPLENGTH + 0x400);
		Rt = (Runtime *)(MMIOBase + (Cap->RTSOFF & ~0x1F));
		Db = (Doorbell *)(MMIOBase + (Cap->DBOFF & ~0x3));
		off_t extOff = Cap->HCCPARAMS1.GetExtendedCapabilitiesPointer() << 2;
		Ext = (ExtendedCapabilityPointer *)(MMIOBase + extOff);

		Allocations.DCBAAP = (uint64_t *)v0::AllocateMemory(DriverID, 1);
	}

	HCD::~HCD()
	{
		this->Stop();
	}
}
