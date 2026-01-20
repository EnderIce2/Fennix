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

#define REG_USBCMD 0x00	   /* USB Command Register (R/W) */
#define REG_USBSTS 0x02	   /* USB Status Register (R/WC) */
#define REG_USBINTR 0x04   /* USB Interrupt Enable Register (R/W) */
#define REG_FRNUM 0x06	   /* Frame Number (R/W) */
#define REG_FLBASEADD 0x08 /* Frame List Base Address (R/W) */
#define REG_SOFMOD 0x0C	   /* Start of Frame Modify Register (R/W) */
#define REG_PORTSC 0x10	   /* Port Status and Control (R/WC) */
#define REG_PORTSC1 0x10   /* Port 1 Status and Control (R/WC) */
#define REG_PORTSC2 0x12   /* Port 2 Status and Control (R/WC) */
#define REG_LEGSUP 0xC0	   /* Legacy Support Register (R/WC) */
#define REG_INTEL 0xC4	   /* Intel Reserved Register (R/WC) */

#define USBCMD_RS (1 << 0)
#define USBCMD_HCRESET (1 << 1)
#define USBCMD_GRESET (1 << 2)
#define USBCMD_EGSM (1 << 3)
#define USBCMD_FGR (1 << 4)
#define USBCMD_SWDBG (1 << 5)
#define USBCMD_CF (1 << 6)
#define USBCMD_MAXP (1 << 7)

#define USBSTS_USBINT (1 << 0)
#define USBSTS_USBERRINT (1 << 1)
#define USBSTS_RD (1 << 2)
#define USBSTS_HSE (1 << 3)
#define USBSTS_HCPE (1 << 4)
#define USBSTS_HCH (1 << 5)

#define USBINTR_TOCRC (1 << 0)
#define USBINTR_RIE (1 << 1)
#define USBINTR_IOCE (1 << 2)
#define USBINTR_SPIE (1 << 3)

#define PORT_CCS (1 << 0)
#define PORT_CSC (1 << 1)
#define PORT_PE (1 << 2)
#define PORT_PEC (1 << 3)
#define PORT_LS (3 << 4)
#define PORT_RD (1 << 6)
#define PORT_ALWAYS_ONE (1 << 7)
#define PORT_LSDA (1 << 8)
#define PORT_PR (1 << 9)
#define PORT_SUS (1 << 12)

#define FLP_T (1 << 0)
#define FLP_Q (1 << 1)

#define QH_HEAD_T (1 << 0)
#define QH_HEAD_Q (1 << 1)

#define QH_ELEMENT_T (1 << 0)
#define QH_ELEMENT_Q (1 << 1)

#define TD_LINK_T (1 << 0)
#define TD_LINK_Q (1 << 1)
#define TD_LINK_Vf (1 << 2)

#define TD_CS_ACTLEN 0x000007FF
#define TD_CS_STATUS_BITSTUFF (1 << 17)
#define TD_CS_STATUS_CRC (1 << 18)
#define TD_CS_STATUS_NAK (1 << 19)
#define TD_CS_STATUS_BABBLE (1 << 20)
#define TD_CS_STATUS_DATABUFFER (1 << 21)
#define TD_CS_STATUS_STALLED (1 << 22)
#define TD_CS_STATUS_ACTIVE (1 << 23)

#define TD_CS_IOC (1 << 24)
#define TD_CS_IOS (1 << 25)
#define TD_CS_LS (1 << 26)
#define TD_CS_SPD (1 << 29)

#define TD_TOKEN_PID 0x000000FF

#define TD_TOKEN_DEVICE_ADDRESS 0x00007F00
#define TD_TOKEN_DEVICE_ADDRESS_SHIFT 8

#define TD_TOKEN_ENDPT 0x0078000
#define TD_TOKEN_ENDPT_SHIFT 15

#define TD_TOKEN_D 0x00080000
#define TD_TOKEN_D_SHIFT 19

