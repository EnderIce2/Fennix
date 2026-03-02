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

	int HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		uint16_t status = sts.in();
		if ((status & 0x1F) == 0)
			return ENOTSUP;

		debug("UHCI Interrupt: USBINT=%d USBERRINT=%d RD=%d HSE=%d HCPE=%d", (status & USBSTS::USBINT) != 0, (status & USBSTS::USBERRINT) != 0, (status & USBSTS::RD) != 0, (status & USBSTS::HSE) != 0, (status & USBSTS::HCPE) != 0);

		if (sts.has(USBSTS::USBINT))
			queue->ProcessComplete();

		if (sts.has(USBSTS::USBERRINT))
		{
			warn("USB Error Interrupt detected");
			queue->ProcessComplete();
		}

		if (sts.has(USBSTS::RD))
		{
			trace("Resume detected");
			this->Start();
		}

		if (sts.has(USBSTS::HSE))
		{
			error("Host System Error detected");
			this->Reset();
			this->Start();
		}

		if (sts.has(USBSTS::HCPE))
		{
			error("Host Controller Process Error detected");
			assert(!"HCPE error handling not implemented");
		}

		sts.out(USBSTS::USBINT | USBSTS::USBERRINT | USBSTS::RD | USBSTS::HSE | USBSTS::HCPE);
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
		outl(io + REG_FLBASEADD, (uint32_t)(uintptr_t)this->queue->GetFrameList());
		outw(io + REG_FRNUM, this->queue->GetCurrentFrame() & 0x7FF);

		intr.out(USBINTR::TOCRC | USBINTR::RIE | USBINTR::IOCE | USBINTR::SPIE);
		return 0;
	}

	int HCD::Start()
	{
		sts.out(0xFFFF);
		cmd.set(USBCMD::CF | USBCMD::RS | USBCMD::MAXP);

		bool timeout = false;
		whileto(sts.has(USBSTS::HCH), 1000, timeout)
			CPU::Pause();

		if (timeout)
			return ETIMEDOUT;
		return 0;
	}

	int HCD::Stop()
	{
		cmd.clr(USBCMD::RS);
		return 0;
	}

	int HCD::Detect()
	{
		if (port1.has(PORTSC::AlwaysOne) && port1.in() != 0xFFFF)
		{
			debug("Port 1 is present %#lx", port1);
			Port *p1 = new Port(port1);
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
				v0::InitializeUSBDevice(DriverID, dev);
				debug("done 1");
			}
		}

		if (port2.has(PORTSC::AlwaysOne) && port2.in() != 0xFFFF)
		{
			debug("Port 2 is present %#lx", port2);
			Port *p2 = new Port(port2);
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
				v0::InitializeUSBDevice(DriverID, dev);
				debug("done 2");
			}
		}
		return 0;
	}

	int HCD::SubmitControl(USBDevice *Device, USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;
		USBEndpoint &ep = Device->Endpoints[0];
		const uint8_t devAddr = Device->Endpoints[0].Address & 0x7F;

		TD *firstTD = nullptr;
		TD *prevTD = nullptr;

		/* SETUP packet */
		{
			ep.Toggle = 0;
			TD *td = queue->AllocateTransferDescriptor();
			if (!td)
				return ENOMEM;

			td->CS.LowSpeedDevice((Device->Speed == USB_LOW_SPEED) ? 1 : 0);
			td->CS.Status_ACTIVE(1);

			td->LINK.Terminate(1);

			td->TOKEN.PacketIdentification(TD_PID_SETUP);
			td->TOKEN.Endpoint(0);
			td->TOKEN.DeviceAddress(devAddr);
			td->TOKEN.DataToggle(ep.Toggle);
			td->TOKEN.MaximumLength(sizeof(USBDeviceRequest));

			td->BufferPointer = (uintptr_t)urb->Request;
			td->URB = urb;

			prevTD = firstTD = td;
		}

		/* DATA packet */
		{
			uint8_t *ptr = (uint8_t *)urb->Buffer;
			size_t remaining = urb->Length;
			uint8_t pid = (urb->Request->bmRequestType.raw & 0x80) ? TD_PID_IN : TD_PID_OUT;
			while (remaining > 0)
			{
				ep.Toggle ^= 1; /* flip DATA1/DATA0 */
				TD *td = queue->AllocateTransferDescriptor();
				if (!td)
				{
					TD *cleanup = firstTD;
					while (cleanup)
					{
						TD *next = cleanup->NextTD;
						queue->ReleaseTransferDescriptor(cleanup);
						cleanup = next;
					}
					return ENOMEM;
				}
				size_t pktSize = std::min((size_t)ep.MaxPacketSize, remaining);

				td->BufferPointer = (uintptr_t)ptr;

				td->CS.LowSpeedDevice((Device->Speed == USB_LOW_SPEED) ? 1 : 0);
				td->CS.Status_ACTIVE(1);
				td->LINK.Terminate(1);

				td->TOKEN.PacketIdentification(pid);
				td->TOKEN.Endpoint(0);
				td->TOKEN.DeviceAddress(devAddr);
				td->TOKEN.DataToggle(ep.Toggle);
				td->TOKEN.MaximumLength(pktSize);
				td->URB = urb;

				prevTD->LINK.Terminate(0);
				prevTD->LINK.DepthBreadthSelect(1);
				prevTD->LINK.LinkPointer((uintptr_t)td);
				prevTD->NextTD = td;
				prevTD = td;

				ptr += pktSize;
				remaining -= pktSize;
			}
		}

		/* STATUS packet */
		{
			ep.Toggle = 1;
			uint8_t pid = (urb->Request->bmRequestType.raw & 0x80) ? TD_PID_OUT : TD_PID_IN;
			TD *td = queue->AllocateTransferDescriptor();
			if (!td)
				return ENOMEM;

			td->CS.LowSpeedDevice((Device->Speed == USB_LOW_SPEED) ? 1 : 0);
			td->CS.Status_ACTIVE(1);
			td->CS.InterruptOnComplete(1);

			td->LINK.Terminate(1);

			td->TOKEN.PacketIdentification(pid);
			td->TOKEN.Endpoint(0);
			td->TOKEN.DeviceAddress(devAddr);
			td->TOKEN.DataToggle(ep.Toggle);
			td->TOKEN.MaximumLength(0);

			td->BufferPointer = 0;
			td->URB = urb;

			prevTD->LINK.Terminate(0);
			prevTD->LINK.LinkPointer((uintptr_t)td);
			prevTD->NextTD = td;
		}

		urb->PrivateData = firstTD;
		queue->EnqueueTD(firstTD);
		return 0;
	}

	int HCD::SubmitBulk(struct USBDevice *Device, struct USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		size_t remaining = urb->Length;
		uint8_t *ptr = (uint8_t *)urb->Buffer;
		uint8_t epNum = urb->EndpointAddress & 0x0F;
		USBEndpoint &ep = Device->Endpoints[epNum];
		const uint8_t devAddr = Device->Endpoints[0].Address & 0x7F;

		TD *firstTD = nullptr;
		TD *prevTD = nullptr;

		while (remaining > 0)
		{
			TD *td = queue->AllocateTransferDescriptor();
			if (!td)
			{
				TD *cleanup = firstTD;
				while (cleanup)
				{
					TD *next = cleanup->NextTD;
					queue->ReleaseTransferDescriptor(cleanup);
					cleanup = next;
				}
				return ENOMEM;
			}

			size_t pktSize = std::min((size_t)ep.MaxPacketSize, remaining);
			uint8_t pid = (urb->EndpointAddress & 0x80) ? TD_PID_IN : TD_PID_OUT;
			debug("Creating TD: PID=%s, EP=%d, DevAddr=%d, MaxLen=%zu", (pid == TD_PID_IN) ? "IN" : "OUT", epNum, Device->Endpoints[epNum].Address, pktSize);

			td->BufferPointer = (uintptr_t)ptr;

			td->CS.LowSpeedDevice((Device->Speed == USB_LOW_SPEED) ? 1 : 0);
			td->CS.Status_ACTIVE(1);
			td->LINK.Terminate(1);

			td->TOKEN.PacketIdentification(pid);
			td->TOKEN.Endpoint(epNum);
			td->TOKEN.DeviceAddress(devAddr);
			td->TOKEN.DataToggle(ep.Toggle);
			td->TOKEN.MaximumLength(pktSize);
			td->URB = urb;

			if (!firstTD)
				firstTD = td;
			if (prevTD)
			{
				prevTD->LINK.Terminate(0);
				prevTD->LINK.DepthBreadthSelect(1);
				prevTD->LINK.LinkPointer((uintptr_t)td);
				prevTD->NextTD = td;
			}

			prevTD = td;

			ptr += pktSize;
			remaining -= pktSize;
			ep.Toggle ^= 1; /* flip DATA0/DATA1 */
		}

		if (!prevTD)
			return EINVAL;
		prevTD->CS.InterruptOnComplete(1);
		urb->PrivateData = firstTD;

		queue->EnqueueTD(firstTD);

		return 0;
	}

	int HCD::SubmitInterrupt(struct USBDevice *Device, struct USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		uint8_t epNum = urb->EndpointAddress & 0x0F;
		size_t len = urb->Length;
		USBEndpoint &ep = Device->Endpoints[epNum];
		const uint8_t devAddr = Device->Endpoints[0].Address & 0x7F;

		TD *td = queue->AllocateTransferDescriptor();
		if (!td)
			return ENOMEM;

		td->CS.LowSpeedDevice((Device->Speed == USB_LOW_SPEED) ? 1 : 0);
		td->CS.Status_ACTIVE(1);
		td->CS.InterruptOnComplete(1);

		td->LINK.Terminate(1);

		td->TOKEN.PacketIdentification(TD_PID_IN);
		td->TOKEN.Endpoint(epNum);
		td->TOKEN.DeviceAddress(devAddr);
		td->TOKEN.DataToggle(ep.Toggle);
		td->TOKEN.MaximumLength(len);

		td->BufferPointer = (uintptr_t)urb->Buffer;
		td->URB = urb;

		urb->PrivateData = td;
		queue->EnqueueTD(td);
		return 0;
	}

	int HCD::SubmitIsochronous(struct USBDevice *Device, struct USBRequestBlock *urb)
	{
		assert(!"Isochronous transfer submission not implemented");
	}

	int HCD::Submit(struct USBDevice *Device, struct USBRequestBlock *urb)
	{
		switch (urb->Type)
		{
		case USB_TRANSFER_CONTROL:
			return SubmitControl(Device, urb);
		case USB_TRANSFER_BULK:
			return SubmitBulk(Device, urb);
		case USB_TRANSFER_INTERRUPT:
			return SubmitInterrupt(Device, urb);
		case USB_TRANSFER_ISOCHRONOUS:
			return SubmitIsochronous(Device, urb);
		default:
			return ENOTSUP;
		}
	}

	int HCD::Cancel(struct USBDevice *Device, struct USBRequestBlock *urb)
	{
		TD *td = (TD *)urb->PrivateData;

		while (td)
		{
			td->CS.Status_ACTIVE(0);
			td = td->NextTD;
		}

		urb->Status = USB_REQ_CANCELED;
		return 0;
	}

	HCD::HCD(PCI::PCIDevice &pciHeader)
		: Interrupt::Handler(pciHeader),
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
		port1.port = io + REG_PORTSC1;
		port2.port = io + REG_PORTSC2;

		/* More info in UHCI Design Guide 1.1 @ 5.2.1 */
		outw(io + REG_LEGSUP, 0x2000);
		if (Header.IsVendor(PCI::IntelCorporation))
		{
			/* Disable non-PME# wakeup on Intel */
			outw(io + REG_INTEL, 0x0000);
		}

		queue = new TransferQueue(io);
	}

	HCD::~HCD()
	{
		this->Stop();
		delete queue;
	}
}
