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

#include "ohci.hpp"

#include <list>

extern Driver::Manager *DriverManager;

namespace Driver::OpenHostControllerInterface
{
	extern dev_t DriverID;

	namespace
	{
		constexpr uint32_t OHCI_INTR_WDH = 1u << 1;

		constexpr uint32_t OHCI_CMD_HCR = 1u << 0;
		constexpr uint32_t OHCI_CMD_CLF = 1u << 1;
		constexpr uint32_t OHCI_CMD_BLF = 1u << 2;
		constexpr uint32_t OHCI_CMD_OCR = 1u << 3;

		constexpr uint32_t OHCI_CTRL_PLE = 1u << 2;
		constexpr uint32_t OHCI_CTRL_CLE = 1u << 4;
		constexpr uint32_t OHCI_CTRL_BLE = 1u << 5;
		constexpr uint32_t OHCI_CTRL_HCFS_SHIFT = 6;
		constexpr uint32_t OHCI_CTRL_HCFS_MASK = 0x3u << OHCI_CTRL_HCFS_SHIFT;
		constexpr uint32_t OHCI_CTRL_IR = 1u << 8;

		constexpr uint32_t OHCI_HCFS_OPERATIONAL = 2u;
		constexpr uint32_t OHCI_HCFS_SUSPEND = 3u;

		constexpr uint32_t OHCI_RH_PORT_CCS = 1u << 0;
		constexpr uint32_t OHCI_RH_PORT_PES = 1u << 1;
		constexpr uint32_t OHCI_RH_PORT_PRS = 1u << 4;
		constexpr uint32_t OHCI_RH_PORT_PPS = 1u << 8;
		constexpr uint32_t OHCI_RH_PORT_LSDA = 1u << 9;
		constexpr uint32_t OHCI_RH_PORT_CSC = 1u << 16;
		constexpr uint32_t OHCI_RH_PORT_PESC = 1u << 17;
		constexpr uint32_t OHCI_RH_PORT_PRSC = 1u << 20;

		constexpr uint32_t OHCI_RHA_NPS = 1u << 9;

		constexpr uint8_t OHCI_TD_DP_SETUP = 0;
		constexpr uint8_t OHCI_TD_DP_OUT = 1;
		constexpr uint8_t OHCI_TD_DP_IN = 2;
		constexpr uint8_t OHCI_TD_DI_NO_INTERRUPT = 7;
		constexpr uint8_t OHCI_TD_T_DATA0 = 2;
		constexpr uint8_t OHCI_TD_T_DATA1 = 3;
		constexpr uint8_t OHCI_CC_NOERROR = 0;
		constexpr uint8_t OHCI_CC_STALL = 4;

		struct TDMetadata
		{
			enum DescriptorType
			{
				GeneralTD,
				IsochronousTD
			} Type;
			void *Descriptor;
			USBRequestBlock *URB;
			uint32_t BufferStart;
			uint32_t BufferEnd;
			size_t RequestedLength;
			bool CountAsPayload;
		};

		struct PendingTransfer
		{
			enum ListType
			{
				Control,
				Bulk,
				Periodic
			} Type;
			USBRequestBlock *URB;
			ED *Descriptor;
			void *DummyTD;
			bool DummyIsIsochronous;
			uint8_t PeriodicSlot;
		};

		struct URBCompletion
		{
			USBRequestBlock *URB;
			size_t ActualLength;
			USBRequestStatus Status;
		};

		std::list<TDMetadata> TDMetadatas;
		std::list<PendingTransfer> PendingTransfers;

		struct DoneNode
		{
			uint32_t Word0;
			uint32_t Word1;
			uint32_t NextTD;
			uint32_t Word3;
		};

		static uint32_t MaskTDPtr(uint32_t ptr) { return ptr & ~0xFu; }
		static uint32_t MaskEDPtr(uint32_t ptr) { return ptr & ~0xFu; }

		static USBRequestStatus ConditionCodeToStatus(uint32_t cc)
		{
			if (cc == OHCI_CC_NOERROR)
				return USB_REQ_SUCCESS;
			if (cc == OHCI_CC_STALL)
				return USB_REQ_STALL;
			return USB_REQ_ERROR;
		}