#define TD_TOKEN_MAXLEN 0xFFE00000
#define TD_TOKEN_MAXLEN_SHIFT 21

#define TD_PID_IN 0x69
#define TD_PID_OUT 0xE1
#define TD_PID_SETUP 0x2D

namespace Driver::UniversalHostControllerInterface
{
	union USBCMD
	{
		struct
		{
			uint16_t RS : 1;
			uint16_t HCRESET : 1;
			uint16_t GRESET : 1;
			uint16_t EGSM : 1;
			uint16_t FGR : 1;
			uint16_t SWDBG : 1;
			uint16_t CF : 1;
			uint16_t MAXP : 1;
			uint16_t __reserved0 : 8;
		} __packed;
		DEFINE_BITWISE_TYPE(uint16_t, USBCMD);
	};

	union USBSTS
	{
		struct
		{
			uint16_t USBINT : 1;
			uint16_t USBERRINT : 1;
			uint16_t RD : 1;
			uint16_t HSE : 1;
			uint16_t HCPE : 1;
			uint16_t HCH : 1;
			uint16_t __reserved0 : 10;
		} __packed;
		DEFINE_BITWISE_TYPE(uint16_t, USBSTS);
	};

	union USBINTR
	{
		struct
		{
			uint16_t TOCRC : 1;
			uint16_t RIE : 1;
			uint16_t IOCE : 1;
			uint16_t SPIE : 1;
			uint16_t __reserved0 : 12;
		} __packed;
		DEFINE_BITWISE_TYPE(uint16_t, USBINTR);
	};

	union FRNUM
	{
		struct
		{
			uint16_t FN : 9;
			uint16_t __reserved0 : 7;
		} __packed;
		DEFINE_BITWISE_TYPE(uint16_t, FRNUM);
	};

	union FLBASEADD
	{
		struct
		{
			uint32_t __reserved0 : 12;
			uint32_t BA : 20;
		} __packed;
		DEFINE_BITWISE_TYPE(uint32_t, FLBASEADD);
	};

	union SOFMOD
	{
		struct
		{
			uint8_t SOFTVAL : 7;
			uint8_t __reserved0 : 1;
		} __packed;
		DEFINE_BITWISE_TYPE(uint8_t, SOFMOD);
	};

	union PORTSC
	{
		struct
		{
			uint16_t CCS : 1;
			uint16_t CSC : 1;
			uint16_t PE : 1;
			uint16_t PEC : 1;
			uint16_t LS : 2;
			uint16_t RD : 1;
			uint16_t AlwaysOne : 1;
			uint16_t LSDA : 1;
			uint16_t PR : 1;
			uint16_t __reserved1 : 2;
			uint16_t SUS : 1;
			uint16_t __reserved2 : 3;
		} __packed;
		DEFINE_BITWISE_TYPE(uint16_t, PORTSC);
	};

	union FrameListPointer
	{
		struct
		{
			/**
			 * Terminate
			 *
			 * 0 = Pointer is valid, this bit indicates that this frame has valid entries (QH or TD)
			 * 1 = Pointer is invalid (empty frame)
			 */
			uint32_t T : 1;

			/**
			 * QH/TD Select
			 *
			 * 0 = Pointer points to a TD
			 * 1 = Pointer points to a QH
			 */
			uint32_t Q : 1;

			/**
			 * Reserved
			 */
			uint32_t __reserved0 : 2;

			/**
			 * Frame List Pointer
			 *
			 * This field contains the address of the first QH or TD in the frame.
			 */
			uint32_t FLP : 28;
		};
		DEFINE_BITWISE_TYPE(uint32_t, FrameListPointer);
	};

	struct TD
	{
		union LINK_UNION
		{
			struct
			{
				/**
				 * Terminate
				 *
				 * 0 = Link Pointer field is valid
				 * 1 = Link Pointer field is not valid
				 */
				uint32_t T : 1;

