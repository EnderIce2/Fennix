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

#pragma once

#include <bw.h>
#include <driver.hpp>
#include <usb.hpp>

namespace Driver::OpenHostControllerInterface
{
	struct OHCIRegisters
	{
		volatile uint32_t HcRevision;		  // 0x00  Revision Number
		volatile uint32_t HcControl;		  // 0x04  Operating Mode
		volatile uint32_t HcCommandStatus;	  // 0x08  Command & Status
		volatile uint32_t HcInterruptStatus;  // 0x0C  Interrupt Status
		volatile uint32_t HcInterruptEnable;  // 0x10  Interrupt Enable
		volatile uint32_t HcInterruptDisable; // 0x14  Interrupt Disable

		volatile uint32_t HcHCCA;			 // 0x18  HCCA Base Address
		volatile uint32_t HcPeriodCurrentED; // 0x1C  Current Periodic ED

		volatile uint32_t HcControlHeadED;	  // 0x20  Control Head ED
		volatile uint32_t HcControlCurrentED; // 0x24  Control Current ED

		volatile uint32_t HcBulkHeadED;	   // 0x28  Bulk Head ED
		volatile uint32_t HcBulkCurrentED; // 0x2C  Bulk Current ED

		volatile uint32_t HcDoneHead; // 0x30  Done Queue Head

		volatile uint32_t HcFmInterval;	 // 0x34  Frame Interval
		volatile uint32_t HcFmRemaining; // 0x38  Frame Remaining
		volatile uint32_t HcFmNumber;	 // 0x3C  Frame Number

		volatile uint32_t HcPeriodicStart; // 0x40  Periodic Start
		volatile uint32_t HcLSThreshold;   // 0x44  Low-Speed Threshold

		volatile uint32_t HcRhDescriptorA; // 0x48  Root Hub Descriptor A
		volatile uint32_t HcRhDescriptorB; // 0x4C  Root Hub Descriptor B
		volatile uint32_t HcRhStatus;	   // 0x50  Root Hub Status

		volatile uint32_t HcRhPortStatus1; // 0x54  Port 1 Status/Control
		volatile uint32_t HcRhPortStatus2; // 0x58  Port 2 Status/Control
										   // (Port 2 may be unusable depending on silicon)
	} __packed;

	/* Figure 4-3: Isochronous TD Format */
	struct ITD
	{
		union ITDControl
		{
			struct
			{
				uint32_t __SF : 16;
				uint32_t __rsvd0 : 5;
				uint32_t __DI : 3;
				uint32_t __FC : 3;
				uint32_t __rsvd1 : 1;
				uint32_t __CC : 4;
			};
			DEFINE_BITWISE_TYPE(uint32_t, ITDControl);

			BF_RW(uint16_t, StartingFrame, 0, 16);
			BF_RW(uint8_t, DelayInterrupt, 21, 3);
			BF_RW(uint8_t, FrameCount, 24, 3);
			BF_RO(uint8_t, ConditionCode, 28, 4);
		} Control;

		volatile uint32_t BufferPage0;
		volatile uint32_t NextTD;
		volatile uint32_t BufferEnd;
		volatile uint16_t OffsetPSW[8];
	} __aligned(32);

	static_assert(sizeof(ITD) == 32);

	struct TD
	{
		union TDControl
		{
			struct
			{
				uint32_t __rsvd : 18;
				uint32_t __R : 1;
				uint32_t __DP : 2;
				uint32_t __DI : 3;
				uint32_t __T : 2;
				uint32_t __EC : 2;
				uint32_t __CC : 4;
			};
			DEFINE_BITWISE_TYPE(uint32_t, TDControl);

			BF_RW(uint8_t, bufferRounding, 18, 1);