		static void RegisterTD(TD *td, USBRequestBlock *urb, uint32_t cbp, uint32_t be, bool countPayload)
		{
			size_t requested = (be >= cbp) ? ((be - cbp) + 1u) : 0u;
			TDMetadatas.push_back({TDMetadata::GeneralTD, td, urb, cbp, be, requested, countPayload});
		}

		static void RegisterITD(ITD *itd, USBRequestBlock *urb, uint32_t bp0, uint32_t be, size_t requested)
		{
			TDMetadatas.push_back({TDMetadata::IsochronousTD, itd, urb, bp0, be, requested, true});
		}

		static void ForgetTDMetadata(TD *td)
		{
			for (auto it = TDMetadatas.begin(); it != TDMetadatas.end(); ++it)
			{
				if (it->Descriptor == td)
				{
					TDMetadatas.erase(it);
					return;
				}
			}
		}

		static bool TakeTDMetadata(void *td, TDMetadata &meta)
		{
			for (auto it = TDMetadatas.begin(); it != TDMetadatas.end(); ++it)
			{
				if (it->Descriptor != td)
					continue;

				meta = *it;
				TDMetadatas.erase(it);
				return true;
			}
			return false;
		}

		static PendingTransfer *FindPendingTransfer(USBRequestBlock *urb)
		{
			for (auto &pending : PendingTransfers)
			{
				if (pending.URB == urb)
					return &pending;
			}
			return nullptr;
		}

		static void ForgetPendingTransfer(USBRequestBlock *urb)
		{
			for (auto it = PendingTransfers.begin(); it != PendingTransfers.end(); ++it)
			{
				if (it->URB == urb)
				{
					PendingTransfers.erase(it);
					return;
				}
			}
		}

		static void LinkEDToList(OHCIRegisters *regs, ED *ed, bool controlList)
		{
			uint32_t head = controlList ? mminl((void *)&regs->HcControlHeadED) : mminl((void *)&regs->HcBulkHeadED);
			ed->NextED = head;
			if (controlList)
				mmoutl((void *)&regs->HcControlHeadED, (uint32_t)(uintptr_t)ed);
			else
				mmoutl((void *)&regs->HcBulkHeadED, (uint32_t)(uintptr_t)ed);
		}

		static void LinkEDToPeriodicSlot(HCCA *hcca, ED *ed, uint8_t slot)
		{
			slot &= 0x1F;
			ed->NextED = hcca->InterruptTable[slot];
			hcca->InterruptTable[slot] = (uint32_t)(uintptr_t)ed;
		}

		static void UnlinkEDFromList(OHCIRegisters *regs, ED *ed, bool controlList)
		{
			uint32_t target = (uint32_t)(uintptr_t)ed;
			uint32_t head = controlList ? mminl((void *)&regs->HcControlHeadED) : mminl((void *)&regs->HcBulkHeadED);

			uint32_t prev = 0;
			uint32_t cur = head;
			while (cur)
			{
				ED *node = (ED *)(uintptr_t)MaskEDPtr(cur);
				if ((uint32_t)(uintptr_t)node == target)
				{
					if (prev == 0)
					{
						if (controlList)
							mmoutl((void *)&regs->HcControlHeadED, node->NextED);
						else
							mmoutl((void *)&regs->HcBulkHeadED, node->NextED);
					}
					else
						((ED *)(uintptr_t)MaskEDPtr(prev))->NextED = node->NextED;
					return;
				}

				prev = cur;
				cur = node->NextED;
			}
		}

		static void UnlinkEDFromPeriodicSlot(HCCA *hcca, ED *ed, uint8_t slot)
		{
			slot &= 0x1F;
			uint32_t target = (uint32_t)(uintptr_t)ed;
			uint32_t prev = 0;
			uint32_t cur = hcca->InterruptTable[slot];

			while (cur)
			{
				ED *node = (ED *)(uintptr_t)MaskEDPtr(cur);
				if ((uint32_t)(uintptr_t)node == target)
				{
					if (prev == 0)
						hcca->InterruptTable[slot] = node->NextED;
					else
						((ED *)(uintptr_t)MaskEDPtr(prev))->NextED = node->NextED;
					return;
				}

				prev = cur;
				cur = node->NextED;
			}
		}
	}

	ED *Queue::AllocateEndpointDescriptor()
	{
		for (size_t i = 0; i < EDCount; ++i)
		{
			if (!EDUsed[i])
			{
				EDUsed[i] = true;
				ED *ed = &EDPool[i];
				memset(ed, 0, sizeof(ED));
				return ed;
			}
		}
		return nullptr;
	}