				/**
				 * QH/TD Select
				 *
				 * 0 = TD
				 * 1 = QH
				 */
				uint32_t Q : 1;

				/**
				 * Depth/Breadth Select
				 *
				 * 0 = Breadth first
				 * 1 = Depth first
				 */
				uint32_t Vf : 1;

				uint32_t __reserved0 : 1;

				/**
				 * Link Pointer
				 *
				 * This field points to another TD or QH.
				 */
				uint32_t LP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, LINK_UNION);
		} LINK;
		static_assert(sizeof(LINK) == sizeof(uint32_t));

		union CS_UNION
		{
			struct
			{
				/**
				 * Actual Length
				 *
				 * The value is encoded as n-1.
				 */
				uint32_t ActLen : 11;

				uint32_t __reserved0 : 5;

				/**
				 * Status field
				 */
				union
				{
					struct
					{
						uint8_t __reserved0 : 1;

						/**
						 * Set if more than 6 consecutive ones are detected in the received data stream.
						 */
						uint8_t BITSTUFF_ERROR : 1;

						/**
						 * Set on CRC error or timeout (no response from device/endpoint within protocol-specified time).
						 */
						uint8_t CRC_TO_ERROR : 1;

						/**
						 * Set when a NAK packet is received during the transaction.
						 */
						uint8_t NAK_RECEIVED : 1;

						/**
						 * Set when a "babble" is detected during the transaction; also sets STALLED.
						 */
						uint8_t BABBLE_DETECTED : 1;

						/**
						 * Set if Host Controller cannot keep up with data reception (overrun) or supply data fast enough (underrun).
						 */
						uint8_t DATA_BUFFER_ERROR : 1;

						/**
						 * Set by Host Controller to indicate a serious error (e.g., babble, error counter zero, or STALL handshake).
						 */
						uint8_t STALLED : 1;

						/**
						 * Set by software to enable execution of a transaction. Cleared by Host Controller when transaction completes or on stall.
						 */
						uint8_t ACTIVE : 1;
					};
					uint8_t raw;
				} STATUS;

				/**
				 * Interrupt On Complete
				 *
				 * 1 = Interrupt on completion
				 */
				uint32_t IOC : 1;

				/**
				 * Isochronous Select
				 *
				 * 0 = Non-isochronous Transfer Descriptor
				 * 1 = Isochronous Transfer Descriptor
				 */
				uint32_t IOS : 1;

				/**
				 * Low Speed Device
				 *
				 * 0 = Full speed device
				 * 1 = Low speed device
				 */
				uint32_t LS : 1;

				/**
				 * 00b = No Error Limit
				 * 01b = 1 error
				 * 10b = 2 errors
				 * 11b = 3 errors
				 *
				 * @note check UHCI spec page 28 (3.2.2)
				 */
				uint32_t unknown_0 : 2; /* 28:27 missing in the spec @ 3.2.2 */

				/**
				 * Short Packet Detect
				 *
				 * 0 = Disable
				 * 1 = Enable
				 */
				uint32_t SPD : 1;

				uint32_t __reserved1 : 2;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CS_UNION);
		} CS;
		static_assert(sizeof(CS) == sizeof(uint32_t));

		union TOKEN_UNION
		{
			struct
			{
				/**
				 * Packet Identification
				 *
				 * IN = 0x69
				 * OUT = 0xE1
				 * SETUP = 0x2D
				 */
				uint32_t PID : 8;

				/**
				 * Device Address
				 */
				uint32_t DeviceAddress : 6;

				/**
				 * Endpoint
				 */
				uint32_t EndPt : 4;

				/**
				 * Data Toggle
				 *
				 * 0 = DATA0
				 * 1 = DATA1
				 */
				uint32_t D : 1;

				uint32_t __reserved0 : 1;

