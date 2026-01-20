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
		return 0;
	}

	int HCD::Stop()
	{
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

		NoOpCommandTRB no_op = {};
		no_op.Control.CycleBit(1);
		no_op.Control.TRBType(TRBT_NoOpCommand);
		auto ret = SendCommand((TRB *)&no_op);
		assert(ret);
		debug("code: %#lx slot: %#lx", ret->Status.CompletionCode(), ret->Control.SlotID());
		return 0;
	}

	int HCD::Poll()
	{
		return 0;
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