	int Queue::ReleaseEndpointDescriptor(ED *ed)
	{
		size_t index = ed - EDPool;
		if (index >= EDCount)
			return EINVAL;

		EDUsed[index] = false;
		return 0;
	}

	TD *Queue::AllocateTransferDescriptor()
	{
		for (size_t i = 0; i < TDCount; ++i)
		{
			if (!TDUsed[i])
			{
				TDUsed[i] = true;
				TD *td = &TDPool[i];
				memset(td, 0, sizeof(TD));
				return td;
			}
		}
		return nullptr;
	}

	ITD *Queue::AllocateIsochronousDescriptor()
	{
		for (size_t i = 0; i < TDCount; ++i)
		{
			if (!ITDUsed[i])
			{
				ITDUsed[i] = true;
				ITD *itd = &ITDPool[i];
				memset(itd, 0, sizeof(ITD));
				return itd;
			}
		}
		return nullptr;
	}

	int Queue::ReleaseTransferDescriptor(TD *td)
	{
		size_t index = td - TDPool;
		if (index >= TDCount)
			return EINVAL;

		TDUsed[index] = false;
		return 0;
	}

	int Queue::ReleaseIsochronousDescriptor(ITD *itd)
	{
		size_t index = itd - ITDPool;
		if (index >= TDCount)
			return EINVAL;

		ITDUsed[index] = false;
		return 0;
	}

	Queue::Queue()
	{
		TDPool = (TD *)KernelAllocator.RequestPage();
		ITDPool = (ITD *)KernelAllocator.RequestPage();
		EDPool = (ED *)KernelAllocator.RequestPage();

		memset(TDPool, 0, 4096);
		memset(ITDPool, 0, 4096);
		memset(EDPool, 0, 4096);

		memset(TDUsed, 0, sizeof(TDUsed));
		memset(ITDUsed, 0, sizeof(ITDUsed));
		memset(EDUsed, 0, sizeof(EDUsed));
	}

	Queue::~Queue()
	{
		KernelAllocator.FreePage(TDPool);
		KernelAllocator.FreePage(ITDPool);
		KernelAllocator.FreePage(EDPool);
	}

