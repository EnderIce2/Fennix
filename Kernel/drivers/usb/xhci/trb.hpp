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

namespace Driver::ExtensibleHostControllerInterface
{
	/* Table 6-91: TRB Type Definitions */
	enum TRBTypeDefinitions
	{
		TRBT_Normal = 1,
		TRBT_SetupStage = 2,
		TRBT_DataStage = 3,
		TRBT_StatusStage = 4,
		TRBT_Isoch = 5,
		TRBT_Link = 6,
		TRBT_EventData = 7,
		TRBT_NoOp = 8,
		TRBT_EnableSlotCommand = 9,
		TRBT_DisableSlotCommand = 10,
		TRBT_AddressDeviceCommand = 11,
		TRBT_ConfigureEndpointCommand = 12,
		TRBT_EvaluateContextCommand = 13,
		TRBT_ResetEndpointCommand = 14,
		TRBT_StopEndpointCommand = 15,
		TRBT_SetTRDequeuePointerCommand = 16,
		TRBT_ResetDeviceCommand = 17,
		TRBT_ForceEventCommand = 18,
		TRBT_NegotiateBandwidthCommand = 19,
		TRBT_SetLatencyToleranceValueCommand = 20,
		TRBT_GetPortBandwidthCommand = 21,
		TRBT_ForceHeaderCommand = 22,
		TRBT_NoOpCommand = 23,
		TRBT_GetExtendedPropertyCommand = 24,
		TRBT_SetExtendedPropertyCommand = 25,
		TRBT_TransferEvent = 32,
		TRBT_CommandCompletionEvent = 33,
		TRBT_PortStatusChangeEvent = 34,
		TRBT_BandwidthRequestEvent = 35,
		TRBT_DoorbellEvent = 36,
		TRBT_HostControllerEvent = 37,
		TRBT_DeviceNotificationEvent = 38,
		TRBT_MFINDEXWrapEvent = 39
	};

	struct TRB
	{
		uint32_t Parameter[2];
		uint32_t Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;	// Cycle Bit
				uint32_t __ENT : 1; // Evaluate Next TRB
				uint32_t __reserved : 8;
				uint32_t __TRBType : 6;
				uint32_t __control : 16; // IOC, Chain, etc
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(TRB) == 16);

	/* Figure 6-8: Normal TRB */
	struct NormalTRB
	{
		uint64_t DataBufferPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __TRBTransferLength : 17;
				uint32_t __TDSize : 5;
				uint32_t __InterrupterTarget : 10;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;	// Cycle Bit
				uint32_t __ENT : 1; // Evaluate Next TRB
				uint32_t __ISP : 1; // Interrupt on Short Packet
				uint32_t __NS : 1;	// No Snoop
				uint32_t __CH : 1;	// Chain Bit
				uint32_t __IOC : 1; // Interrupt on Completion
				uint32_t __IDT : 1; // Immediate Data
				uint32_t __BEI : 1; // Block Event Interrupt
				uint32_t __TRBType : 6;
				uint32_t __RsvdZ : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(NormalTRB) == 16);

	/* Figure 6-9: Setup Stage TRB */
	struct SetupStageTRB
	{
		union PARAM0_UNION
		{
			struct
			{
				uint32_t __bmRequestType : 8;
				uint32_t __bRequest : 8;
				uint32_t __wValue : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAM0_UNION);
		} Parameter0;

		union PARAM1_UNION
		{
			struct
			{
				uint32_t __wIndex : 16;
				uint32_t __wLength : 16;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAM1_UNION);
		} Parameter1;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __TRBTransferLength : 17;
				uint32_t __RsvdZ : 5;
				uint32_t __InterrupterTarget : 10;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __ENT : 1;		// Evaluate Next TRB
				uint32_t __RsvdZ0 : 4;	// Reserved
				uint32_t __IOC : 1;		// Interrupt on Completion
				uint32_t __IDT : 1;		// Immediate Data, always 1 for Setup Stage
				uint32_t __RsvdZ1 : 3;	// Reserved
				uint32_t __TRBType : 6; // Setup Stage TRB Type
				uint32_t __TRT : 2;		// Transfer Type: 0=No Data, 2=OUT, 3=IN
				uint32_t __RsvdZ2 : 12; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(SetupStageTRB) == 16);

