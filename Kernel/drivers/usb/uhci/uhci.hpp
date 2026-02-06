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
	IO_REG_RW(uint16_t, USBCMD,
			  RS = 1u << 0,
			  HCRESET = 1u << 1,
			  GRESET = 1u << 2,
			  EGSM = 1u << 3,
			  FGR = 1u << 4,
			  SWDBG = 1u << 5,
			  CF = 1u << 6,
			  MAXP = 1u << 7);

	IO_REG_RWC(uint16_t, USBSTS,
			   USBINT = 1u << 0,
			   USBERRINT = 1u << 1,
			   RD = 1u << 2,
			   HSE = 1u << 3,
			   HCPE = 1u << 4,
			   HCH = 1u << 5);

	IO_REG_RW(uint16_t, USBINTR,
			  TOCRC = 1u << 0,
			  RIE = 1u << 1,
			  IOCE = 1u << 2,
			  SPIE = 1u << 3);

	IO_REG_RW(uint16_t, PORTSC,
			  CCS = 1u << 0,	   /* RO */
			  CSC = 1u << 1,	   /* R/WC */
			  PE = 1u << 2,		   /* R/W */
			  PEC = 1u << 3,	   /* R/WC */
			  LS_MASK = 3u << 4,   /* RO */
			  RD = 1u << 6,		   /* R/W */
			  AlwaysOne = 1u << 7, /* RO */
			  LSDA = 1u << 8,	   /* RO */
			  PR = 1u << 9,		   /* R/W */
			  SUS = 1u << 12);	   /* R/W */

	union FrameListPointer
	{
		struct
		{
			uint32_t __T : 1;
			uint32_t __Q : 1;
			uint32_t __reserved0 : 2;
			uint32_t __FLP : 28;
		} __packed;
		DEFINE_BITWISE_TYPE(uint32_t, FrameListPointer);

		/**
		 * Terminate
		 *
		 * 0 = Pointer is valid, this bit indicates that this frame has valid entries (QH or TD)
		 * 1 = Pointer is invalid (empty frame)
		 */
		BF_RW(uint8_t, Terminate, 0, 1);

		/**
		 * QH/TD Select
		 *
		 * 0 = Pointer points to a TD
		 * 1 = Pointer points to a QH
		 */
		BF_RW(uint8_t, QHTDSelect, 1, 1);

		/**
		 * Frame List Pointer
		 *
		 * This field contains the address of the first QH or TD in the frame.
		 */
		BF_PHYS_RW(uint32_t, FLP, 4, 28);
	};
	static_assert(sizeof(FrameListPointer) == sizeof(uint32_t));

	struct TD
	{
		union LINK_UNION
		{
			struct
			{
				uint32_t __T : 1;
				uint32_t __Q : 1;
				uint32_t __Vf : 1;
				uint32_t __reserved0 : 1;
				uint32_t __LP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, LINK_UNION);

			/**
			 * Terminate
			 *
			 * 0 = Link Pointer field is valid
			 * 1 = Link Pointer field is not valid
			 */
			BF_RW(uint8_t, Terminate, 0, 1);

			/**
			 * QH/TD Select
			 *
			 * 0 = TD
			 * 1 = QH
			 */
			BF_RW(uint8_t, QHTDSelect, 1, 1);

			/**
			 * Depth/Breadth Select
			 *
			 * 0 = Breadth first
			 * 1 = Depth first
			 */
			BF_RW(uint8_t, DepthBreadthSelect, 2, 1);

			/**
			 * Link Pointer
			 *
			 * This field points to another TD or QH.
			 */
			BF_PHYS_RW(uint32_t, LinkPointer, 4, 28);
		} LINK;
		static_assert(sizeof(LINK) == sizeof(uint32_t));

		union CS_UNION
		{
			struct
			{
				uint32_t __ActLen : 11;
				uint32_t __reserved0 : 5;

				union
				{
					struct
					{
						uint8_t __reserved0 : 1;

						/**
						 * Set if more than 6 consecutive ones are detected in the received data stream.
						 */
						uint8_t __BITSTUFF_ERROR : 1;

						/**
						 * Set on CRC error or timeout (no response from device/endpoint within protocol-specified time).
						 */
						uint8_t __CRC_TO_ERROR : 1;

						/**
						 * Set when a NAK packet is received during the transaction.
						 */
						uint8_t __NAK_RECEIVED : 1;

						/**
						 * Set when a "babble" is detected during the transaction; also sets STALLED.
						 */
						uint8_t __BABBLE_DETECTED : 1;

						/**
						 * Set if Host Controller cannot keep up with data reception (overrun) or supply data fast enough (underrun).
						 */
						uint8_t __DATA_BUFFER_ERROR : 1;

						/**
						 * Set by Host Controller to indicate a serious error (e.g., babble, error counter zero, or STALL handshake).
						 */
						uint8_t __STALLED : 1;

						/**
						 * Set by software to enable execution of a transaction. Cleared by Host Controller when transaction completes or on stall.
						 */
						uint8_t __ACTIVE : 1;
					};
					uint8_t raw;
				} STATUS;

				uint32_t __IOC : 1;
				uint32_t __IOS : 1;
				uint32_t __LS : 1;
				uint32_t __unknown_0 : 2; /* 28:27 missing in the spec @ 3.2.2 */
				uint32_t __SPD : 1;
				uint32_t __reserved1 : 2;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, CS_UNION);

			BF_RO(uint16_t, __ActualLength, 0, 11);

			/**
			 * Actual Length
			 *
			 * The value is encoded as n-1.
			 */
			inline size_t ActualLength() const
			{
				uint16_t v = __ActualLength();
				return (v == 0x7FF) ? 0 : (v + 1);
			}

			/**
			 * Status field
			 */
			BF_RO(uint8_t, Status, 16, 8);

			/**
			 * Set if more than 6 consecutive ones are detected in the received data stream.
			 */
			BF_RO(uint8_t, Status_BITSTUFF_ERROR, 17, 1);

			/**
			 * Set on CRC error or timeout (no response from device/endpoint within protocol-specified time).
			 */
			BF_RO(uint8_t, Status_CRC_TO_ERROR, 18, 1);

			/**
			 * Set when a NAK packet is received during the transaction.
			 */
			BF_RO(uint8_t, Status_NAK_RECEIVED, 19, 1);

			/**
			 * Set when a "babble" is detected during the transaction; also sets STALLED.
			 */
			BF_RO(uint8_t, Status_BABBLE_DETECTED, 20, 1);

			/**
			 * Set if Host Controller cannot keep up with data reception (overrun) or supply data fast enough (underrun).
			 */
			BF_RO(uint8_t, Status_DATA_BUFFER_ERROR, 21, 1);

			/**
			 * Set by Host Controller to indicate a serious error (e.g., babble, error counter zero, or STALL handshake).
			 */
			BF_RO(uint8_t, Status_STALLED, 22, 1);

			/**
			 * Set by software to enable execution of a transaction. Cleared by Host Controller when transaction completes or on stall.
			 */
			BF_RW(uint8_t, Status_ACTIVE, 23, 1);

			/**
			 * Interrupt On Complete
			 *
			 * 1 = Interrupt on completion
			 */
			BF_RW(uint8_t, InterruptOnComplete, 24, 1);

			/**
			 * Isochronous Select
			 *
			 * 0 = Non-isochronous Transfer Descriptor
			 * 1 = Isochronous Transfer Descriptor
			 */
			BF_RW(uint8_t, IsochronousSelect, 25, 1);

			/**
			 * Low Speed Device
			 *
			 * 0 = Full speed device
			 * 1 = Low speed device
			 */
			BF_RW(uint8_t, LowSpeedDevice, 26, 1);

			/**
			 * 00b = No Error Limit
			 * 01b = 1 error
			 * 10b = 2 errors
			 * 11b = 3 errors
			 *
			 * @note check UHCI spec page 28 (3.2.2)
			 */
			BF_RW(uint8_t, ErrorCounter, 27, 2);

			/**
			 * Short Packet Detect
			 *
			 * 0 = Disable
			 * 1 = Enable
			 */
			BF_RW(uint8_t, ShortPacketDetect, 29, 1);
		} CS;
		static_assert(sizeof(CS) == sizeof(uint32_t));

		union TOKEN_UNION
		{
			struct
			{
				uint32_t __PID : 8;
				uint32_t __DeviceAddress : 6;
				uint32_t __EndPt : 4;
				uint32_t __D : 1;
				uint32_t __reserved0 : 1;
				uint32_t __MaxLen : 11;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, TOKEN_UNION);

			/**
			 * Packet Identification
			 *
			 * IN = 0x69
			 * OUT = 0xE1
			 * SETUP = 0x2D
			 */
			BF_RW(uint8_t, PacketIdentification, 0, 8);

			/**
			 * Device Address
			 */
			BF_RW(uint8_t, DeviceAddress, 8, 7);

			/**
			 * Endpoint
			 */
			BF_RW(uint8_t, Endpoint, 15, 4);

			/**
			 * Data Toggle
			 *
			 * 0 = DATA0
			 * 1 = DATA1
			 */
			BF_RW(uint8_t, DataToggle, 19, 1);

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
			BF_RW(uint16_t, MaximumLengthRaw, 21, 11);

			inline uint16_t MaximumLength() const
			{
				uint16_t raw = MaximumLengthRaw();
				if (raw == 0x7FF)
					return 0;
				if (raw < 0x400)
					return raw + 1;
				return raw - 0x3FF + 1024;
			}

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
			inline void MaximumLength(uint16_t length)
			{
				uint16_t raw;
				if (length == 0)
					raw = 0x7FF; /* Null Data Packet */
				else if (length <= 1023)
					raw = length - 1; /* 0x000-0x3FE = 1-1023 bytes */
				else if (length <= 1280)
					raw = (length - 1024) + 1023; /* 0x3FF-0x4FF = 1024-1280 bytes */
				else
					raw = 0x4FF; /* Invalid length, clamp to maximum */
				MaximumLengthRaw(raw);
			}
		} TOKEN;
		static_assert(sizeof(TOKEN) == sizeof(uint32_t));

		/**
		 * Buffer Pointer
		 */
		volatile uint32_t BufferPointer;

		/* The last 4 DWords of the Transfer Descriptor are reserved for use by software.
			  - UHCI Design Guide 1.1 @ 3.2.5 */
		uint8_t __software[0];
		USBRequestBlock *URB;
		TD *NextTD;
	};
	static_assert(sizeof(TD) == 32);

	struct QH
	{
		union HEAD_UNION
		{
			struct
			{
				uint32_t __T : 1;
				uint32_t __Q : 1;
				uint32_t __reserved0 : 2; /* Must be zero */
				uint32_t __QHLP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, HEAD_UNION);

			/**
			 * Terminate
			 *
			 * 0 = Pointer is valid
			 * 1 = Last QH
			 */
			BF_RW(uint8_t, Terminate, 0, 1);

			/**
			 * QH/TD Select
			 *
			 * 0 = TD
			 * 1 = QH
			 */
			BF_RW(uint8_t, QHTDSelect, 1, 1);

			/**
			 * Queue Head Link Pointer
			 *
			 * This field contains the next data object to be processed in the horizontal list.
			 */
			BF_PHYS_RW(uint32_t, QueueHeadLinkPointer, 4, 28);
		} HEAD;
		static_assert(sizeof(HEAD) == sizeof(uint32_t));

		union ELEMENT_UNION
		{
			struct
			{

				uint32_t __T : 1;
				uint32_t __Q : 1;
				uint32_t __reserved0 : 1;
				uint32_t __reserved1 : 1; /* Must be zero */
				uint32_t __QELP : 28;
			} __packed;
			DEFINE_BITWISE_TYPE(uint32_t, ELEMENT_UNION);

			/**
			 * Terminate
			 *
			 * 1 = No valid queue entries
			 */
			BF_RW(uint8_t, Terminate, 0, 1);

			/**
			 * QH/TD Select
			 *
			 * 0 = TD
			 * 1 = QH
			 */
			BF_RW(uint8_t, QHTDSelect, 1, 1);

			/**
			 * Queue Element Link Pointer
			 *
			 * This field contains the address of the next QH or TD.
			 */
			BF_PHYS_RW(uint32_t, QELP, 4, 28);
		} ELEMENT;
		static_assert(sizeof(ELEMENT) == sizeof(uint32_t));

		/* same as TD but QH has 64 bytes */
		uint8_t __software[56];
	};
	static_assert(sizeof(QH) == 64);

	class Port
	{
	private:
		PORTSC port;
		USBSpeeds Speed;

	public:
		USBSpeeds GetSpeed() { return Speed; }
		PORTSC Status();
		int Clear(uint16_t Change);
		int Set(uint16_t Change);
		int Reset();
		int Probe();
		Port(PORTSC io);
		~Port();
	};

	class TransferQueue
	{
	private:
		TD *TDPool __aligned(0x10);
		QH *QHPool __aligned(0x10);

		FrameListPointer *FrameList;
		uint16_t CurrentFrame = 0;

		bool QHUsed[16];
		bool TDUsed[64];
		size_t QHCount = 16;
		size_t TDCount = 64;

		QH *InterruptQH[9];
		QH *AsyncQH;

		uint16_t base;

	public:
		uint32_t *GetFrameList() { return (uint32_t *)FrameList; }
		uint16_t GetCurrentFrame() { return CurrentFrame; }

		void AdvanceFrame();
		void ProcessComplete();
		void EnqueueTD(TD *td);

		QH *AllocateQueueHead();
		int ReleaseQueueHead(QH *qh);
		TD *AllocateTransferDescriptor();
		int ReleaseTransferDescriptor(TD *td);
		TransferQueue(uint16_t io);
		~TransferQueue();
	};

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		uint16_t io;
		PCI::PCIDevice Header;

		USBCMD cmd;
		USBSTS sts;
		USBINTR intr;
		PORTSC port1;
		PORTSC port2;

		int OnInterruptReceived(CPU::TrapFrame *Frame) final;

		int SubmitControl(USBDevice *Device, USBRequestBlock *urb);
		int SubmitBulk(USBDevice *Device, USBRequestBlock *urb);
		int SubmitInterrupt(USBDevice *Device, USBRequestBlock *urb);
		int SubmitIsochronous(USBDevice *Device, USBRequestBlock *urb);

	public:
		TransferQueue *queue = nullptr;

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