	int HCD::OnInterruptReceived(CPU::TrapFrame *Frame)
	{
		uint32_t status = mminl((void *)&regs->HcInterruptStatus);
		uint32_t enabled = mminl((void *)&regs->HcInterruptEnable);
		status &= enabled; // Only process enabled interrupts

		if (!status)
			return ENOTSUP;

		if (status & OHCI_INTR_WDH)
		{ // WritebackDoneHead
			uint32_t doneHead = hcca->DoneHead;
			hcca->DoneHead = 0;

			// 1. Reverse the list (HC produces LIFO, we need FIFO)
			uint32_t reversedList = 0;
			uint32_t currentAddr = MaskTDPtr(doneHead);
			while (currentAddr)
			{
				DoneNode *node = (DoneNode *)(uintptr_t)currentAddr;
				uint32_t next = node->NextTD;
				node->NextTD = reversedList;
				reversedList = currentAddr;
				currentAddr = MaskTDPtr(next);
			}

			std::list<URBCompletion> completions;

			auto getCompletion = [&completions](USBRequestBlock *urb) -> URBCompletion *
			{
				for (auto &completion : completions)
				{
					if (completion.URB == urb)
						return &completion;
				}

				completions.push_back({urb, 0, USB_REQ_SUCCESS});
				return &completions.back();
			};

			// 2. Process the reversed done list
			DoneNode *td = (DoneNode *)(uintptr_t)reversedList;
			while (td)
			{
				TDMetadata meta{};
				if (TakeTDMetadata(td, meta))
				{
					URBCompletion *completion = getCompletion(meta.URB);
					uint32_t cc = (td->Word0 >> 28) & 0xF;
					if (cc != OHCI_CC_NOERROR && completion->Status == USB_REQ_SUCCESS)
						completion->Status = ConditionCodeToStatus(cc);

					if (meta.CountAsPayload)
					{
						if (meta.Type == TDMetadata::GeneralTD)
						{
							TD *gtd = (TD *)td;
							size_t requested = meta.RequestedLength;
							size_t remaining = 0;
							if (gtd->CurrentBufferPointer != 0 && gtd->CurrentBufferPointer <= meta.BufferEnd)
								remaining = (meta.BufferEnd - gtd->CurrentBufferPointer) + 1;
							if (remaining > requested)
								remaining = requested;
							completion->ActualLength += (requested - remaining);
						}
						else
						{
							if (cc == OHCI_CC_NOERROR)
								completion->ActualLength += meta.RequestedLength;
						}
					}
				}

				DoneNode *next = (DoneNode *)(uintptr_t)td->NextTD;
				if (meta.Type == TDMetadata::IsochronousTD)
					queue->ReleaseIsochronousDescriptor((ITD *)td);
				else
					queue->ReleaseTransferDescriptor((TD *)td);
				td = next;
			}

			for (auto &completion : completions)
			{
				PendingTransfer *pending = FindPendingTransfer(completion.URB);
				if (!pending)
					continue;

				switch (pending->Type)
				{
				case PendingTransfer::Control:
					UnlinkEDFromList(regs, pending->Descriptor, true);
					break;
				case PendingTransfer::Bulk:
					UnlinkEDFromList(regs, pending->Descriptor, false);
					break;
				case PendingTransfer::Periodic:
					UnlinkEDFromPeriodicSlot(hcca, pending->Descriptor, pending->PeriodicSlot);
					break;
				default:
					assert(!"Invalid PendingTransfer::Type");
				}
				if (pending->DummyIsIsochronous)
					queue->ReleaseIsochronousDescriptor((ITD *)pending->DummyTD);
				else
					queue->ReleaseTransferDescriptor((TD *)pending->DummyTD);
				queue->ReleaseEndpointDescriptor(pending->Descriptor);

				completion.URB->ActualLength = completion.ActualLength;
				if (completion.URB->Status != USB_REQ_CANCELED)
					completion.URB->Status = completion.Status;

				if (completion.URB->Complete)
					completion.URB->Complete(completion.URB);

				ForgetPendingTransfer(completion.URB);
			}
		}

		mmoutl((void *)&regs->HcInterruptStatus, status);
		return EOK;
	}

	int HCD::Reset()
	{
		uint32_t interval = mminl((void *)&regs->HcFmInterval);
		debug("frame interval = %#lx", interval);

		uint32_t control = mminl((void *)&regs->HcControl);
		if (control & OHCI_CTRL_IR)
		{
			mmoutl((void *)&regs->HcCommandStatus, OHCI_CMD_OCR);
			for (int i = 0; i < 500; ++i)
			{
				if ((mminl((void *)&regs->HcControl) & OHCI_CTRL_IR) == 0)
					break;
				v0::Sleep(DriverID, 1);
			}
		}

		mmoutl((void *)&regs->HcCommandStatus, OHCI_CMD_HCR);

		for (int i = 0; i < 100000; ++i)
		{
			uint32_t cs = mminl((void *)&regs->HcCommandStatus);
			if ((cs & OHCI_CMD_HCR) == 0)
			{
				debug("ok");
				break;
			}
		}

		mmoutl((void *)&regs->HcFmInterval, interval);

		memset(hcca, 0, sizeof(HCCA));
		debug("HCCA physical address: %#lx", (uintptr_t)hcca);
		mmoutl((void *)&regs->HcHCCA, (uint32_t)(uintptr_t)hcca);

		mmoutl((void *)&regs->HcControlHeadED, 0);
		mmoutl((void *)&regs->HcBulkHeadED, 0);
		mmoutl((void *)&regs->HcDoneHead, 0);

		uint32_t fi = interval & 0x3FFFu;
		uint32_t ps = (fi * 90) / 100; /* approx 90% */
		mmoutl((void *)&regs->HcPeriodicStart, ps & 0x3FFFU);
		mmoutl((void *)&regs->HcControlCurrentED, 0);
		mmoutl((void *)&regs->HcBulkCurrentED, 0);
		return 0;
	}