			/**
			 * Direction/PID:
			 *   00b - SETUP     to endpoint
			 *   01b - OUT       to endpoint
			 *   10b - IN      from endpoint
			 *   11b - Reserved
			 */
			BF_RW(uint8_t, DirectionPID, 19, 2);
			BF_RW(uint8_t, DelayInterrupt, 21, 3);
			BF_RW(uint8_t, DataToggle, 24, 2);
			BF_RO(uint8_t, ErrorCount, 26, 2);
			BF_RO(uint8_t, ConditionCode, 28, 4);
		} Control;

		volatile uint32_t CurrentBufferPointer;
		volatile uint32_t NextTD;
		volatile uint32_t BufferEnd;
	} __aligned(16);

	/* Table 4-1: Field Definitions for Endpoint Descriptor */
	struct ED
	{
		union EDControl
		{
			struct
			{
				uint32_t __FA : 7;
				uint32_t __EN : 4;
				uint32_t __D : 2;
				uint32_t __S : 1;
				uint32_t __K : 1;
				uint32_t __F : 1;
				uint32_t __MPS : 11;
				uint32_t __rsvd : 5;
			};
			DEFINE_BITWISE_TYPE(uint32_t, EDControl);

			BF_RW(uint8_t, FunctionAddress, 0, 7);
			BF_RW(uint8_t, EndpointNumber, 7, 4);

			/**
			 * Direction:
			 *   00b - Get direction From TD
			 *   01b - OUT
			 *   10b - IN
			 *   11b - Get direction From TD
			 */
			BF_RW(uint8_t, Direction, 11, 2);
			BF_RW(uint8_t, Speed, 13, 1);
			BF_RW(uint8_t, sKip, 14, 1);
			BF_RW(uint8_t, Format, 15, 1);
			BF_RW(uint16_t, MaximumPacketSize, 16, 11);
		} Control;

		volatile uint32_t TDQueueTailPointer;

		union EDHeadP
		{
			struct
			{
				uint32_t __H : 1;
				uint32_t __C : 1;
				uint32_t __zero : 2;
				uint32_t __HeadP : 28;
			};
			DEFINE_BITWISE_TYPE(uint32_t, EDHeadP);

			BF_RW(uint8_t, Halted, 0, 1);
			BF_RW(uint8_t, toggleCarry, 1, 1);
			BF_PHYS_RW(uintptr_t, TDQueueHeadPointer, 4, 28);
		} HeadP;
		volatile uint32_t NextED;
	} __aligned(16);

	struct HCCA
	{
		uint32_t InterruptTable[32];
		uint16_t FrameNumber;
		uint16_t Pad1;
		uint32_t DoneHead;
		uint8_t Reserved[116];
	} __packed __aligned(256);

	class Queue
	{
	private:
		TD *TDPool;
		ITD *ITDPool;
		ED *EDPool;

		bool TDUsed[64];
		bool ITDUsed[64];
		bool EDUsed[32];

		size_t TDCount = 64;
		size_t EDCount = 32;

	public:
		ED *AllocateEndpointDescriptor();
		int ReleaseEndpointDescriptor(ED *ed);

		TD *AllocateTransferDescriptor();
		int ReleaseTransferDescriptor(TD *td);
		ITD *AllocateIsochronousDescriptor();
		int ReleaseIsochronousDescriptor(ITD *itd);

		Queue();
		~Queue();
	};

	class HCD : public Interrupt::Handler, public USBController
	{
	private:
		OHCIRegisters *regs;
		HCCA *hcca;
		PCI::PCIDevice Header;

		int OnInterruptReceived(CPU::TrapFrame *Frame) final;

		int SubmitControl(USBDevice *Device, USBRequestBlock *urb);
		int SubmitBulk(USBDevice *Device, USBRequestBlock *urb);
		int SubmitInterrupt(USBDevice *Device, USBRequestBlock *urb);
		int SubmitIsochronous(USBDevice *Device, USBRequestBlock *urb);

	public:
		Queue *queue = nullptr;

		int Reset();
		int Start();
		int Stop();
		int Detect();

		int Submit(USBDevice *Device, USBRequestBlock *urb);
		int Cancel(USBDevice *Device, USBRequestBlock *urb);

		HCD(PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
