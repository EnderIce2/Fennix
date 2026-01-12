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

#include "trb.hpp"

namespace Driver::ExtensibleHostControllerInterface
{
	/* Table 7-2: xHCI Extended Capability Codes */
	enum ExtendedCapabilityCodes
	{
		/**
		 * This capability provides the xHCI Pre-OS to OS
		 * Handoff Synchronization support capability.
		 */
		EXTCAP_USBLegacySupport = 1,

		/**
		 * This capability enumerates the protocols and
		 * revisions supported by this xHC. At least one of
		 * these capability structures is required for all xHC
		 * implementations.
		 */
		EXTCAP_SupportedProtocol = 2,

		/**
		 * This capability is required for all xHC non-PCI
		 * implementations.
		 */
		EXTCAP_ExtendedPowerManagement = 3,

		/**
		 * This capability is optional-normative for xHC
		 * implementations that require hardware
		 * virtualization support.
		 */
		EXTCAP_IOVirtualization = 4,

		/**
		 * Either this or the xHCI Extended Message
		 * Interrupt capability is required for all xHC non-
		 * PCI implementations.
		 */
		EXTCAP_MessageInterrupt = 5,

		/**
		 * This capability is optional-normative for xHC
		 * implementations that require local memory
		 * support.
		 */
		EXTCAP_LocalMemory = 6,

		/**
		 * This capability is optional-normative for xHC
		 * implementations and describes the xHCI USB
		 * Debug Capability.
		 */
		EXTCAP_USBDebugCapability = 10,

		/**
		 * Either this or the xHCI Message Interrupt
		 * capability is required for all xHC non-PCI
		 * implementations.
		 */
		EXTCAP_ExtendedMessageInterrupt = 17,

		/* 192-155: These IDs are available for vendor
			specific extensions to the xHCI. */
	};

	struct Capability
	{
		uint8_t CAPLENGTH;	 // 00h - Capability Register Length
		uint8_t __rsvd0;	 // 01h - Reserved
		uint16_t HCIVERSION; // 02h - Interface Version Number

		union HCSPARAMS1_UNION
		{
			struct
			{
				uint32_t __MaxDeviceSlots : 8;
				uint32_t __MaxInterrupters : 11;
				uint32_t __Rsvd : 5;
				uint32_t __MaxPorts : 8;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, HCSPARAMS1_UNION);

			uint8_t MaxDeviceSlots() { return (raw >> 0) & 0xFF; }
			uint16_t MaxInterrupters() { return (raw >> 8) & 0x7FF; }
			uint8_t MaxPorts() { return (raw >> 24) & 0xFF; }
		} HCSPARAMS1; // 04h - Structural Parameters 1

		union HCSPARAMS2_UNION
		{
			struct
			{
				uint32_t __IsochronousSchedulingThreshold : 4;
				uint32_t __EventRingSegmentTableMax : 4;
				uint32_t __Rsvd : 13;
				uint32_t __MaxScratchpadBuffersHi : 5;
				uint32_t __ScratchpadRestore : 1;
				uint32_t __MaxScratchpadBuffersLo : 5;
			};
			DEFINE_BITWISE_TYPE(uint32_t, HCSPARAMS2_UNION);

			uint8_t IST() { return (raw >> 0) & 0xF; }
			uint8_t ERSTMax() { return (raw >> 4) & 0xF; }
			uint8_t SPR() { return (raw >> 26) & 0x1; }
			uint16_t MaxScratchpadBuffers() { return (((raw >> 21) & 0x1F) << 5) | ((raw >> 27) & 0x1F); }
		} HCSPARAMS2; // 08h - Structural Parameters 2

		union HCSPARAMS3_UNION
		{
			struct
			{
				uint32_t __U1DeviceExitLatency : 8;
				uint32_t __U2DeviceExitLatency : 8;
				uint32_t __Rsvd : 16;
			};
			DEFINE_BITWISE_TYPE(uint32_t, HCSPARAMS3_UNION);

			uint8_t U1DeviceExitLatency() { return (raw >> 0) & 0xFF; }
			uint8_t U2DeviceExitLatency() { return (raw >> 8) & 0xFF; }
		} HCSPARAMS3; // 0Ch - Structural Parameters 3