	int HCD::Start()
	{
		uint32_t control = mminl((void *)&regs->HcControl);

		control |= OHCI_CTRL_CLE; // ControlListEnable
		control |= OHCI_CTRL_BLE; // BulkListEnable
		control |= OHCI_CTRL_PLE; // PeriodicListEnable

		control &= ~OHCI_CTRL_HCFS_MASK;
		control |= (OHCI_HCFS_OPERATIONAL << OHCI_CTRL_HCFS_SHIFT);

		mmoutl((void *)&regs->HcControl, control);

		mmoutl((void *)&regs->HcInterruptEnable,
			   OHCI_INTR_WDH | // WritebackDoneHead
				   (1 << 2) |  // SOF
				   (1 << 30)); // Master Interrupt

		return 0;
	}

	int HCD::Stop()
	{
		uint32_t control = mminl((void *)&regs->HcControl);
		control &= ~OHCI_CTRL_HCFS_MASK;
		control |= (OHCI_HCFS_SUSPEND << OHCI_CTRL_HCFS_SHIFT);
		mmoutl((void *)&regs->HcControl, control);
		return 0;
	}

	int HCD::Detect()
	{
		uint32_t descA = mminl((void *)&regs->HcRhDescriptorA);
		int ports = descA & 0xFF;
		bool perPortPower = (descA & OHCI_RHA_NPS) == 0;

		for (int i = 0; i < ports; ++i)
		{
			volatile uint32_t *portReg = &regs->HcRhPortStatus1 + i;
			uint32_t status = mminl((void *)portReg);

			if ((status & OHCI_RH_PORT_CCS) == 0)
				continue;

			if (perPortPower)
			{
				mmoutl((void *)portReg, OHCI_RH_PORT_PPS);
				v0::Sleep(DriverID, 20);
			}

			mmoutl((void *)portReg, OHCI_RH_PORT_CSC | OHCI_RH_PORT_PESC | OHCI_RH_PORT_PRSC);
			mmoutl((void *)portReg, OHCI_RH_PORT_PRS); // Set PortReset

			for (int retry = 0; retry < 100; retry++)
			{
				if (mminl((void *)portReg) & OHCI_RH_PORT_PRSC)
					break;
				v0::Sleep(DriverID, 10);
			}
			mmoutl((void *)portReg, OHCI_RH_PORT_PRSC); // Clear reset change

			status = mminl((void *)portReg);
			if ((status & OHCI_RH_PORT_PES) == 0)
				mmoutl((void *)portReg, OHCI_RH_PORT_PES);

			USBDevice *dev = v0::CreateUSBDevice(DriverID);
			if (!dev)
				return ENOMEM;

			dev->Controller = this;
			dev->Port = i;
			dev->Speed = (status & OHCI_RH_PORT_LSDA) ? USB_LOW_SPEED : USB_FULL_SPEED;
			v0::InitializeUSBDevice(DriverID, dev);
		}
		return 0;
	}

