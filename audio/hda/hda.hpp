/*
	This file is part of Fennix Drivers.

	Fennix Drivers is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Drivers is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Drivers. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <types.h>

#ifdef __cplusplus

struct StreamDescriptor
{
	/** Control */
	uint32_t CTL : 24;

	/** Status */
	uint8_t STS;

	/** Link Position in Current Buffer */
	uint32_t LPIB;

	/** Cyclic Buffer Length */
	uint32_t CBL;

	/** Last Valid Index */
	uint16_t LVI;

	/** Reserved */
	uint8_t Rsvd0[2];

	/** FIFO Size */
	uint16_t FIFOD;

	/** Format */
	uint16_t FMT;

	/** Reserved */
	uint8_t Rsvd1[4];

	/** Buffer Descriptor List Pointer - Lower */
	uint32_t BDPL;

	/** Buffer Descriptor List Pointer - Upper */
	uint32_t BDPU;
} __attribute__((packed));

struct ControllerRegisters
{
	uint16_t GCAP;
	uint8_t VMIN;
	uint8_t VMJ;
	uint16_t OUTPAY;
	uint16_t INPAY;
	uint32_t GCTL;
	uint16_t WAKEEN;
	uint16_t WAKESTS;
	uint16_t GSTS;
	uint8_t Rsvd0[6];
	uint16_t OUTSTRMPAY;
	uint16_t INSTRMPAY;
	uint8_t Rsvd1[4];
	uint32_t INTCTL;
	uint32_t INTSTS;
	uint8_t Rsvd2[8];
	uint32_t WALCLK;
	uint8_t Rsvd3[4];
	uint32_t SSYNC;
	uint8_t Rsvd4[4];
	uint32_t CORBLBASE;
	uint32_t CORBUBASE;
	uint16_t CORBWP;
	uint16_t CORBRP;
	uint8_t CORBCTL;
	uint8_t CORBSTS;
	uint8_t CORBSIZE;
	uint8_t Rsvd5;
	uint32_t RIRBLBASE;
	uint32_t RIRBUBASE;
	uint16_t RIRBWP;
	uint16_t RINTCNT;
	uint8_t RIRBCTL;
	uint8_t RIRBSTS;
	uint8_t RIRBSIZE;
	uint8_t Rsvd6;
	uint32_t ICOI;
	uint32_t ICII;
	uint16_t ICIS;
	uint8_t Rsvd7[6];
	uint32_t DPIBLBASE;
	uint32_t DPIBUBASE;
	uint8_t Rsvd8[8];
	StreamDescriptor SD[];
} __attribute__((packed));

/* Not working as expected */
struct __ControllerRegisters
{
	/** Global Capabilities */
	union
	{
		struct
		{
			/** 64 Bit Address Supported
			 *
			 * 0 = 32-bit addressing
			 * 1 = 64-bit addressing
			 */
			uint16_t _64OK : 1;

			/** Number of Serial Data Out Signals
			 *
			 * 00 = 1 SDO
			 * 01 = 2 SDOs
			 * 10 = 4 SDOs
			 * 11 = Reserved
			 */
			uint16_t NSDO : 2;

			/** Number of Bidirectional Streams Supported
			 *
			 * 00000b = No bidirectional streams supported
			 * 00001b = 1 bidirectional stream supported
			 * ...
			 * 11110b = 30 bidirectional streams supported
			 */
			uint16_t BSS : 5;

			/** Number of Input Streams Supported
			 *
			 * 0000b = No input streams supported
			 * 0001b = 1 input stream supported
			 * ...
			 * 1111b = 15 input streams supported
			 */
			uint16_t ISS : 4;

			/** Number of Output Streams Supported
			 *
			 * 0000b = No output streams supported
			 * 0001b = 1 output stream supported
			 * ...
			 * 1111b = 15 output streams supported
			 */
			uint16_t OSS : 4;
		} __attribute__((packed));
		uint16_t Raw;
	} GCAP;

	/** Minor Version */
	uint8_t VMIN;

	/** Major Version */
	uint8_t VMJ;

	/** Output Payload Capability
	 *
	 * 00h = 0 Words
	 * 01h = 1 Word payload
	 * ...
	 * FFh = 255h Word payload
	 */
	uint16_t OUTPAY;

	/** Input Payload Capability
	 *
	 * 00h = 0 Words
	 * 01h = 1 Word payload
	 * ...
	 * FFh = 255h Word payload
	 */
	uint16_t INPAY;

	/** Global Control */
	union
	{
		struct
		{
			/** Controller Reset
			 *
			 * 0 = Reset
			 * 1 = Normal Operation
			 */
			uint32_t CRST : 1;

			/** Flush Control
			 *
			 * 0 = Idle
			 * 1 = Flush
			 */
			uint32_t FCNTRL : 1;

			/** Reserved */
			uint32_t RsvdP0 : 6;