		/* Table 5-13: Host Controller Capability 1 Parameters */
		union HCCPARAMS1_UNION
		{
			struct
			{
				uint32_t __AC64 : 1;
				uint32_t __BNC : 1;
				uint32_t __CSZ : 1;
				uint32_t __PPC : 1;
				uint32_t __PIND : 1;
				uint32_t __LHRC : 1;
				uint32_t __LTC : 1;
				uint32_t __NSS : 1;
				uint32_t __PAE : 1;
				uint32_t __SPC : 1;
				uint32_t __SEC : 1;
				uint32_t __CFC : 1;
				uint32_t __MaxPSASize : 4;
				uint32_t __xHCIExtendedCapacitiesPointer : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, HCCPARAMS1_UNION);

			uint16_t GetExtendedCapabilitiesPointer() { return (raw >> 16) & 0xFFFF; }
		} HCCPARAMS1; // 10h - Capability Parameters 1

		uint32_t DBOFF;		 // 14h - Doorbell Offset
		uint32_t RTSOFF;	 // 18h - Runtime Register Space Offset
		uint32_t HCCPARAMS2; // 1Ch - Capability Parameters 2
		uint32_t __rsvd1;	 // CAPLENGTH-20h - Reserved
	};

	static_assert(offsetof(Capability, CAPLENGTH) == 0x00);
	static_assert(offsetof(Capability, HCIVERSION) == 0x02);
	static_assert(offsetof(Capability, HCSPARAMS1) == 0x04);
	static_assert(offsetof(Capability, HCSPARAMS2) == 0x08);
	static_assert(offsetof(Capability, HCSPARAMS3) == 0x0C);
	static_assert(offsetof(Capability, HCCPARAMS1) == 0x10);
	static_assert(offsetof(Capability, DBOFF) == 0x14);
	static_assert(offsetof(Capability, RTSOFF) == 0x18);
	static_assert(offsetof(Capability, HCCPARAMS2) == 0x1C);

	struct Operational
	{
		/* 5.4.1 USB Command Register */
		union USBCMD_UNION
		{
			struct
			{
				uint32_t __RS : 1;
				uint32_t __HCRST : 1;
				uint32_t __INTE : 1;
				uint32_t __HSEE : 1;
				uint32_t __RsvdP0 : 3;
				uint32_t __LHCRST : 1;
				uint32_t __CSS : 1;
				uint32_t __CRS : 1;
				uint32_t __EWE : 1;
				uint32_t __EU3S : 1;
				uint32_t __RsvdP1 : 1;
				uint32_t __CME : 1;
				uint32_t __ETE : 1;
				uint32_t __TSCEN : 1;
				uint32_t __VTIOEN : 1;
				uint32_t __RsvdP2 : 15;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, USBCMD_UNION);

			uint8_t RunStop() { return (raw >> 0) & 0x1; }
			void RunStop(uint8_t value) { raw = (raw & ~0x1) | (value & 0x1); }

			uint8_t HostControllerReset() { return (raw >> 1) & 0x1; }
			void HostControllerReset(uint8_t value) { raw = (raw & ~0x2) | ((value & 0x1) << 1); }
		} USBCMD; // 00h - USB Command

		/* 5.4.2 USB Status Register */
		union USBSTS_UNION
		{
			struct
			{

				uint32_t __HCH : 1;
				uint32_t __RsvdZ0 : 1;
				uint32_t __HSE : 1;
				uint32_t __EINT : 1;
				uint32_t __PCB : 1;
				uint32_t __RsvdZ1 : 3;
				uint32_t __SSS : 1;
				uint32_t __RSS : 1;
				uint32_t __SRE : 1;
				uint32_t __CNR : 1;
				uint32_t __HCE : 1;
				uint32_t __RsvdZ2 : 18;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, USBSTS_UNION);

			uint8_t HCHalted() { return (raw >> 0) & 0x1; }

			uint8_t ControllerNotReady() { return (raw >> 11) & 0x1; }
		} USBSTS; // 04h - USB Status

		uint32_t PAGESIZE;	 // 08h - Page Size
		uint32_t __rsvd0[2]; // 0C-13h - Reserved
		uint32_t DNCTRL;	 // 14h - Device Notification Control

		/* 5.4.5 Command Ring Control Register */
		union CRCR_UNION
		{
			struct
			{
				uint64_t __RCS : 1;
				uint64_t __CS : 1;
				uint64_t __CA : 1;
				uint64_t __CRR : 1;
				uint64_t __RsvdP : 2;
				uint64_t __CRP : 58;
			} __packed;
			DEFINE_BITWISE_TYPE(uint64_t, CRCR_UNION);

