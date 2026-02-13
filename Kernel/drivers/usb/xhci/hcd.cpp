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

#include <quirks.hpp>

namespace Driver::ExtensibleHostControllerInterface
{
	extern dev_t DriverID;

	const char *TRBCompletionCodeString[] =
		{
			"Invalid",
			"Success",
			"DataBufferError",
			"BabbleDetectedError",
			"USBTransactionError",
			"TRBError",
			"StallError",
			"ResourceError",
			"BandwidthError",
			"NoSlotsAvailableError",
			"InvalidStreamTypeError",
			"SlotNotEnabledError",
			"EndpointNotEnabledError",
			"ShortPacket",
			"RingUnderrun",
			"RingOverrun",
			"VFEventRingFullError",
			"ParameterError",
			"BandwidthOverrunError",
			"ContextStateError",
			"NoPingResponseError",
			"EventRingFullError",
			"IncompatibleDeviceError",
			"MissedServiceError",
			"CommandRingStopped",
			"CommandAborted",
			"Stopped",
			"StoppedLengthInvalid",
			"StoppedShortPacket",
			"MaxExitLatencyTooLargeError",
			"Reserved",
			"IsochBufferOverrun",
			"EventLostError",
			"UndefinedError",
			"InvalidStreamIDError",
			"SecondaryBandwidthError",
			"SplitTransactionError",
	};

	bool HCD::TakeOwnership()
	{
		ExtendedCapabilityPointer *ext = (ExtendedCapabilityPointer *)Ext;
		while (true)
		{
			if (ext->CapabilityID() == EXTCAP_USBLegacySupport)
				break;

			uint32_t nextOffset = ext->NextExtendedCapabilityPointer();
			if (nextOffset == 0)
			{
				debug("xHCI Legacy Support capability not found");
				return true;
			}
			ext = (ExtendedCapabilityPointer *)((uintptr_t)ext + (nextOffset << 2));
		}
		LegacySupportCapability *legacy = (LegacySupportCapability *)ext;

		/* Figure 4-37: OS Ownership State Machine */
		if (legacy->USBLEGSUP.OSOwnedSemaphore != 1)
		{
			legacy->USBLEGCTLSTS.SMIOnBAREnable = 0;
			legacy->USBLEGCTLSTS.SMIOnOSOwnershipEnable = 0;
			legacy->USBLEGCTLSTS.SMIOnHostSystemErrorEnable = 0;
			legacy->USBLEGCTLSTS.SMIOnPCICommandEnable = 0;
			legacy->USBLEGCTLSTS.SMIOnBAREnable = 0;

			v0::Sleep(DriverID, 10);

			legacy->USBLEGSUP.OSOwnedSemaphore = 1;
		}

		bool timeout = false;
		whileto(legacy->USBLEGSUP.BIOSOwnedSemaphore == 1, 5000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			warn("Timeout waiting for BIOS to release xHCI ownership!");
			return false;
		}
		return true;
	}

	void HCD::InitializeProtocols()
	{
		if (!SupportedProtocols.empty())
			SupportedProtocols.clear();
		ExtendedCapabilityPointer *ext = (ExtendedCapabilityPointer *)Ext;
		while (true)
		{
			if (ext->CapabilityID() == EXTCAP_SupportedProtocol)
			{
				SupportedProtocolCapability *proto = (SupportedProtocolCapability *)ext;
				debug("Found supported protocol: \"%c%c%c%c%d.%d\" %u-%u", proto->NameString, proto->NameString >> 8, proto->NameString >> 16, proto->NameString >> 24,
					  proto->C.RevisionMajor() & 0xF, proto->C.RevisionMinor() & 0xF,
					  proto->PCT.CompatiblePortOffset() - 1, proto->PCT.CompatiblePortOffset() + proto->PCT.CompatiblePortCount() - 1);

				SupportedProtocols.push_back(proto);
			}

			uint32_t nextOffset = ext->NextExtendedCapabilityPointer();
			if (nextOffset == 0)
				break;
			ext = (ExtendedCapabilityPointer *)((uintptr_t)ext + (nextOffset << 2));
		}
	}

	CommandCompletionEventTRB *HCD::SendCommand(TRB *trb, size_t TimeoutMs)
	{
		CmdRing.EnqueueTRB(trb);
		DbManager->RingCommand();
		bool timeout = false;
		whileto(CommandIRQComplete == 0, (int)TimeoutMs, timeout) v0::Sleep(DriverID, 10);
		UNUSED(timeout);

		auto event = CompletedCmds.size() ? CompletedCmds.front() : nullptr;
		CompletedCmds.clear();
		CommandIRQComplete = 0;

		if (event == nullptr)
		{
			error("xHCI command timed out!");
			return nullptr;
		}

		if (event->Status.CompletionCode() != TRBCC_Success)
		{
			error("xHCI command failed with error %s", TRBCompletionCodeString[event->Status.__CommandCompletionParameter]);
			return nullptr;
		}

		return event;
	}