				/**
				 * Maximum Length
				 *
				 * 0x000 = 1 byte
				 * 0x001 = 2 bytes
				 *      ...
				 * 0x3FE = 1023 bytes
				 * 0x3FF = 1024 bytes
				 *      ...
				 * 0x4FF = 1280 bytes
				 * 0x7FF = 0 bytes (Null Data Packet)
				 */
				uint32_t MaxLen : 11;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, TOKEN_UNION);
		} TOKEN;
		static_assert(sizeof(TOKEN) == sizeof(uint32_t));

		union BUFFER_UNION
		{
			struct
			{
				/**
				 * Buffer Pointer
				 */
				uint32_t Addr : 32;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, BUFFER_UNION);
		} BUFFER;
		static_assert(sizeof(BUFFER) == sizeof(uint32_t));

		/* The last 4 DWords of the Transfer Descriptor are reserved for use by software.
			  - UHCI Design Guide 1.1 @ 3.2.5 */
		uint8_t __software[16];
	};

	struct QH
	{
		union HEAD_UNION
		{
			struct
			{
				/**
				 * Terminate
				 *
				 * 0 = Pointer is valid
				 * 1 = Last QH
				 */
				uint32_t T : 1;

				/**
				 * QH/TD Select
				 *
				 * 0 = TD
				 * 1 = QH
				 */
				uint32_t Q : 1;

				/**
				 * Must be zero
				 */
				uint32_t __reserved0 : 2;

				/**
				 * Queue Head Link Pointer
				 *
				 * This field contains the next data object to be processed in the horizontal list.
				 */
				uint32_t QHLP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, HEAD_UNION);
		} HEAD;
		static_assert(sizeof(HEAD) == sizeof(uint32_t));

		union ELEMENT_UNION
		{
			struct
			{
				/**
				 * Terminate
				 *
				 * 1 = No valid queue entries
				 */
				uint32_t T : 1;

				/**
				 * QH/TD Select
				 *
				 * 0 = TD
				 * 1 = QH
				 */
				uint32_t Q : 1;

				uint32_t __reserved0 : 1;

				/**
				 * Must be zero
				 */
				uint32_t __reserved1 : 1;

				/**
				 * Queue Element Link Pointer
				 *
				 * This field contains the address of the next QH or TD.
				 */
				uint32_t QELP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, ELEMENT_UNION);
		} ELEMENT;
		static_assert(sizeof(ELEMENT) == sizeof(uint32_t));

		/* same as TD but QH has 64 bytes */
		uint8_t __software[56];
	};

	class Port
	{
	private:
		uint16_t PortIO;
		USBSpeeds Speed;

	public:
		USBSpeeds GetSpeed() { return Speed; }
		PORTSC Status();
		int Clear(PORTSC Change = 0);
		int Set(PORTSC Change);
		int Reset();
		int Probe();
		Port(uint16_t io);
		~Port();
	};

	class Queue
	{
	private:
		TD *TDPool __aligned(0x10);
		QH *QHPool __aligned(0x10);

		FrameListPointer *FrameList;
		size_t Frames = 1024;
		size_t CurrentFrame = 0;
		size_t Subframes = 1;
		size_t MaxBandwidth = 900;
		size_t MaxQHs = 8;
		size_t MaxTDs = 32;
		QH *CurrentQueue;

	public:
		UniversalSerialBus::Scheduler *sched;

		QH *AllocateQueueHead();
		int ReleaseQueueHead(QH *qh);
		TD *AllocateTransferDescriptor();
		int ReleaseTransferDescriptor(TD *td);
		Queue();
		~Queue();
	};

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		uint16_t io;
		PCI::PCIDevice Header;

		int OnInterruptReceived(CPU::TrapFrame *Frame) final;

	public:
		Queue *queue = nullptr;

		int Reset();
		int Start(bool WaitForStart);
		int Stop();
		int Detect();
		int ProcessQueueHead(QH *qh);
		int Poll();
		HCD(uintptr_t base, PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