			/** Accept Unsolicited Response Enable
			 *
			 * 0 = Disabled
			 * 1 = Enabled
			 */
			uint32_t UNSOL : 1;

			/** Reserved */
			uint32_t RsvdP1 : 23;
		} __attribute__((packed));
		uint32_t Raw;
	} GCTL;

	/** Wake Enable */
	union
	{
		struct
		{
			/** SDIN Wake Enable Flags */
			uint16_t SDIWEN : 15;

			/** Reserved */
			uint16_t RsvdP0 : 1;
		} __attribute__((packed));
		uint16_t Raw;
	} WAKEEN;

	/** Wake Status */
	union
	{
		struct
		{
			/** SDIN State Change Status Flags */
			uint16_t SDIWAKE : 15;

			/** Reserved */
			uint16_t RsvdZ0 : 1;
		} __attribute__((packed));
		uint16_t Raw;
	} WAKESTS;

	/** Global Status */
	union
	{
		struct
		{
			uint16_t RsvdZ0 : 1;
			uint16_t FSTS : 1;
			uint16_t RsvdZ1 : 14;
		} __attribute__((packed));
		uint16_t Raw;
	} GSTS;

	/** Reserved */
	uint8_t Rsvd0[6];

	/** Output Stream Payload Capability */
	uint16_t OUTSTRMPAY;

	/** Input Stream Payload Capability */
	uint16_t INSTRMPAY;

	/** Reserved */
	uint8_t Rsvd1[4];

	/** Interrupt Control */
	union
	{
		struct
		{
			/** Stream Interrupt Enable
			 *
			 * Bit 0 = Input Stream 0
			 * Bit 1 = Input Stream 1
			 * Bit 2 = Output Stream 0
			 * Bit 3 = Output Stream 1
			 * Bit 4 = Output Stream 2
			 * Bit 5 = Bidirectional Stream 0
			 * Bits 6-28 = Reserved
			 */
			uint32_t SIE : 30;

			/** Controller Interrupt Enable */
			uint32_t CIE : 1;

			/** Global Interrupt Enable */
			uint32_t GIE : 1;
		} __attribute__((packed));
		uint32_t Raw;
	} INTCTL;

	/** Interrupt Status */
	union
	{
		struct
		{
			/** Stream Interrupt Status */
			uint32_t SIS : 30;

			/** Controller Interrupt Status */
			uint32_t CIS : 1;

			/** Global Interrupt Status */
			uint32_t GIS : 1;
		} __attribute__((packed));
		uint32_t Raw;
	} INTSTS;

	/** Reserved */
	uint8_t Rsvd2[8];

	/** Wall Clock Counter */
	uint32_t WALCLK;

	/** Reserved */
	uint8_t Rsvd3[4];

	/** Stream Synchronization */
	union
	{
		struct
		{
			/** Stream Synchronization Bits */
			uint32_t SSYNC : 30;

			/** Reserved */
			uint32_t RsvdP0 : 2;
		} __attribute__((packed));
		uint32_t Raw;
	} SSYNC;

	/** Reserved */
	uint8_t Rsvd4[4];

	/** CORB Lower Base Address */
	union
	{
		struct
		{
			/** CORB Lower Base Unimplemented Bits */
			uint32_t Unimplemented : 7;

			/** CORB Lower Base Address */
			uint32_t CORBLBASE : 25;
		} __attribute__((packed));
		uint32_t Raw;
	} CORBLBASE;

	/** CORB Upper Base Address */
	uint32_t CORBUBASE;

	/** CORB Write Pointer */
	union
	{
		struct
		{
			/** CORB Write Pointer */
			uint16_t CORBWP : 8;

			/** Reserved */
			uint16_t RsvdP0 : 8;
		} __attribute__((packed));
		uint16_t Raw;
	} CORBWP;

	/** CORB Read Pointer */
	union
	{
		struct
		{
			/** CORB Read Pointer */
			uint16_t CORBRP : 8;

			/** Reserved */
			uint16_t RsvdP0 : 7;

			/** CORB Read Pointer Reset */
			uint16_t CORBRPRST : 1;
		} __attribute__((packed));
		uint16_t Raw;
	} CORBRP;

	/** CORB Control */
	union
	{
		struct
		{
			/** CORB Memory Error Interrupt Enable */
			uint8_t CMEIE : 1;

			/** Enable CORB DMA Engine
			 *
			 * 0 = DMA Stop
			 * 1 = DMA Run
			 *
			 * @note Must read the value back.
			 */
			uint8_t CORBRUN : 1;

			/** Reserved */
			uint8_t RsvdP0 : 6;
		} __attribute__((packed));
		uint8_t Raw;
	} CORBCTL;

	/** CORB Status */
	union
	{
		struct
		{
			/** CORB Memory Error Indication */
			uint8_t CMEI : 1;

			/** Reserved */
			uint8_t RsvdZ0 : 7;
		} __attribute__((packed));
		uint8_t Raw;
	} CORBSTS;