	int HCD::Reset()
	{
		if (TakeOwnership() == false)
		{
			error("Unable to take ownership from BIOS!");
			return EADDRNOTAVAIL;
		}

		Op->USBCMD.RunStop(0);
		bool timeout = false;
		whileto(Op->USBSTS.HCHalted() == 0, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to stop!");
			return ETIMEDOUT;
		}

		v0::Sleep(DriverID, 50);

		Op->USBCMD.HostControllerReset(1);
		whileto(Op->USBCMD.HostControllerReset() == 1, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to reset!");
			return ETIMEDOUT;
		}

		whileto(Op->USBSTS.ControllerNotReady() == 1, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to become ready!");
			return ETIMEDOUT;
		}

		v0::Sleep(DriverID, 50);

		if (Op->USBCMD != 0x0)
			error("Wrong state USBCMD = %#x; expected 0x0", Op->USBCMD);
		if (Op->USBSTS != 0x1 && Op->USBSTS != 0x11)
			error("Wrong state USBSTS = %#x; expected 0x1 or 0x11", Op->USBSTS);
		if (Op->DNCTRL != 0x0)
			error("Wrong state DNCTRL = %#x; expected 0x0", Op->DNCTRL);
		if (Op->CRCR != 0x0)
			error("Wrong state CRCR = %#x; expected 0x0", Op->CRCR);
		if (Op->DCBAAP != 0x0)
			error("Wrong state DCBAAP = %#x; expected 0x0", Op->DCBAAP);
		if (Op->CONFIG != 0x0)
			error("Wrong state CONFIG = %#x; expected 0x0", Op->CONFIG);

		debug("HC PAGE_SIZE: %#lx", (Op->PAGESIZE & 0xFFFFF) << 12);
		Op->DNCTRL = (uint32_t)0xFFFF;
		Op->CONFIG = (uint32_t)DeviceSlots;

		memset((void *)Allocations.DCBAAP, 0, PAGE_SIZE);
		Op->DCBAAP = (uint64_t)Allocations.DCBAAP;

		if (ScratchpadCount > 0)
		{
			if (Allocations.ScratchpadBuffers == 0)
			{
				Allocations.ScratchpadBuffers = (uint64_t *)v0::AllocateMemory(DriverID, TO_PAGES(sizeof(uint64_t) * ScratchpadCount));
				Memory::Virtual(KernelPageTable).Map(Allocations.ScratchpadBuffers, Allocations.ScratchpadBuffers, sizeof(uint64_t) * ScratchpadCount, Memory::P | Memory::RW | Memory::PCD | Memory::PWT);

				for (uint16_t i = 0; i < ScratchpadCount; i++)
				{
					Allocations.ScratchpadBuffers[i] = (uint64_t)v0::AllocateMemory(DriverID, 1);
					Memory::Virtual(KernelPageTable).Map((void *)Allocations.ScratchpadBuffers[i], (void *)Allocations.ScratchpadBuffers[i], PAGE_SIZE, Memory::P | Memory::RW | Memory::PCD | Memory::PWT);
				}
			}

			Allocations.DCBAAP[0] = (uint64_t)Allocations.ScratchpadBuffers;
		}

		Op->CRCR.RingCycleState(CmdRing.GetycleState());
		Op->CRCR.CommandRingPointer((uint64_t)CmdRing.GetBuffer());

		Interrupter = &Rt->Interrupter[0];
		assert(Interrupter != nullptr);
		Interrupter->IMAN.InterruptEnable(1);
		EvRing = std::shared_ptr<EventRing>(new EventRing(1, Interrupter));

		Op->USBSTS.EventInterrupt(1);
		Interrupter->IMAN.InterruptPending(1);

		InitializeProtocols();
		return 0;
	}

	int HCD::Start()
	{
		Op->USBCMD.HostSystemErrorEnable(1);
		Op->USBCMD.InterrupterEnable(1);
		Op->USBCMD.RunStop(1);

		bool timeout = false;
		whileto(Op->USBSTS.HCHalted() == 1, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to start!");
			return ETIMEDOUT;
		}

		whileto(Op->USBSTS.ControllerNotReady() == 1, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to start!");
			return ETIMEDOUT;
		}

		if (hwquirks::IsQEMU())
		{
			// TODO
		}

		return 0;
	}

	int HCD::Stop()
	{
		Op->USBCMD.RunStop(0);
		bool timeout = false;
		whileto(Op->USBSTS.HCHalted() == 0, 1000, timeout) v0::Sleep(DriverID, 10);
		if (timeout)
		{
			error("Timeout waiting for xHCI to stop!");
			return ETIMEDOUT;
		}
		return 0;
	}