	int HCD::SubmitControl(USBDevice *Device, USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		USBEndpoint &ep = Device->Endpoints[0];

		ED *ed = queue->AllocateEndpointDescriptor();
		if (!ed)
			return ENOMEM;

		/* ED Control: Use Device Address, not endpoint address */
		ed->Control =
			(Device->Endpoints[0].Address & 0x7F) |				 // FA: Function Address (bits 0-6)
			(0 << 7) |											 // EN: Endpoint 0 (bits 7-10)
			((Device->Speed == USB_LOW_SPEED) ? (1 << 13) : 0) | // S: Speed
			((ep.MaxPacketSize & 0x7FF) << 16);					 // MPS: Max Packet Size (bits 16-26)

		TD *setup = queue->AllocateTransferDescriptor();
		if (!setup)
		{
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}

		TD *data = nullptr;
		TD *status = queue->AllocateTransferDescriptor();
		if (!status)
		{
			queue->ReleaseTransferDescriptor(setup);
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}

		/* Allocate dummy TD for TDQueueTailPointer */
		TD *dummy = queue->AllocateTransferDescriptor();
		if (!dummy)
		{
			queue->ReleaseTransferDescriptor(status);
			queue->ReleaseTransferDescriptor(setup);
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}
		memset(dummy, 0, sizeof(TD));

		/* SETUP TD */
		setup->Control =
			(OHCI_TD_DP_SETUP << 19) |		  // DP: SETUP PID (bits 19-20)
			(OHCI_TD_DI_NO_INTERRUPT << 21) | // DI: No interrupt (bits 21-23)
			(OHCI_TD_T_DATA0 << 24) |		  // T: DATA0
			(0xE << 28);					  // CC: Not accessed (bits 28-31)

		setup->CurrentBufferPointer = (uint32_t)(uintptr_t)urb->Request;
		setup->BufferEnd = setup->CurrentBufferPointer + sizeof(USBDeviceRequest) - 1;
		RegisterTD(setup, urb, setup->CurrentBufferPointer, setup->BufferEnd, false);

		/* DATA phase (optional) */
		if (urb->Length)
		{
			data = queue->AllocateTransferDescriptor();
			if (!data)
			{
				ForgetTDMetadata(setup);
				queue->ReleaseTransferDescriptor(dummy);
				queue->ReleaseTransferDescriptor(status);
				queue->ReleaseTransferDescriptor(setup);
				queue->ReleaseEndpointDescriptor(ed);
				return ENOMEM;
			}

			data->Control =
				((urb->Request->bmRequestType.raw & 0x80) ? (OHCI_TD_DP_IN << 19) : (OHCI_TD_DP_OUT << 19)) | // IN/OUT
				(OHCI_TD_DI_NO_INTERRUPT << 21) |															  // DI: No interrupt
				(OHCI_TD_T_DATA1 << 24) |																	  // T: DATA1
				(0xE << 28);																				  // CC: Not accessed

			data->CurrentBufferPointer = (uint32_t)(uintptr_t)urb->Buffer;
			data->BufferEnd = data->CurrentBufferPointer + urb->Length - 1;
			RegisterTD(data, urb, data->CurrentBufferPointer, data->BufferEnd, true);
		}

		/* STATUS TD - opposite direction, interrupt on completion */
		status->Control =
			((urb->Request->bmRequestType.raw & 0x80) ? (OHCI_TD_DP_OUT << 19) : (OHCI_TD_DP_IN << 19)) | // Opposite direction
			(0 << 21) |																					  // DI: Interrupt immediately (bits 21-23)
			(OHCI_TD_T_DATA1 << 24) |																	  // T: DATA1
			(0xE << 28);																				  // CC: Not accessed

		status->CurrentBufferPointer = 0;
		status->BufferEnd = 0;
		RegisterTD(status, urb, 0, 0, false);

		/* Link TD chain */
		setup->NextTD = data ? (uintptr_t)data : (uintptr_t)status;
		if (data)
			data->NextTD = (uintptr_t)status;
		status->NextTD = (uintptr_t)dummy;

		/* Setup ED pointers - TDQueueTailPointer points to dummy */
		ed->HeadP = (uintptr_t)setup;
		ed->TDQueueTailPointer = (uintptr_t)dummy;
		ed->NextED = 0;

		/* Attach to Control list */
		LinkEDToList(regs, ed, true);

		/* Signal control list filled */
		mmoutl((void *)&regs->HcCommandStatus, OHCI_CMD_CLF); // CLF bit

		urb->PrivateData = ed;
		PendingTransfers.push_back({PendingTransfer::Control, urb, ed, dummy, false, 0});
		return 0;
	}