			uint8_t RingCycleState() { return (raw >> 0) & 0x1; }
			void RingCycleState(uint8_t value) { raw = (raw & ~0x1) | (value & 0x1); }

			uint8_t CommandStop() { return (raw >> 1) & 0x1; }
			void CommandStop(uint8_t value) { raw = (raw & ~0x2) | ((value & 0x1) << 1); }

			uint8_t CommandAbort() { return (raw >> 2) & 0x1; }
			void CommandAbort(uint8_t value) { raw = (raw & ~0x4) | ((value & 0x1) << 2); }

			uint8_t CommandRingRunning() { return (raw >> 3) & 0x1; }

			uint64_t CommandRingPointer() { return (raw >> 6) & 0x03FFFFFFFFFFFFFFULL; }
			void CommandRingPointer(uint64_t value) { raw = (raw & 0x3FULL) | (value & 0xFFFFFFFFFFFFFFC0ULL); }
		} CRCR; // 18h - Command Ring Control

		uint64_t __rsvd1[2];  // 20-2Fh - Reserved
		uint64_t DCBAAP;	  // 30h - Device Context Base Address Array Pointer
		uint32_t CONFIG;	  // 38h - Configure
		uint32_t __rsvd2[13]; // 3C-3FFh - Reserved

		// Port Register Set at 400-13FFh
	};

	static_assert(offsetof(Operational, USBCMD) == 0x00);
	static_assert(offsetof(Operational, USBSTS) == 0x04);
	static_assert(offsetof(Operational, PAGESIZE) == 0x08);
	static_assert(offsetof(Operational, DNCTRL) == 0x14);
	static_assert(offsetof(Operational, CRCR) == 0x18);
	static_assert(offsetof(Operational, DCBAAP) == 0x30);
	static_assert(offsetof(Operational, CONFIG) == 0x38);

	struct PortRegister
	{
		uint32_t PORTSC;	// 0h - Port Status and Control
		uint32_t PORTPMSC;	// 4h - Port Power Management Status and Control
		uint32_t PORTLI;	// 8h - Port Link Info
		uint32_t PORTHLPMC; // Ch - Port Hardware LPM Control
	} __packed;

	static_assert(offsetof(PortRegister, PORTSC) == 0x00);
	static_assert(offsetof(PortRegister, PORTPMSC) == 0x04);
	static_assert(offsetof(PortRegister, PORTLI) == 0x08);
	static_assert(offsetof(PortRegister, PORTHLPMC) == 0x0C);

	struct Runtime
	{
		uint32_t MFINDEX;  // 0000h - Microframe Index
		uint32_t rsvd0[7]; // 0004h-001Fh - Reserved
						   // Interrupter Register Sets at 0020h onwards (IR0-IR1023)
		struct XHCIinterrupter
		{
			union IMAN_UNION
			{
				struct
				{
					uint32_t IP : 1;
					uint32_t IE : 1;
					uint32_t __reserved0 : 30;
				} __packed;
				DEFINE_BITWISE_TYPE(uint32_t, IMAN_UNION);
			} IMAN;

			union IMOD_UNION
			{
				struct
				{
					uint32_t IMODI : 16;
					uint32_t IMODC : 16;
				} __packed;
				DEFINE_BITWISE_TYPE(uint32_t, IMOD_UNION);
			} IMOD;

			union ERSTSZ_UNION
			{
				struct
				{
					uint32_t ERSTSZ : 16;
					uint32_t __reserved0 : 16;
				} __packed;
				DEFINE_BITWISE_TYPE(uint32_t, ERSTSZ_UNION);
			} ERSTSZ;

			uint32_t __RsvdP;

			union ERSTBA_UNION
			{
				struct
				{
					uint64_t __reserved0 : 6;
					uint64_t ERSTBAR : 58;
				} __packed;
				DEFINE_BITWISE_TYPE(uint64_t, ERSTBA_UNION);
			} ERSTBA;

			union ERDP_UNION
			{
				struct
				{
					uint64_t DESI : 3;
					uint64_t EHB : 1;
					uint64_t ERDP : 60;
				} __packed;
				DEFINE_BITWISE_TYPE(uint64_t, ERDP_UNION);
			} ERDP;
		} Interrupter[];