	/* Figure 6-10: Data Stage TRB */
	struct DataStageTRB
	{
		uint64_t DataBufferPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __TRBTransferLength : 17;
				uint32_t __TDSize : 5;
				uint32_t __InterrupterTarget : 10;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __ENT : 1;		// Evaluate Next TRB
				uint32_t __ISP : 1;		// Interrupt on Short Packet
				uint32_t __NS : 1;		// No Snoop
				uint32_t __CH : 1;		// Chain
				uint32_t __IOC : 1;		// Interrupt on Completion
				uint32_t __IDT : 1;		// Immediate Data
				uint32_t __RsvdZ0 : 1;	// Reserved
				uint32_t __TRBType : 6; // Data Stage TRB Type
				uint32_t __DIR : 1;		// Direction: 0=OUT, 1=IN
				uint32_t __RsvdZ1 : 11; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(DataStageTRB) == 16);

	/* Figure 6-11: Status Stage TRB */
	struct StatusStageTRB
	{
		uint32_t __RsvdZ[2]; // Reserved

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 22;
				uint32_t __InterrupterTarget : 10;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __ENT : 1;		// Evaluate Next TRB
				uint32_t __RsvdZ0 : 2;	// Reserved
				uint32_t __CH : 1;		// Chain Bit
				uint32_t __IOC : 1;		// Interrupt on Completion
				uint32_t __RsvdZ1 : 4;	// Reserved
				uint32_t __TRBType : 6; // Status Stage TRB Type
				uint32_t __DIR : 1;		// Direction: 0=OUT, 1=IN
				uint32_t __RsvdZ2 : 11; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(StatusStageTRB) == 16);

	/* Figure 6-12: Isoch TRB */
	struct IsochTRB
	{
		uint64_t DataBufferPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __TRBTransferLength : 17;
				uint32_t __TDSize_TBC : 5; // TD Size or Transfer Burst Count
				uint32_t __InterrupterTarget : 10;
			} __packed;