	int HCD::SubmitBulk(USBDevice *Device, USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		USBEndpoint &ep =
			Device->Endpoints[urb->EndpointAddress & 0x0F];

		ED *ed = queue->AllocateEndpointDescriptor();
		if (!ed)
			return ENOMEM;

		/* Fixed: Use Device Address */
		ed->Control =
			(Device->Endpoints[0].Address & 0x7F) |			  // FA: Function Address
			((urb->EndpointAddress & 0x0F) << 7) |			  // EN: Endpoint Number
			(((urb->EndpointAddress & 0x80) ? 2 : 1) << 11) | // D: Direction from endpoint
			((ep.MaxPacketSize & 0x7FF) << 16);				  // MPS

		TD *first = nullptr;
		TD *prev = nullptr;

		uint8_t *ptr = (uint8_t *)urb->Buffer;
		size_t remaining = urb->Length;

		while (remaining)
		{
			size_t chunk = std::min((size_t)ep.MaxPacketSize, remaining);

			TD *td = queue->AllocateTransferDescriptor();
			if (!td)
			{
				/* Cleanup on error */
				TD *cleanup = first;
				while (cleanup)
				{
					TD *next = (TD *)(uintptr_t)cleanup->NextTD;
					ForgetTDMetadata(cleanup);
					queue->ReleaseTransferDescriptor(cleanup);
					cleanup = next;
				}
				queue->ReleaseEndpointDescriptor(ed);
				return ENOMEM;
			}

			td->Control =
				((urb->EndpointAddress & 0x80) ? (OHCI_TD_DP_IN << 19) : (OHCI_TD_DP_OUT << 19)) | // Direction
				(OHCI_TD_DI_NO_INTERRUPT << 21) |												   // DI: No interrupt yet
				(OHCI_TD_T_DATA0 << 24) |														   // T: DATA0
				(0xE << 28);																	   // CC: Not accessed

			td->CurrentBufferPointer = (uint32_t)(uintptr_t)ptr;
			td->BufferEnd = td->CurrentBufferPointer + chunk - 1;
			td->NextTD = 0;
			RegisterTD(td, urb, td->CurrentBufferPointer, td->BufferEnd, true);

			if (!first)
				first = td;
			if (prev)
				prev->NextTD = (uintptr_t)td;

			prev = td;

			ptr += chunk;
			remaining -= chunk;
		}

		/* Set IOC on last TD */
		if (prev)
		{
			prev->Control &= ~(7 << 21); // Clear DI field
			prev->Control |= (0 << 21);	 // Interrupt immediately
		}

		/* Allocate dummy TD */
		TD *dummy = queue->AllocateTransferDescriptor();
		if (!dummy)
		{
			TD *cleanup = first;
			while (cleanup)
			{
				TD *next = (TD *)(uintptr_t)cleanup->NextTD;
				ForgetTDMetadata(cleanup);
				queue->ReleaseTransferDescriptor(cleanup);
				cleanup = next;
			}
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}
		memset(dummy, 0, sizeof(TD));

		if (prev)
			prev->NextTD = (uintptr_t)dummy;

		/* Setup ED - TDQueueTailPointer points to dummy */
		ed->HeadP = (uintptr_t)first;
		ed->TDQueueTailPointer = (uintptr_t)dummy;
		ed->NextED = 0;

		/* Attach to Bulk list */
		LinkEDToList(regs, ed, false);

		/* Signal bulk list filled */
		mmoutl((void *)&regs->HcCommandStatus, OHCI_CMD_BLF); // BLF bit

		urb->PrivateData = ed;
		PendingTransfers.push_back({PendingTransfer::Bulk, urb, ed, dummy, false, 0});
		return 0;
	}

	int HCD::SubmitInterrupt(USBDevice *Device, USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		uint8_t epNum = urb->EndpointAddress & 0x0F;
		USBEndpoint &ep = Device->Endpoints[epNum];

		ED *ed = queue->AllocateEndpointDescriptor();
		if (!ed)
			return ENOMEM;

		ed->Control =
			(Device->Endpoints[0].Address & 0x7F) |			  // FA: Function Address
			((epNum & 0x0F) << 7) |							  // EN: Endpoint Number
			(((urb->EndpointAddress & 0x80) ? 2 : 1) << 11) | // D: Direction
			((Device->Speed == USB_LOW_SPEED) ? (1 << 13) : 0) |
			((ep.MaxPacketSize & 0x7FF) << 16);

		TD *td = queue->AllocateTransferDescriptor();
		if (!td)
		{
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}

		TD *dummy = queue->AllocateTransferDescriptor();
		if (!dummy)
		{
			queue->ReleaseTransferDescriptor(td);
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}
		memset(dummy, 0, sizeof(TD));

		td->Control =
			((urb->EndpointAddress & 0x80) ? (OHCI_TD_DP_IN << 19) : (OHCI_TD_DP_OUT << 19)) |
			(0 << 21) | // IOC on completion
			(OHCI_TD_T_DATA0 << 24) |
			(0xE << 28);

		if (urb->Length != 0)
		{
			td->CurrentBufferPointer = (uint32_t)(uintptr_t)urb->Buffer;
			td->BufferEnd = td->CurrentBufferPointer + urb->Length - 1;
		}
		else
		{
			td->CurrentBufferPointer = 0;
			td->BufferEnd = 0;
		}
		td->NextTD = (uintptr_t)dummy;
		RegisterTD(td, urb, td->CurrentBufferPointer, td->BufferEnd, urb->Length != 0);

		ed->HeadP = (uintptr_t)td;
		ed->TDQueueTailPointer = (uintptr_t)dummy;

		uint8_t slot = epNum & 0x1F;
		LinkEDToPeriodicSlot(hcca, ed, slot);

		urb->PrivateData = ed;
		PendingTransfers.push_back({PendingTransfer::Periodic, urb, ed, dummy, false, slot});
		return 0;
	}

