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

	struct TD
	{
		union LINK_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, LINK_UNION);
		} LINK;
		static_assert(sizeof(LINK) == sizeof(uint32_t));

		union CS_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, CS_UNION);
		} CS;
		static_assert(sizeof(CS) == sizeof(uint32_t));

		union TOKEN_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, TOKEN_UNION);
		} TOKEN;
		static_assert(sizeof(TOKEN) == sizeof(uint32_t));

		union BUFFER_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, BUFFER_UNION);
		} BUFFER;
		static_assert(sizeof(BUFFER) == sizeof(uint32_t));

		/* The last 4 DWords of the Transfer Descriptor are reserved for use by software.
			  - UHCI Design Guide 1.1 @ 3.2.5 */
		uint8_t __software[16];
	};

	struct ED
	{
		union HEAD_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, HEAD_UNION);
		} HEAD;
		static_assert(sizeof(HEAD) == sizeof(uint32_t));

		union ELEMENT_UNION
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(std::atomic_uint32_t, uint32_t, ELEMENT_UNION);
		} ELEMENT;
		static_assert(sizeof(ELEMENT) == sizeof(uint32_t));

		/* same as TD but QH has 64 bytes */
		uint8_t __software[56];
	};

	class Queue
	{
	private:
		TD *TDPool __aligned(0x10);
		ED *EDPool __aligned(0x10);

		// FrameListPointer *FrameList;
		size_t Frames = 1024;
		size_t CurrentFrame = 0;
		size_t Subframes = 1;
		size_t MaxBandwidth = 900;
		size_t MaxQHs = 8;
		size_t MaxTDs = 32;
		ED *CurrentQueue;

	public:
		UniversalSerialBus::Scheduler *sched;

		ED *AllocateQueueHead();
		int ReleaseQueueHead(ED *qh);
		TD *AllocateTransferDescriptor();
		int ReleaseTransferDescriptor(TD *td);
		Queue();
		~Queue();
	};

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		OHCIRegisters *regs;
		PCI::PCIDevice Header;

		void OnInterruptReceived(CPU::TrapFrame *Frame) final;

	public:
		Queue *queue = nullptr;

		int Reset();
		int Start(bool WaitForStart);
		int Stop();
		int Detect();
		int Poll();
		HCD(uintptr_t base, PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