		static_assert(offsetof(XHCIinterrupter, IMAN) == 0x00);
		static_assert(offsetof(XHCIinterrupter, IMOD) == 0x04);
		static_assert(offsetof(XHCIinterrupter, ERSTSZ) == 0x08);
		static_assert(offsetof(XHCIinterrupter, __RsvdP) == 0x0C);
		static_assert(offsetof(XHCIinterrupter, ERSTBA) == 0x10);
		static_assert(offsetof(XHCIinterrupter, ERDP) == 0x18);
	};

	static_assert(offsetof(Runtime, MFINDEX) == 0x0000);
	static_assert(offsetof(Runtime, Interrupter) == 0x0020);

	union Doorbell
	{
		/* Figure 5-29: Doorbell Register */
		struct
		{
			uint32_t DBTarget : 8;
			uint32_t __RsvdZ : 8;
			uint32_t DBTaskID : 16;
		} __packed;
		DEFINE_BITWISE_TYPE(uint32_t, Doorbell);
	};

	/* Table 7-1: Format of xHCI Extended Capability Pointer Register */
	union ExtendedCapabilityPointer
	{
		struct
		{
			uint32_t __CapabilityID : 8;
			uint32_t __NextExtendedCapabilityPointer : 8;
			uint32_t __CapabilitySpecific : 16;
		} __packed;
		DEFINE_BITWISE_TYPE(uint32_t, ExtendedCapabilityPointer);

		uint8_t CapabilityID() { return (raw >> 0) & 0xFF; }
		uint8_t NextExtendedCapabilityPointer() { return (raw >> 8) & 0xFF; }
		uint16_t CapabilitySpecific() { return (raw >> 16) & 0xFFFF; }
	};

	/* Table 7-3: HC Extended Capability Registers */
	struct LegacySupportCapability
	{
		/* Table 7-4: USB Legacy Support Extended Capability */
		union USBLEGSUP_UNION
		{
			struct
			{
				uint32_t CapabilityID : 8;
				uint32_t NextCapabilityPointer : 8;
				uint32_t BIOSOwnedSemaphore : 1;
				uint32_t __RsvdP0 : 7;
				uint32_t OSOwnedSemaphore : 1;
				uint32_t __RsvdP1 : 7;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, USBLEGSUP_UNION);
		} USBLEGSUP;

		/* Table 7-5: USB Legacy Support Control/Status */
		union USBLEGCTLSTS_UNION
		{
			struct
			{
				uint32_t USBSMIEnable : 1;
				uint32_t __RsvdP0 : 3;
				uint32_t SMIOnHostSystemErrorEnable : 1;
				uint32_t __RsvdP1 : 8;
				uint32_t SMIOnOSOwnershipEnable : 1;
				uint32_t SMIOnPCICommandEnable : 1;
				uint32_t SMIOnBAREnable : 1;
				uint32_t SMIOnEventInterrupt : 1;
				uint32_t __RsvdP2 : 3;
				uint32_t SMIOnHostSystemError : 1;
				uint32_t __RsvdZ3 : 8;
				uint32_t SMIOnOSOwnershipChange : 1;
				uint32_t SMIOnPCICommand : 1;
				uint32_t SMIOnBAR : 1;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, USBLEGCTLSTS_UNION);
		} USBLEGCTLSTS;
	};

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		PCI::PCIDevice Header;
		uintptr_t MMIOBase;
		Capability *Cap;
		Operational *Op;
		Runtime *Rt;
		PortRegister *Ports;
		Doorbell *Db;
		ExtendedCapabilityPointer *Ext;

		uint8_t DeviceSlots = 32;

		struct
		{
			uint64_t *DCBAAP = 0;
			uint64_t *ScratchpadBuffers = 0;
		} Allocations;

		class CommandRing
		{
		private:
			HCD &hc;
			TRB *Buffer = nullptr;
			size_t MaxSize = 0;

		public:
			decltype(Buffer) GetBuffer() { return Buffer; }

			CommandRing(HCD &Controller);
			~CommandRing();
		} CmdRing = CommandRing(*this);

		void OnInterruptReceived(CPU::TrapFrame *Frame) final;

		bool TakeOwnership();

	public:
		int Reset();
		int Start(bool WaitForStart);
		int Stop();
		int Detect();
		int Poll();
		HCD(PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
