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

#include <aip.h>

#include <errno.h>
#include <io.h>

extern void Log(const char *Format, ...);

void PIC_EOI(uint8_t IRQ)
{
	if (IRQ >= 8)
		outb(PIC2_CMD, _PIC_EOI);
	outb(PIC1_CMD, _PIC_EOI);
}

void IRQ_MASK(uint8_t IRQ)
{
	uint16_t port;
	uint8_t value;

	if (IRQ < 8)
		port = PIC1_DATA;
	else
	{
		port = PIC2_DATA;
		IRQ -= 8;
	}

	value = inb(port) | (1 << IRQ);
	outb(port, value);
}

void IRQ_UNMASK(uint8_t IRQ)
{
	uint16_t port;
	uint8_t value;

	if (IRQ < 8)
		port = PIC1_DATA;
	else
	{
		port = PIC2_DATA;
		IRQ -= 8;
	}

	value = inb(port) & ~(1 << IRQ);
	outb(port, value);
}

void PS2Wait(const bool Output)
{
	int Timeout = 100000;
	PS2_STATUSES Status = {.Raw = inb(PS2_STATUS)};
	while (Timeout--)
	{
		if (!Output) /* FIXME: Reverse? */
		{
			if (Status.OutputBufferFull == 0)
				return;
		}
		else
		{
			if (Status.InputBufferFull == 0)
				return;
		}
		Status.Raw = inb(PS2_STATUS);
	}

	Log("PS/2 controller timeout! (Status: %#x, %d)", Status, Output);
}

void PS2WriteCommand(uint8_t Command)
{
	WaitInput;
	outb(PS2_CMD, Command);
}

void PS2WriteData(uint8_t Data)
{
	WaitInput;
	outb(PS2_DATA, Data);
}

uint8_t PS2ReadData()
{
	WaitOutput;
	return inb(PS2_DATA);
}

uint8_t PS2ReadStatus()
{
	WaitOutput;
	return inb(PS2_STATUS);
}

uint8_t PS2ReadAfterACK()
{
	uint8_t ret = PS2ReadData();
	while (ret == PS2_ACK)
	{
		WaitOutput;
		ret = inb(PS2_DATA);
	}
	return ret;
}

void PS2ClearOutputBuffer()
{
	PS2_STATUSES Status;
	int timeout = 0x500;
	while (timeout--)
	{
		Status.Raw = inb(PS2_STATUS);
		if (Status.OutputBufferFull == 0)
			return;
		inb(PS2_DATA);
	}
}

int PS2ACKTimeout()
{
	int timeout = 0x500;
	while (timeout > 0)
	{
		if (PS2ReadData() == PS2_ACK)
			return 0;
		timeout--;
	}
	return -ETIMEDOUT;
}