	/** CORB Size */
	union
	{
		struct
		{
			/** CORB Size
			 *
			 * 00b = 2 entries
			 * 01b = 16 entries
			 * 10b = 256 entries
			 * 11b = Reserved
			 */
			uint8_t CORBSIZE : 2;

			/** Reserved */
			uint8_t RsvdP0 : 2;

			/** CORB Size Capability
			 *
			 * 0001b = 2 entries
			 * 0010b = 16 entries
			 * 0100b = 256 entries
			 * 1000b = Reserved
			 */
			uint8_t CORBSZCAP : 4;
		} __attribute__((packed));
		uint8_t Raw;
	} CORBSIZE;

	/** Reserved */
	uint8_t Rsvd5;

	/** RIRB Lower Base Address */
	union
	{
		struct
		{
			/** RIRB Lower Base Unimplemented Bits */
			uint32_t Unimplemented : 7;

			/** RIRB Lower Base Address */
			uint32_t RIRBLBASE : 25;
		} __attribute__((packed));
		uint32_t Raw;
	} RIRBLBASE;

	/** RIRB Upper Base Address */
	uint32_t RIRBUBASE;

	/** RIRB Write Pointer */
	union
	{
		struct
		{
			/** RIRB Write Pointer */
			uint16_t RIRBWP : 8;

			/** Reserved */
			uint16_t RsvdP0 : 7;

			/** RIRB Write Pointer Reset */
			uint16_t RIRBWPRST : 1;
		} __attribute__((packed));
		uint16_t Raw;
	} RIRBWP;

	/** Response Interrupt Count */
	union
	{
		struct
		{
			/** N Response Interrupt Count
			 *
			 * 00000001b = 1 Response sent to RIRB
			 * ...
			 * 11111111b = 255 Responses sent to RIRB
			 * 00000000b = 256 Response sent to RIRB
			 */
			uint16_t RINTCNT : 8;

			/** Reserved */
			uint16_t RsvdP0 : 8;
		} __attribute__((packed));
		uint16_t Raw;
	} RINTCNT;

	/** RIRB Control */
	union
	{
		struct
		{
			/** Response Interrupt Control
			 *
			 * 0 = Disable Interrupt
			 * 1 = Generate an interrupt after N responses are sent to the RIRB
			 */
			uint8_t RINTCTL : 1;

			/** RIRB DMA Enable
			 *
			 * 0 = DMA Stop
			 * 1 = DMA Run
			 */
			uint8_t RIRBDMAEN : 1;

			/** Response Overrun Interrupt Control */
			uint8_t RIRBOIC : 1;

			/** Reserved */
			uint8_t RsvdP0 : 5;
		} __attribute__((packed));
		uint8_t Raw;
	} RIRBCTL;

	/** RIRB Status */
	union
	{
		struct
		{
			/** Response Interrupt */
			uint8_t RINTFL : 1;

			/** Reserved */
			uint8_t RsvdZ0 : 1;

			/** Response Overrun Interrupt Status */
			uint8_t RIRBOIS : 1;

			/** Reserved */
			uint8_t RsvdZ1 : 5;
		} __attribute__((packed));
		uint8_t Raw;
	} RIRBSTS;

	/** RIRB Size */
	union
	{
		struct
		{
			/** RIRB Size
			 *
			 * 00b = 2 entries
			 * 01b = 16 entries
			 * 10b = 256 entries
			 * 11b = Reserved
			 */
			uint8_t RIRBSIZE : 2;

			/** Reserved */
			uint8_t RsvdP0 : 2;

			/** RIRB Size Capability
			 *
			 * 0001b = 2 entries
			 * 0010b = 16 entries
			 * 0100b = 256 entries
			 * 1000b = Reserved
			 */
			uint8_t RIRBSZCAP : 4;
		} __attribute__((packed));
		uint8_t Raw;
	} RIRBSIZE;

	/** Reserved */
	uint8_t Rsvd6;

	/** Immediate Command Output Interface */
	uint32_t ICOI;

	/** Immediate Command Input Interface */
	uint32_t ICII;

	/** Immediate Command Status */
	uint16_t ICIS;

	/** Reserved */
	uint8_t Rsvd7[6];

	/** DMA Position Buffer Lower Base */
	union
	{
		struct
		{
			/** DMA Position Buffer Enable */
			uint32_t DPBEN : 1;

			/** Reserved */
			uint32_t RsvdZ0 : 6;

			/** DMA Position Lower Base Address */
			uint32_t DPLBASE : 25;
		} __attribute__((packed));
		uint32_t Raw;
	} DPIBLBASE;

	/** DMA Position Buffer Upper Base */
	uint32_t DPIBUBASE;

	/** Reserved */
	uint8_t Rsvd8[8];

	StreamDescriptor SD[];
} __attribute__((packed));
#endif

EXTERNC int cxx_Panic();
EXTERNC int cxx_Probe();
EXTERNC int cxx_Initialize();
EXTERNC int cxx_Finalize();