			struct
			{
				uint32_t __FrameID : 11; // Frame ID
				uint32_t __SIA : 1;		 // Start Isoch ASAP
			} __packed;

			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __ENT : 1;		// Evaluate Next TRB
				uint32_t __ISP : 1;		// Interrupt on Short Packet
				uint32_t __NS : 1;		// No Snoop
				uint32_t __CH : 1;		// Chain
				uint32_t __IOC : 1;		// Interrupt on Completion
				uint32_t __IDT : 1;		// Immediate Data
				uint32_t __BEI : 1;		// Block Event Interrupt
				uint32_t __TRBType : 6; // Isoch TRB Type
				uint32_t __TLBPC : 4;	// Transfer Last Burst Packet Count
				uint32_t __RsvdZ0 : 8;	// Reserved
			} __packed;

			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(IsochTRB) == 16);

	/* Figure 6-13: No-Op TRB */
	struct NoOpTRB
	{
		uint32_t __RsvdZ[2];

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 22;
				uint32_t __InterrupterTarget : 10;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __ENT : 1;		// Evaluate Next TRB
				uint32_t __RsvdZ0 : 2;	// Reserved
				uint32_t __CH : 1;		// Chain Bit
				uint32_t __IOC : 1;		// Interrupt On Completion
				uint32_t __RsvdZ1 : 4;	// Reserved
				uint32_t __TRBType : 6; // No-Op TRB Type
				uint32_t __RsvdZ2 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(NoOpTRB) == 16);

	/* Figure 6-14: Transfer Event TRB */
	struct TransferEventTRB
	{
		uint64_t TRBPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __TRBTransferLength : 24;
				uint32_t __CompletionCode : 8;
			} __packed;
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		   // Cycle Bit
				uint32_t __RsvdZ0 : 1;	   // Reserved
				uint32_t __ED : 1;		   // Event Data flag
				uint32_t __RsvdZ1 : 6;	   // Reserved
				uint32_t __TRBType : 6;	   // Transfer Event TRB Type
				uint32_t __EndpointID : 5; // Endpoint ID
				uint32_t __RsvdZ2 : 12;	   // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(TransferEventTRB) == 16);

	/* Figure 6-15: Command Completion Event TRB */
	struct CommandCompletionEventTRB
	{
		uint64_t CommandTRBPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __CommandCompletionParameter : 24;
				uint32_t __CompletionCode : 8;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 8;	// Reserved
				uint32_t __TRBType : 6; // Command Completion Event TRB Type
				uint32_t __VFID : 8;	// Virtual Function ID
				uint32_t __SlotID : 8;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(CommandCompletionEventTRB) == 16);

	/* Figure 6-16: Port Status Change Event TRB */
	struct PortStatusChangeEventTRB
	{
		union PARAMETER_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 24; // Reserved
				uint32_t __PortID : 8;	// Root Hub Port Number
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_UNION);
		} Parameter;

		uint32_t __RsvdZ;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ1 : 24;		   // Reserved
				uint32_t __CompletionCode : 8; // Completion code (always Success)
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Port Status Change Event TRB Type
				uint32_t __RsvdZ1 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(PortStatusChangeEventTRB) == 16);

	/* Figure 6-17: Bandwidth Request Event TRB */
	struct BandwidthRequestEventTRB
	{
		uint32_t __RsvdZ0[2];

		union PARAMETER_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 24;
				uint32_t __CompletionCode : 8;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Bandwidth Request Event TRB Type
				uint32_t __RsvdZ1 : 8;	// Reserved
				uint32_t __SlotID : 8;	// Device Slot to evaluate bandwidth

			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(BandwidthRequestEventTRB) == 16);

	/* Figure 6-18: Doorbell Event TRB */
	struct DoorbellEventTRB
	{
		union PARAMETER_UNION
		{
			struct
			{
				uint32_t __DBReason : 5; // DB Target value
				uint32_t __RsvdZ0 : 27;	 // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_UNION);
		} Parameter;

		uint32_t __RsvdZ;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ1 : 24;		   // Reserved
				uint32_t __CompletionCode : 8; // Always Success
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Doorbell Event TRB Type
				uint32_t __VFID : 8;	// Virtual Function ID
				uint32_t __SlotID : 8;	// Device Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(DoorbellEventTRB) == 16);

	/* Figure 6-19: Host Controller Event TRB */
	struct HostControllerEventTRB
	{
		uint32_t __RsvdZ[2];

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ1 : 24;		   // Reserved
				uint32_t __CompletionCode : 8; // Completion Code
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Host Controller Event TRB Type
				uint32_t __RsvdZ1 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(HostControllerEventTRB) == 16);

	/* Figure 6-20: Device Notification Event TRB */
	struct DeviceNotificationEventTRB
	{
		union PARAMETER_UNION
		{
			struct
			{
				uint32_t __RsvdZ : 4;					  // Reserved
				uint32_t __NotificationType : 4;		  // Notification Type
				uint32_t __DeviceNotificationDataLo : 24; // Device Notification Data Lo
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_UNION);
		} Parameter;

		uint32_t __DeviceNotificationDataHi; // Device Notification Data Hi

		union PARAMETER_HI_UNION
		{
			struct
			{
				uint32_t __RsvdZ : 24;
				uint32_t __CompletionCode : 8;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_HI_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Device Notification Event TRB Type
				uint32_t __RsvdZ1 : 10; // Reserved
				uint32_t __SlotID : 6;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(DeviceNotificationEventTRB) == 16);

	/* Figure 6-21: MFINDEX Wrap Event TRB */
	struct MFINDEXWrapEventTRB
	{
		uint32_t __RsvdZ[2];

		union PARAMETER_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 24;		   // Reserved
				uint32_t __CompletionCode : 8; // Completion Code (always Success)
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, PARAMETER_UNION);
		} Status;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // MFINDEX Wrap Event TRB Type
				uint32_t __RsvdZ1 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(MFINDEXWrapEventTRB) == 16);

	/* Figure 6-22: No Op Command TRB */
	struct NoOpCommandTRB
	{
		uint32_t __RsvdZ[3];

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // No Op Command TRB Type
				uint32_t __RsvdZ1 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(NoOpCommandTRB) == 16);

	/* Figure 6-23: Enable Slot Command TRB */
	struct EnableSlotCommandTRB
	{
		uint32_t __RsvdZ[3];

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		 // Cycle Bit
				uint32_t __RsvdZ0 : 9;	 // Reserved
				uint32_t __TRBType : 6;	 // Enable Slot Command TRB Type
				uint32_t __SlotType : 5; // Slot Type
				uint32_t __RsvdZ1 : 11;	 // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(EnableSlotCommandTRB) == 16);

	/* Figure 6-24: Disable Slot Command TRB */
	struct DisableSlotCommandTRB
	{
		uint32_t __RsvdZ[3];

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 9;	// Reserved
				uint32_t __TRBType : 6; // Disable Slot Command TRB Type
				uint32_t __RsvdZ1 : 8;	// Reserved
				uint32_t __SlotID : 8;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(DisableSlotCommandTRB) == 16);

	/* Figure 6-25: Address Device Command TRB */
	struct AddressDeviceCommandTRB
	{
		uint64_t InputContextPointer; // 64-bit pointer to Input Context
		uint32_t __RsvdZ;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 8;	// Reserved
				uint32_t __BSR : 1;		// Block Set Address Request
				uint32_t __TRBType : 6; // Address Device Command TRB Type
				uint32_t __RsvdZ1 : 8;	// Reserved
				uint32_t __SlotID : 8;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(AddressDeviceCommandTRB) == 16);

	/* Figure 6-26: Configure Endpoint Command TRB */
	struct ConfigureEndpointCommandTRB
	{
		uint64_t InputContextPointer; // 64-bit pointer to Input Context
		uint32_t __RsvdZ;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 8;	// Reserved
				uint32_t __DC : 1;		// Deconfigure flag
				uint32_t __TRBType : 6; // Configure Endpoint Command TRB Type
				uint32_t __RsvdZ1 : 8;	// Reserved
				uint32_t __SlotID : 8;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(ConfigureEndpointCommandTRB) == 16);

	/* Figure 6-27: Evaluate Context Command TRB */
	struct EvaluateContextCommandTRB
	{
		uint64_t InputContextPointer; // 64-bit pointer to Input Context
		uint32_t __RsvdZ;

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle Bit
				uint32_t __RsvdZ0 : 8;	// Reserved
				uint32_t __BSR : 1;		// Block Set Address Request (not used for Evaluate Context)
				uint32_t __TRBType : 6; // Evaluate Context Command TRB Type
				uint32_t __RsvdZ1 : 8;	// Reserved
				uint32_t __SlotID : 8;	// Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(EvaluateContextCommandTRB) == 16);

	/* Figure 6-28: Reset Endpoint Command TRB */
	struct ResetEndpointCommandTRB
	{
		uint32_t __RsvdZ[3];

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		   // Cycle Bit
				uint32_t __RsvdZ0 : 8;	   // Reserved
				uint32_t __TSP : 1;		   // Transfer State Preserve
				uint32_t __TRBType : 6;	   // Reset Endpoint Command TRB Type
				uint32_t __EndpointID : 5; // Endpoint ID (DCI)
				uint32_t __RsvdZ1 : 3;	   // Reserved
				uint32_t __SlotID : 8;	   // Slot ID
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;
	} __packed;
	static_assert(sizeof(ResetEndpointCommandTRB) == 16);

	/* Figure 6-38: Link TRB */
	struct LinkTRB
	{
		uint64_t RingSegmentPointer;

		union STATUS_UNION
		{
			struct
			{
				uint32_t __RsvdZ0 : 22;			   // Reserved
				uint32_t __InterrupterTarget : 10; // Interrupter Target
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, STATUS_UNION);
		} Status;

		uint16_t InterrupterTarget() { return (Status.raw >> 22) & 0x3FF; }
		void InterrupterTarget(uint16_t value) { Status.raw = (Status.raw & ~(0x3FF << 22)) | ((value & 0x3FF) << 22); }

		union CONTROL_UNION
		{
			struct
			{
				uint32_t __C : 1;		// Cycle bit
				uint32_t __TC : 1;		// Toggle Cycle
				uint32_t __RsvdZ0 : 2;	// Reserved
				uint32_t __CH : 1;		// Chain bit
				uint32_t __IOC : 1;		// Interrupt On Completion
				uint32_t __RsvdZ1 : 4;	// Reserved
				uint32_t __TRBType : 6; // Link TRB Type
				uint32_t __RsvdZ2 : 16; // Reserved
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CONTROL_UNION);
		} Control;

		uint8_t CycleBit() { return (Control.raw >> 0) & 0x1; }
		void CycleBit(uint8_t value) { Control.raw = (Control.raw & ~0x1) | (value & 0x1); }

		uint8_t ToggleCycle() { return (Control.raw >> 1) & 0x1; }
		void ToggleCycle(uint8_t value) { Control.raw = (Control.raw & ~(0x1 << 1)) | ((value & 0x1) << 1); }

		uint8_t ChainBit() { return (Control.raw >> 4) & 0x1; }
		void ChainBit(uint8_t value) { Control.raw = (Control.raw & ~(0x1 << 4)) | ((value & 0x1) << 4); }

		uint8_t InterruptOnCompletion() { return (Control.raw >> 5) & 0x1; }
		void InterruptOnCompletion(uint8_t value) { Control.raw = (Control.raw & ~(0x1 << 5)) | ((value & 0x1) << 5); }

		uint8_t TRBType() { return (Control.raw >> 16) & 0x3F; }
		void TRBType(uint8_t value) { Control.raw = (Control.raw & ~(0x3F << 16)) | ((value & 0x3F) << 16); }
	} __packed;
	static_assert(sizeof(LinkTRB) == 16);
}
