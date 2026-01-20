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

	/* Table 5-9: eXtensible Host Controller Capability Registers */
	struct Capability
	{
		volatile uint8_t CAPLENGTH;	  // 00h - Capability Register Length
		volatile uint8_t __rsvd0;	  // 01h - Reserved
		volatile uint16_t HCIVERSION; // 02h - Interface Version Number

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

			BF_RO(uint8_t, NumberOfDeviceSlots, 0, 8);
			BF_RO(uint16_t, NumberOfInterrupters, 8, 11);
			BF_RO(uint8_t, NumberOfPorts, 24, 8);
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

			BF_RO(uint8_t, IsochronousSchedulingThreshold, 0, 4);
			BF_RO(uint8_t, EventRingSegmentTableMax, 4, 4);
			BF_RO(uint8_t, MaxScratchpadBuffersHi, 21, 5);
			BF_RO(uint8_t, ScratchpadRestore, 26, 1);
			BF_RO(uint8_t, MaxScratchpadBuffersLo, 27, 5);
			/* FIXME: this is wrong! double shifting! */
			uint16_t MaxScratchpadBuffers() { return (MaxScratchpadBuffersHi() << 5) | MaxScratchpadBuffersLo(); }
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

			BF_RO(uint8_t, U1DeviceExitLatency, 0, 8);
			BF_RO(uint8_t, U2DeviceExitLatency, 8, 8);
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

			BF_RO(uint8_t, _64bitAddressingCapability, 0, 1);
			BF_RO(uint8_t, BWNegotiationCapability, 1, 1);
			BF_RO(uint8_t, ContextSize, 2, 1);
			BF_RO(uint8_t, PortPowerControl, 3, 1);
			BF_RO(uint8_t, PortIndicators, 4, 1);
			BF_RO(uint8_t, LightHCResetCapability, 5, 1);
			BF_RO(uint8_t, LatencyToleranceMessagingCapability, 6, 1);
			BF_RO(uint8_t, NoSecondarySIDSupport, 7, 1);
			BF_RO(uint8_t, ParseAllEventData, 8, 1);
			BF_RO(uint8_t, StoppedShortPacketCapability, 9, 1);
			BF_RO(uint8_t, StoppedEDTLACapability, 10, 1);
			BF_RO(uint8_t, ContiguousFrameIDCapability, 11, 1);
			BF_RO(uint8_t, MaximumPrimaryStreamArraySize, 12, 4);
			BF_RO(uint8_t, xHCIExtendedCapabilitiesPointer, 16, 16);
		} HCCPARAMS1; // 10h - Capability Parameters 1

		volatile uint32_t DBOFF;	  // 14h - Doorbell Offset
		volatile uint32_t RTSOFF;	  // 18h - Runtime Register Space Offset
		volatile uint32_t HCCPARAMS2; // 1Ch - Capability Parameters 2
		volatile uint32_t __rsvd1;	  // CAPLENGTH-20h - Reserved

		// BF_RO_EX(DBOFF, uint32_t, DoorbellArrayOffset, 2, 30);
		// BF_RO_EX(RTSOFF, uint32_t, RuntimeRegisterSpaceOffset, 5, 27);
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

			BF_RW(uint8_t, RunStop, 0, 1);
			BF_RW(uint8_t, HostControllerReset, 1, 1);
			BF_RW(uint8_t, InterrupterEnable, 2, 1);
			BF_RW(uint8_t, HostSystemErrorEnable, 3, 1);
			BF_RW(uint8_t, LightHostControllerReset, 7, 1);
			BF_RW(uint8_t, ControllerSaveState, 8, 1);
			BF_RW(uint8_t, ControllerRestoreState, 9, 1);
			BF_RW(uint8_t, EnableWrapEvent, 10, 1);
			BF_RW(uint8_t, EnableU3MFINDEXStop, 11, 1);
			BF_RW(uint8_t, CEMEnable, 13, 1);
			BF_RO(uint8_t, ExtendedTBCEnable, 14, 1);
			BF_RO(uint8_t, ExtendedTBCTRBStatusEnable, 15, 1);
			BF_RW(uint8_t, VTIOEnable, 16, 1);
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

			BF_RO(uint8_t, HCHalted, 0, 1);
			BF_RW1C(uint8_t, HostSystemError, 2, 1);
			BF_RW1C(uint8_t, EventInterrupt, 3, 1);
			BF_RW1C(uint8_t, PortChangeDetect, 4, 1);
			BF_RO(uint8_t, SaveStateStatus, 8, 1);
			BF_RO(uint8_t, RestoreStateStatus, 9, 1);
			BF_RW1C(uint8_t, SaveRestoreError, 10, 1);
			BF_RO(uint8_t, ControllerNotReady, 11, 1);
			BF_RO(uint8_t, HostControllerError, 12, 1);
		} USBSTS; // 04h - USB Status

		volatile uint32_t PAGESIZE;	  // 08h - Page Size
		volatile uint32_t __rsvd0[2]; // 0C-13h - Reserved
		volatile uint32_t DNCTRL;	  // 14h - Device Notification Control

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

			BF_RW(uint8_t, RingCycleState, 0, 1);
			BF_RW1S(uint8_t, CommandStop, 1, 1);
			BF_RW1S(uint8_t, CommandAbort, 2, 1);
			BF_RO(uint8_t, CommandRingRunning, 3, 1);
			BF_PHYS_RW(uint64_t, CommandRingPointer, 6, 58);
		} CRCR; // 18h - Command Ring Control

		volatile uint64_t __rsvd1[2];  // 20-2Fh - Reserved
		volatile uint64_t DCBAAP;	   // 30h - Device Context Base Address Array Pointer
		volatile uint32_t CONFIG;	   // 38h - Configure
		volatile uint32_t __rsvd2[13]; // 3C-3FFh - Reserved

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
		/* Figure 5-20: Port Status and Control Register */
		union PortStatusAndControl
		{
			uint32_t __CCS : 1;
			uint32_t __PED : 1;
			uint32_t __RsvdZ0 : 1;
			uint32_t __OCA : 1;
			uint32_t __PR : 1;
			uint32_t __PLS : 4;
			uint32_t __PP : 1;
			uint32_t __PortSpeed : 4;
			uint32_t __PIC : 2;
			uint32_t __LWS : 1;
			uint32_t __CSC : 1;
			uint32_t __PEC : 1;
			uint32_t __WRC : 1;
			uint32_t __OCC : 1;
			uint32_t __PRC : 1;
			uint32_t __PLC : 1;
			uint32_t __CEC : 1;
			uint32_t __CAS : 1;
			uint32_t __WCE : 1;
			uint32_t __WDE : 1;
			uint32_t __WOE : 1;
			uint32_t __RsvdZ1 : 2;
			uint32_t __DR : 1;
			uint32_t __WPR : 1;
			DEFINE_BITWISE_TYPE(uint32_t, PortStatusAndControl);

			BF_ROS(uint8_t, CurrentConnectStatus, 0, 1);
			BF_RW1CS(uint8_t, PortEnabledDisabled, 1, 1);
			BF_RO(uint8_t, OverCurrentActive, 3, 1);
			BF_RW1S(uint8_t, PortReset, 4, 1);
			BF_RWS(uint8_t, PortLinkState, 5, 4);
			BF_RWS(uint8_t, PortPower, 9, 1);
			BF_ROS(uint8_t, PortSpeed, 10, 4);
			BF_RWS(uint8_t, PortIndicatorControl, 14, 2);
			BF_RW(uint8_t, PortLinkStateWriteStrobe, 16, 1);
			BF_RW1CS(uint8_t, ConnectStatusChange, 17, 1);
			BF_RW1CS(uint8_t, PortEnabledDisabledChange, 18, 1);
			BF_RW1CS(uint8_t, WarmPortResetChange, 19, 1);
			BF_RW1CS(uint8_t, OverCurrentChange, 20, 1);
			BF_RW1CS(uint8_t, PortResetChange, 21, 1);
			BF_RW1CS(uint8_t, PortLinkStateChange, 22, 1);
			BF_RW1CS(uint8_t, PortConfigErrorChange, 23, 1);
			BF_RO(uint8_t, ColdAttachStatus, 24, 1);
			BF_RWS(uint8_t, WakeOnConnectEnable, 25, 1);
			BF_RWS(uint8_t, WakeOnDisconnectEnable, 26, 1);
			BF_RWS(uint8_t, WakeOnOverCurrentEnable, 27, 1);
			BF_RO(uint8_t, DeviceRemovable, 30, 1);
			BF_RW1S(uint8_t, WarmPortReset, 31, 1);
		} PORTSC;

		union PortPowerManagementStatusAndControl
		{
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			DEFINE_BITWISE_TYPE(uint32_t, PortPowerManagementStatusAndControl);

			/* TODO */
		} PORTPMSC;

		union PortLinkInfo
		{
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			DEFINE_BITWISE_TYPE(uint32_t, PortLinkInfo);

			/* TODO */
		} PORTLI;

		union PortHardwareLPMControl
		{
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			uint32_t : 1;
			DEFINE_BITWISE_TYPE(uint32_t, PortHardwareLPMControl);

			/* TODO */
		} PORTHLPMC;
	};

	static_assert(offsetof(PortRegister, PORTSC) == 0x00);
	static_assert(offsetof(PortRegister, PORTPMSC) == 0x04);
	static_assert(offsetof(PortRegister, PORTLI) == 0x08);
	static_assert(offsetof(PortRegister, PORTHLPMC) == 0x0C);

	struct XHCIinterrupter
	{
		union InterrupterManagementRegister
		{
			struct
			{
				uint32_t __IP : 1;
				uint32_t __IE : 1;
				uint32_t __RsvdP : 30;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, InterrupterManagementRegister);

			BF_RW1C(uint8_t, InterruptPending, 0, 1);
			BF_RW(uint8_t, InterruptEnable, 1, 1);
		} IMAN;

		union InterrupterModerationRegister
		{
			struct
			{
				uint32_t __InterrupterModerationInterval : 16;
				uint32_t __InterrupterModerationCounter : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, InterrupterModerationRegister);

			BF_RW(uint16_t, InterruptModerationInterval, 0, 16);
			BF_RW(uint16_t, InterruptModerationCounter, 16, 16);
		} IMOD;

		union EventRingSegmentTableSizeRegister
		{
			struct
			{
				uint32_t __EventRingSegmentTableSize : 16;
				uint32_t __RsvdP : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, EventRingSegmentTableSizeRegister);

			BF_RW(uint16_t, EventRingSegmentTableSize, 0, 16);
		} ERSTSZ;

		volatile uint32_t __RsvdP;

		union EventRingSegmentTableBaseAddressRegister
		{
			struct
			{
				uint64_t __RsvdP : 6;
				uint64_t __EventRingSegmentTableBaseAddress : 58;
			} __packed;
			DEFINE_BITWISE_TYPE(uint64_t, EventRingSegmentTableBaseAddressRegister);

			BF_PHYS_RW(uint64_t, EventRingSegmentTableBaseAddress, 6, 58);
		} ERSTBA;

		union EventRingDequeuePointerRegister
		{
			struct
			{
				uint64_t __DESI : 3;
				uint64_t __EHB : 1;
				uint64_t __EventRingDequeuePointer : 60;
			} __packed;
			DEFINE_BITWISE_TYPE(uint64_t, EventRingDequeuePointerRegister);

			BF_RW(uint8_t, DequeueERSTSegmentIndex, 0, 3);
			BF_RW1C(uint8_t, EventHandlerBusy, 3, 1);
			BF_PHYS_RW(uint64_t, EventRingDequeuePointer, 3, 60);
		} ERDP;
	};

	static_assert(offsetof(XHCIinterrupter, IMAN) == 0x00);
	static_assert(offsetof(XHCIinterrupter, IMOD) == 0x04);
	static_assert(offsetof(XHCIinterrupter, ERSTSZ) == 0x08);
	static_assert(offsetof(XHCIinterrupter, __RsvdP) == 0x0C);
	static_assert(offsetof(XHCIinterrupter, ERSTBA) == 0x10);
	static_assert(offsetof(XHCIinterrupter, ERDP) == 0x18);

	struct Runtime
	{
		volatile uint32_t MFINDEX;	// 0000h - Microframe Index
		volatile uint32_t rsvd0[7]; // 0004h-001Fh - Reserved
									// Interrupter Register Sets at 0020h onwards (IR0-IR1023)
		XHCIinterrupter Interrupter[1024];
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

		BF_RO(uint8_t, CapabilityID, 0, 8);
		BF_RO(uint8_t, NextExtendedCapabilityPointer, 8, 8);
		BF_RO(uint16_t, CapabilitySpecific, 16, 16);
	};
	static_assert(sizeof(ExtendedCapabilityPointer) == 4);

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

	/* Table 7-10: Offset 10h to (PSIC*4)+10h */
	union ProtocolSpeedID
	{
		struct
		{
			uint32_t __PSIV : 4;
			uint32_t __PSIE : 2;
			uint32_t __PLT : 2;
			uint32_t __PFD : 1;
			uint32_t __RsvdP : 5;
			uint32_t __LP : 2;
			uint32_t __ProtocolSpeedIDMantissa : 16;
		} __packed;
		DEFINE_BITWISE_TYPE(uint32_t, ProtocolSpeedID);

		BF_RO(uint8_t, ProtocolSpeedIDValue, 0, 4);
		BF_RO(uint8_t, ProtocolSpeedIDExponent, 4, 2);
		BF_RO(uint8_t, PSIType, 6, 2);
		BF_RO(uint8_t, PSIFullDuplex, 8, 1);
		BF_RO(uint8_t, LinkProtocol, 14, 2);
		BF_RO(uint8_t, ProtocolSpeedIDMantissa, 16, 16);
	};

	/* Figure 7-1: xHCI Supported Protocol Capability */
	struct SupportedProtocolCapability
	{
		union ExtendedCapability
		{
			struct
			{
				uint32_t __CapabilityID : 8;
				uint32_t __NextCapabilityPointer : 8;
				uint32_t __RevisionMinor : 8;
				uint32_t __RevisionMajor : 8;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, ExtendedCapability);

			BF_RO(uint8_t, CapabilityID, 0, 8);
			BF_RO(uint8_t, NextCapabilityPointer, 8, 8);
			BF_RO(uint8_t, RevisionMinor, 16, 8);
			BF_RO(uint8_t, RevisionMajor, 24, 8);
		} C;

		volatile uint32_t NameString;

		union ProtocolCapabilityType
		{
			struct
			{
				uint32_t __CompatiblePortOffset : 8;
				uint32_t __CompatiblePortCount : 8;
				uint32_t __ProtocolDefined : 12;
				uint32_t __PSIC : 4;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, ProtocolCapabilityType);

			BF_RO(uint8_t, CompatiblePortOffset, 0, 8);
			BF_RO(uint8_t, CompatiblePortCount, 8, 8);
			BF_RO(uint16_t, ProtocolDefined, 16, 12);
			BF_RO(uint8_t, ProtocolSpeedIDCount, 28, 4);
		} PCT;

		union SlotType
		{
			struct
			{
				uint32_t __ProtocolSlotType : 4;
				uint32_t __RsvdP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, SlotType);

			BF_RO(uint8_t, ProtocolSlotType, 0, 4);
		} ST;

		ProtocolSpeedID PSI[3];
	};

	/* Figure 6-40: Event Ring Segment Table Entry */
	struct EventRingSegmentTableEntry
	{
		union RingSegmentBaseAddress_union
		{
			uint64_t __RsvdZ : 6;
			uint64_t __RingSegmentBaseAddress : 58;
			DEFINE_BITWISE_TYPE(uint64_t, RingSegmentBaseAddress_union);

			BF_PHYS_RW(uint64_t, RingSegmentBaseAddress, 6, 58);
		} RSBA;

		union RingSegmentSize_union
		{
			uint32_t __RingSegmentSize : 16;
			uint32_t __RsvdZ : 16;
			DEFINE_BITWISE_TYPE(uint32_t, RingSegmentSize_union);

			BF_RW(uint16_t, RingSegmentSize, 0, 16);
		} RSS;

		volatile uint32_t __RsvdZ;
	};

	class Port
	{
	private:
		PortRegister *Reg;
		SupportedProtocolCapability *Proto;

	public:
		bool IsPowered();
		bool IsEnabled();
		bool IsConnected();
		bool IsOverCurrent();

		int PowerOn();
		int Reset();

		Port(PortRegister *Register, SupportedProtocolCapability *Protocol);
		~Port() = default;
	};

	class EventRing
	{
	public:
		struct Segment
		{
			TRB *trb = nullptr;
			size_t Size = 0;
		};

	private:
		std::vector<Segment> Segments;
		EventRingSegmentTableEntry *Table = nullptr;
		XHCIinterrupter *Interrupter = nullptr;
		size_t TableSize = 0;
		size_t DequeuePtr = 0;
		bool RingCycleState = true;

		void UpdateERDP();
		TRB *DequeueTRB();

	public:
		bool HasUnprocessedEvents();
		void DequeueEvents(std::vector<TRB *> &TRBs);
		void FlushUnprocessedEvents();

		EventRing(size_t Count, XHCIinterrupter *Interrupter);
		~EventRing();
	};

	class DoorbellManager
	{
	private:
		Doorbell *DBs;

	public:
		void Ring(uint8_t Slot, uint8_t Target);
		void RingCommand();
		void RingControlEndpoint(uint8_t Slot);

		DoorbellManager(Doorbell *db);
		~DoorbellManager() = default;
	};

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		PCI::PCIDevice Header;
		uintptr_t MMIOBase;
		Capability *Cap;
		Operational *Op;
		Runtime *Rt;
		PortRegister *Por;
		Doorbell *Db;
		ExtendedCapabilityPointer *Ext;

		uint8_t DeviceSlots = 32;
		uint16_t ScratchpadCount = 0;
		XHCIinterrupter *Interrupter = nullptr;
		std::vector<Port> Ports;
		std::vector<SupportedProtocolCapability *> SupportedProtocols;

		std::vector<CommandCompletionEventTRB *> CompletedCmds;
		volatile uint8_t CommandIRQComplete = 0;

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
			size_t MaxTRBCount = 0;
			size_t EnqueuePtr = 0;
			bool RingCycleState = true;

			void QueueTRB(TRB *trb);
			CommandCompletionEventTRB *_EnqueueTRB(TRB *trb);

		public:
			decltype(Buffer) GetBuffer() { return Buffer; }
			decltype(RingCycleState) GetycleState() { return RingCycleState; }
			void AddTRB(auto trb) { QueueTRB((TRB *)trb); }
			CommandCompletionEventTRB *EnqueueTRB(auto trb) { return _EnqueueTRB((TRB *)trb); }

			CommandRing(HCD &Controller, size_t Count);
			~CommandRing();
		} CmdRing = CommandRing(*this, 256);

		std::shared_ptr<EventRing> EvRing;
		std::shared_ptr<DoorbellManager> DbManager;

		void OnInterruptReceived(CPU::TrapFrame *Frame) final;

		bool TakeOwnership();
		void InitializeProtocols();

	public:
		CommandCompletionEventTRB *SendCommand(TRB *trb, size_t TimeoutMs = 500);

		int Reset();
		int Start();
		int Stop();
		int Detect();
		int Poll();
		HCD(PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