	int HCD::SubmitIsochronous(USBDevice *Device, USBRequestBlock *urb)
	{
		urb->Status = USB_REQ_PENDING;
		urb->ActualLength = 0;

		uint8_t epNum = urb->EndpointAddress & 0x0F;
		USBEndpoint &ep = Device->Endpoints[epNum];

		ED *ed = queue->AllocateEndpointDescriptor();
		if (!ed)
			return ENOMEM;

		ed->Control =
			(Device->Endpoints[0].Address & 0x7F) |			  // FA: Function Address
			((epNum & 0x0F) << 7) |							  // EN: Endpoint Number
			(((urb->EndpointAddress & 0x80) ? 2 : 1) << 11) | // D: Direction
			((Device->Speed == USB_LOW_SPEED) ? (1 << 13) : 0) |
			(1 << 15) | // F: Isochronous format
			((ep.MaxPacketSize & 0x7FF) << 16);

		ITD *itd = queue->AllocateIsochronousDescriptor();
		if (!itd)
		{
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}

		ITD *dummy = queue->AllocateIsochronousDescriptor();
		if (!dummy)
		{
			queue->ReleaseIsochronousDescriptor(itd);
			queue->ReleaseEndpointDescriptor(ed);
			return ENOMEM;
		}

		uint16_t frame = (uint16_t)(mminl((void *)&regs->HcFmNumber) & 0xFFFF);
		itd->Control = (0xEu << 28); // Not accessed
		itd->Control.StartingFrame(frame + 1);
		itd->Control.DelayInterrupt(0);
		itd->Control.FrameCount(0); // one packet

		if (urb->Length != 0 && urb->Buffer != nullptr)
		{
			uint32_t bp0 = (uint32_t)(uintptr_t)urb->Buffer;
			uint32_t be = bp0 + urb->Length - 1;

			itd->BufferPage0 = bp0 & ~0xFFFu;
			itd->BufferEnd = be;
			itd->OffsetPSW[0] = (uint16_t)(bp0 & 0x0FFFu);
		}
		else
		{
			itd->BufferPage0 = 0;
			itd->BufferEnd = 0;
			itd->OffsetPSW[0] = 0;
		}
		for (size_t i = 1; i < 8; ++i)
			itd->OffsetPSW[i] = 0xE000u; // unused packet entries
		itd->NextTD = (uintptr_t)dummy;
		RegisterITD(itd, urb, itd->BufferPage0, itd->BufferEnd, urb->Length);

		ed->HeadP = (uintptr_t)itd;
		ed->TDQueueTailPointer = (uintptr_t)dummy;

		uint8_t slot = epNum & 0x1F;
		LinkEDToPeriodicSlot(hcca, ed, slot);

		urb->PrivateData = ed;
		PendingTransfers.push_back({PendingTransfer::Periodic, urb, ed, dummy, true, slot});
		return 0;
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
		ED *ed = (ED *)urb->PrivateData;
		if (!ed)
			return EINVAL;

		ed->Control |= (1u << 14); // Skip
		ed->HeadP |= 1;			   // Halt

		urb->Status = USB_REQ_CANCELED;
		return 0;
	}

	HCD::HCD(PCI::PCIDevice &pciHeader)
		: Interrupts::Handler(pciHeader),
		  regs((OHCIRegisters *)pciHeader.GetBAR(0)),
		  Header(pciHeader)
	{
		debug("Found OHCI controller at %#lx", regs);
		mmoutl((void *)&regs->HcInterruptEnable, 0);
		mmoutl((void *)&regs->HcInterruptDisable, (1 << 31));

		uint32_t hcRevision = mminl((void *)&regs->HcRevision) & 0xFF;
		debug("Host Controller Revision: %d.%d (%#lx)", (hcRevision >> 4) & 0xF, hcRevision & 0xF, hcRevision);

		queue = new Queue;

		hcca = (HCCA *)KernelAllocator.RequestPage();
		memset(hcca, 0, sizeof(HCCA));
	}

	HCD::~HCD()
	{
		this->Stop();
		KernelAllocator.FreePage(hcca);
		delete queue;
	}
}
