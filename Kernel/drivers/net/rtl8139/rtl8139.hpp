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

#include <types.h>

struct RXQueueEntry
{
	uint8_t *Buffer;
	uint16_t Size;
} __attribute__((packed));

enum InterruptStatus
{
	RecOK = 0x1,
	RecBad = 0x2,
	SendOK = 0x4,
	SendBad = 0x8,
};

enum ReceiveConfig
{
	/**
	 * Accept broadcast packets
	 * sent to mac ff:ff:ff:ff:ff:ff
	 */
	RcAB = 0x1,

	/**
	 * Accept packets sent to the
	 * multicast address
	 */
	RcAM = 0x2,

	/**
	 * Accept packets sent to the
	 * NIC's MAC address
	 */
	RcAPM = 0x4,

	/**
	 * Accept all packets
	 * (promiscuous mode)
	 */
	RcAAP = 0x8,

	/**
	 * The WRAP bit is used to tell the
	 * NIC to wrap around the ring
	 * buffer when it reaches the end
	 * of the buffer.
	 *
	 * @note If this bit is set, the
	 * buffer must have an additional
	 * 1500 bytes of space at the end
	 * of the buffer to prevent the
	 * NIC from overflowing the buffer.
	 */
	RcWRAP = 0x80,
};

enum Registers
{
	RegMAC = 0x0,
	RegMAR = 0x8,
	RegRBSTART = 0x30,
	RegCMD = 0x37,
	RegCAPR = 0x38,
	regRCR = 0x44,
	RegCONFIG1 = 0x52,
	RegIMR = 0x3C,
	RegISR = 0x3E,
};