	int HCD::Detect()
	{
		if (!Ports.empty())
			Ports.clear();

		for (auto &&proto : SupportedProtocols)
		{
			uint8_t off = proto->PCT.CompatiblePortOffset();
			uint8_t offMax = proto->PCT.CompatiblePortOffset() + proto->PCT.CompatiblePortCount();
			uint8_t cnt = proto->PCT.CompatiblePortCount();
			uint8_t maxPorts = Cap->HCSPARAMS1.NumberOfPorts();

			for (ssize_t i = off; i < offMax && i < maxPorts + 1; i++)
			{
				PortRegister &port = Por[i - 1];

				if (port.PORTSC.ConnectStatusChange() && port.PORTSC.CurrentConnectStatus())
				{
					Port portObj(&port, proto);
					int ret = portObj.Reset();
					if (ret == 0)
					{
						debug("port speed: %#lx", port.PORTSC.PortSpeed());
					}
					Ports.push_back(portObj);
				}
			}
		}

#ifdef DEBUG
		NoOpCommandTRB no_op = {};
		no_op.Control.CycleBit(1);
		no_op.Control.TRBType(TRBT_NoOpCommand);
		auto ret = SendCommand((TRB *)&no_op);
		if (ret == nullptr)
			error("no-op command timed out!");
		else
			info("code: %#lx slot: %#lx", ret->Status.CompletionCode(), ret->Control.SlotID());
#endif
		return 0;
	}

	int HCD::SubmitControl(USBDevice *Device, USBRequestBlock *urb)
	{
		uint32_t *transferStatus = (uint32_t *)v0::AllocateMemory(DriverID, 1);
		*transferStatus = 0;
		uint8_t *dataBuffer = (uint8_t *)v0::AllocateMemory(DriverID, TO_PAGES(256));
		memset(dataBuffer, 0, 256);

		SetupStageTRB setup = {};
		setup.Parameter.raw = *(uint64_t *)urb->Request;

		setup.Status.TRBTransferLength(8);
		setup.Status.InterrupterTarget(0);

		setup.Control.CycleBit(0);
		setup.Control.InterruptOnCompletion(0);
		setup.Control.ImmediateData(1);
		setup.Control.TRBType(TRBT_SetupStage);
		setup.Control.TransferType(3);

		DataStageTRB data = {};
		data.DataBufferPointer = (uintptr_t)dataBuffer;

		data.Status.TRBTransferLength(urb->Length);
		data.Status.TDSize(0);
		data.Status.InterrupterTarget(0);

		data.Control.CycleBit(0);
		data.Control.EvaluateNextTRB(0);
		data.Control.InterruptOnShortPacket(0);
		data.Control.NoSnoop(0);
		data.Control.ChainBit(1);
		data.Control.InterruptOnCompletion(0);
		data.Control.ImmediateData(0);
		data.Control.TRBType(TRBT_DataStage);
		data.Control.Direction(1);

		EventDataTRB eventData = {};
		eventData.EventDataPointer = (uintptr_t)transferStatus;

		data.Status.InterrupterTarget(0);

		eventData.Control.CycleBit(0);
		eventData.Control.EvaluateNextTRB(0);
		eventData.Control.ChainBit(0);
		eventData.Control.InterruptOnCompletion(1);
		eventData.Control.BlockEventInterrupt(0);
		eventData.Control.TRBType(TRBT_EventData);

		CmdRing.EnqueueTRB((TRB *)&setup);
		CmdRing.EnqueueTRB((TRB *)&data);
		CmdRing.EnqueueTRB((TRB *)&eventData);
		DbManager->RingCommand();

		StatusStageTRB status = {};

		return ENOSYS;
	}

	int HCD::SubmitBulk(USBDevice *Device, USBRequestBlock *urb)
	{
		assert(!"Bulk transfer submission not implemented");
	}

	int HCD::SubmitInterrupt(USBDevice *Device, USBRequestBlock *urb)
	{
		assert(!"Interrupt transfer submission not implemented");
	}

	int HCD::SubmitIsochronous(USBDevice *Device, USBRequestBlock *urb)
	{
		assert(!"Isochronous transfer submission not implemented");
	}

	int HCD::Submit(USBDevice *Device, USBRequestBlock *urb)
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

	int HCD::Cancel(USBDevice *Device, USBRequestBlock *urb)
	{
		return ENOSYS;
	}

	HCD::HCD(PCI::PCIDevice &pciHeader) : Interrupts::Handler(pciHeader), Header(pciHeader)
	{
		MMIOBase = pciHeader.GetBAR(0);

		Cap = (Capability *)MMIOBase;
		Op = (Operational *)(MMIOBase + Cap->CAPLENGTH);
		Por = (PortRegister *)(MMIOBase + Cap->CAPLENGTH + 0x400);
		Rt = (Runtime *)(MMIOBase + (Cap->RTSOFF & ~0x1F));
		Db = (Doorbell *)(MMIOBase + (Cap->DBOFF & ~0x3));
		off_t extOff = Cap->HCCPARAMS1.xHCIExtendedCapabilitiesPointer() << 2;
		Ext = (ExtendedCapabilityPointer *)(MMIOBase + extOff);
		DbManager = std::shared_ptr<DoorbellManager>(new DoorbellManager(Db));
		DeviceSlots = Cap->HCSPARAMS1.NumberOfDeviceSlots();
		ScratchpadCount = Cap->HCSPARAMS2.MaxScratchpadBuffers();
		size_t dcbaaLength = sizeof(uintptr_t) * (DeviceSlots + 1);
		Allocations.DCBAAP = (uint64_t *)v0::AllocateMemory(DriverID, TO_PAGES(dcbaaLength));
	}

	HCD::~HCD()
	{
		this->Stop();
	}
}
