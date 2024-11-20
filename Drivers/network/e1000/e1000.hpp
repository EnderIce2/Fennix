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
enum REG
{
	CTRL = 0x0000,
	STATUS = 0x0008,
	ICR = 0x000C,
	EEPROM = 0x0014,
	CTRL_EXT = 0x0018,
	ITR = 0x00C4,
	IMASK = 0x00D0,
	IAM = 0x00D8,
	RCTRL = 0x0100,
	RXDESCLO = 0x2800,
	RXDESCHI = 0x2804,
	RXDESCLEN = 0x2808,
	RXDESCHEAD = 0x2810,
	RXDESCTAIL = 0x2818,
	TCTRL = 0x0400,
	TXDESCLO = 0x3800,
	TXDESCHI = 0x3804,
	TXDESCLEN = 0x3808,
	TXDESCHEAD = 0x3810,
	TXDESCTAIL = 0x3818,
	RDTR = 0x2820,
	RXDCTL = 0x3828,
	RADV = 0x282C,
	RSRPD = 0x2C00,
	TIPG = 0x0410
};

enum PCTRL
{
	RESERVED = 0b000000,
	SPEED_SELECTION_MSB = 0b010000,
	UPDATE_COLLISION_TEST = 0b001000,
	DUPLEX_MODE = 0b000100,
	RESTART_AUTO_NEGOTIATION = 0b000010,
	ISOLATE = 0b000001,
	POWER_DOWN = 0b100000,
	SPEED_SELECTION_LSB = 0b100000,
};

enum ECTRL
{
	SLU = 0x40
};

enum RTCL
{
	RDMTS_HALF = (0 << 8),
	RDMTS_QUARTER = (1 << 8),
	RDMTS_EIGHTH = (2 << 8)
};

enum RCTL
{
	EN = (1 << 1),
	SBP = (1 << 2),
	UPE = (1 << 3),
	MPE = (1 << 4),
	LPE = (1 << 5),
	LBM_NONE = (0 << 6),
	LBM_PHY = (3 << 6),
	MO_36 = (0 << 12),
	MO_35 = (1 << 12),
	MO_34 = (2 << 12),
	MO_32 = (3 << 12),
	BAM = (1 << 15),
	VFE = (1 << 18),
	CFIEN = (1 << 19),
	CFI = (1 << 20),
	DPF = (1 << 22),
	PMCF = (1 << 23),
	SECRC = (1 << 26),
	BSIZE_256 = (3 << 16),
	BSIZE_512 = (2 << 16),
	BSIZE_1024 = (1 << 16),
	BSIZE_2048 = (0 << 16),
	BSIZE_4096 = ((3 << 16) | (1 << 25)),
	BSIZE_8192 = ((2 << 16) | (1 << 25)),
	BSIZE_16384 = ((1 << 16) | (1 << 25))
};

enum CMD
{
	EOP = (1 << 0),
	IFCS = (1 << 1),
	IC = (1 << 2),
	RS = (1 << 3),
	RPS = (1 << 4),
	VLE = (1 << 6),
	IDE = (1 << 7)
};

enum TCTL
{
	EN_ = (1 << 1),
	PSP = (1 << 3),
	CT_SHIFT = 4,
	COLD_SHIFT = 12,
	SWXOFF = (1 << 22),
	RTLC = (1 << 24)
};

enum TSTA
{
	DD = (1 << 0),
	EC = (1 << 1),
	LC = (1 << 2)
};

enum LSTA
{
	LSTA_TU = (1 << 3)
};

struct RXDescriptor
{
	volatile uint64_t Address;
	volatile uint16_t Length;
	volatile uint16_t Checksum;
	volatile uint8_t Status;
	volatile uint8_t Errors;
	volatile uint16_t Special;
} __attribute__((packed));

struct TXDescriptor
{
	volatile uint64_t Address;
	volatile uint16_t Length;
	volatile uint8_t cso;
	volatile uint8_t Command;
	volatile uint8_t Status;
	volatile uint8_t css;
	volatile uint16_t Special;
} __attribute__((packed));
#endif

EXTERNC int cxx_Panic();
EXTERNC int cxx_Probe();
EXTERNC int cxx_Initialize();
EXTERNC int cxx_Finalize();
