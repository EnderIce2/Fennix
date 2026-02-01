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

#include "uhci.hpp"

extern Driver::Manager *DriverManager;

namespace Driver::UniversalHostControllerInterface
{
	extern dev_t DriverID;

	IO_REG_RW_EX(uint16_t, USBCMD, cmd,
				 RS = 1u << 0,
				 HCRESET = 1u << 1,
				 GRESET = 1u << 2,
				 EGSM = 1u << 3,
				 FGR = 1u << 4,
				 SWDBG = 1u << 5,
				 CF = 1u << 6,
				 MAXP = 1u << 7);

	IO_REG_RWC_EX(uint16_t, USBSTS, sts,
				  USBINT = 1u << 0,
				  USBERRINT = 1u << 1,
				  RD = 1u << 2,
				  HSE = 1u << 3,
				  HCPE = 1u << 4,
				  HCH = 1u << 5);

	IO_REG_RW_EX(uint16_t, USBINTR, intr,
				 TOCRC = 1u << 0,
				 RIE = 1u << 1,
				 IOCE = 1u << 2,
				 SPIE = 1u << 3);

	int UHCI_Port_Control(struct USBDevice *Device, struct USBTransfer *Transfer);
	int UHCI_Port_Interrupt(struct USBDevice *, struct USBTransfer *);

	int HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		return EOK;
	}

	int HCD::Reset()
	{
		cmd.out(USBCMD::GRESET);
		bool timeout = false;
		whileto(cmd.has(USBCMD::GRESET), 100, timeout)
			v0::Sleep(DriverID, 10);

		if (timeout)
			warn("global reset timeout");

		cmd.out(USBCMD::HCRESET);
		whileto(cmd.has(USBCMD::HCRESET), 100, timeout)
			v0::Sleep(DriverID, 10);

		if (timeout)
			warn("reset timeout");

		outb(io + REG_SOFMOD, 64); /* Timing Value. 64 = 12000 @ 12MHz */
		outl(io + REG_FLBASEADD, (uint32_t)(uintptr_t)this->queue->sched->FrameList);
		outw(io + REG_FRNUM, this->queue->sched->GetCurrentFrame() & 0x7FF);

		intr.out(USBINTR::TOCRC | USBINTR::RIE | USBINTR::IOCE | USBINTR::SPIE);
		return 0;
	}

	int HCD::Start()
	{
		// outw(io + REG_USBSTS, 0xFFFF);
		// cmd.set(USBCMD::CF | USBCMD::RS | USBCMD::MAXP);

		// bool timeout = false;
		// whileto(sts.has(USBSTS::HCH), 1000, timeout)
		// 	CPU::Pause();

		// if (timeout)
		// 	return ETIMEDOUT;
		return 0;
	}

	int HCD::Stop()
	{
		cmd.clr(USBCMD::RS);
		return 0;
	}

	int HCD::Detect()
	{
		PORTSC port1 = inw(io + REG_PORTSC1);
		PORTSC port2 = inw(io + REG_PORTSC2);

		if (port1.AlwaysOne && port1 != 0xFFFF)
		{
			debug("Port 1 is present %#lx", port1);
			Port *p1 = new Port(io + REG_PORTSC1);
			int ret = p1->Probe();
			if (ret != 0)
			{
				delete p1;
				debug("no 1");
			}
			else
			{
				USBDevice *dev = v0::CreateUSBDevice(DriverID);
				assert(dev != nullptr);
				dev->Controller = this;
				dev->Port = 0;
				dev->Speed = p1->GetSpeed();
				dev->MaxPacketSize = 8;
				dev->Address = 0;
				dev->PortCtl = UHCI_Port_Control;
				dev->PortInt = UHCI_Port_Interrupt;
				v0::InitializeUSBDevice(DriverID, dev);
				debug("done 1");
			}
		}

		if (port2.AlwaysOne && port2 != 0xFFFF)
		{
			debug("Port 2 is present %#lx", port2);
			Port *p2 = new Port(io + REG_PORTSC2);
			int ret = p2->Probe();
			if (ret != 0)
			{
				delete p2;
				debug("no 2");
			}
			else
			{
				USBDevice *dev = v0::CreateUSBDevice(DriverID);
				assert(dev != nullptr);
				dev->Controller = this;
				dev->Port = 1;
				dev->Speed = p2->GetSpeed();
				dev->MaxPacketSize = 8;
				dev->Address = 0;
				dev->PortCtl = UHCI_Port_Control;
				dev->PortInt = UHCI_Port_Interrupt;
				v0::InitializeUSBDevice(DriverID, dev);
				debug("done 2");
			}
		}
		return 0;
	}

	int HCD::ProcessQueueHead(QH *qh)
	{
		stub;
		return 0;
	}

	int HCD::Poll()
	{
		stub;
		return 0;
	}

	HCD::HCD(PCI::PCIDevice &pciHeader)
		: Interrupts::Handler(pciHeader),
		  io((uint16_t)pciHeader.GetBAR(4)),
		  Header(pciHeader)
	{
		/**
		 * Remove Resource Type Indicator
		 *
		 * 2.2.2 USBBASE in UHCI Design Guide 1.1
		 */
		io &= ~0x1;

		cmd.port = io + REG_USBCMD;
		sts.port = io + REG_USBSTS;
		intr.port = io + REG_USBINTR;

		/* More info in UHCI Design Guide 1.1 @ 5.2.1 */
		outw(io + REG_LEGSUP, 0x2000);
		if (Header.IsVendor(PCI::IntelCorporation))
		{
			/* Disable non-PME# wakeup on Intel */
			outw(io + REG_INTEL, 0x0000);
		}

		queue = new Queue;
	}

	HCD::~HCD()
	{
		this->Stop();
		delete queue;
	}
}
