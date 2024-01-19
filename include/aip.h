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

#ifndef __FENNIX_API_AIP_H__
#define __FENNIX_API_AIP_H__

#include <types.h>
#include <aip/kbd.h>
#include <aip/mouse.h>

#define PIC1_CMD 0x20
#define PIC1_DATA (PIC1_CMD + 1)
#define PIC2_CMD 0xA0
#define PIC2_DATA (PIC2_CMD + 1)
#define _PIC_EOI 0x20

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD PS2_STATUS
#define PS2_ACK 0xFA
#define PS2_TEST_PASSED 0x55
#define PS2_TEST_FAILED 0xFC

#define PS2_CMD_READ_CONFIG 0x20
#define PS2_CMD_READ_CONFIG_N(n) (PS2_CMD_READ_CONFIG + n)
#define PS2_CMD_WRITE_CONFIG 0x60
#define PS2_CMD_WRITE_CONFIG_N(n) (PS2_CMD_WRITE_CONFIG + n)
#define PS2_CMD_DISABLE_PORT_2 0xA7
#define PS2_CMD_ENABLE_PORT_2 0xA8
#define PS2_CMD_TEST_PORT_2 0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT_1 0xAB
#define PS2_CMD_DIAGNOSTIC_DUMP 0xAC
#define PS2_CMD_DISABLE_PORT_1 0xAD
#define PS2_CMD_ENABLE_PORT_1 0xAE
#define PS2_CMD_READ_INPUT_PORT 0xC0
#define PS2_CMD_COPY_INPUT_0_3_TO_4_7_STATUS 0xC1
#define PS2_CMD_COPY_INPUT_4_7_TO_4_7_STATUS 0xC2
#define PS2_CMD_READ_OUTPUT_PORT 0xD0
#define PS2_CMD_WRITE_NEXT_BYTE_TO_OUTPUT_PORT 0xD1
#define PS2_CMD_WRITE_NEXT_BYTE_TO_PS2_PORT_1_OUTPUT 0xD2
#define PS2_CMD_WRITE_NEXT_BYTE_TO_PS2_PORT_2_OUTPUT 0xD3
#define PS2_CMD_WRITE_NEXT_BYTE_TO_PS2_PORT_2_INPUT 0xD4
#define PS2_CMD_PULSE_OUTPUT_LINE(n) (0xF0 + n)

typedef union
{
	struct
	{
		uint8_t OutputBufferFull : 1;
		uint8_t InputBufferFull : 1;
		uint8_t SystemFlag : 1;
		uint8_t CommandData : 1;
		uint8_t Unknown1 : 1;
		uint8_t Unknown2 : 1;
		uint8_t TimeoutError : 1;
		uint8_t ParityError : 1;
	};
	uint8_t Raw;
} PS2_STATUSES;

typedef union
{
	struct
	{
		uint8_t Port1Interrupt : 1;
		uint8_t Port2Interrupt : 1;
		uint8_t SystemFlag : 1;
		uint8_t Zero0 : 1;
		uint8_t Port1Clock : 1;
		uint8_t Port2Clock : 1;
		uint8_t Port1Translation : 1;
		uint8_t Zero1 : 1;
	};
	uint8_t Raw;
} PS2_CONFIGURATION;

typedef union
{
	struct
	{
		uint8_t SystemReset : 1;
		uint8_t A20Gate : 1;
		uint8_t Port2Clock : 1;
		uint8_t Port2Data : 1;
		uint8_t Port1OutputBufferFull : 1;
		uint8_t Port2OutputBufferFull : 1;
		uint8_t Port1InputBufferFull : 1;
		uint8_t Port2InputBufferFull : 1;
	};
	uint8_t Raw;
} PS2_OUTPUT_PORT;

void PIC_EOI(uint8_t IRQ);
void IRQ_MASK(uint8_t IRQ);
void IRQ_UNMASK(uint8_t IRQ);
void PS2Wait(const bool Output);
void PS2WriteCommand(uint8_t Command);
void PS2WriteData(uint8_t Data);
uint8_t PS2ReadData();
uint8_t PS2ReadStatus();
uint8_t PS2ReadAfterACK();
void PS2ClearOutputBuffer();
int PS2ACKTimeout();

#define WaitOutput PS2Wait(true)
#define WaitInput PS2Wait(false)

#endif // !__FENNIX_API_AIP_H__
