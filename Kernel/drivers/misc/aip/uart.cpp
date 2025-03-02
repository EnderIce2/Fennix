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

#include <driver.hpp>
#include <io.h>

#include "aip.hpp"

namespace Driver::AdvancedIntegratedPeripheral
{
	extern dev_t DriverID;

#define SERIAL_ENABLE_DLAB 0x80
#define SERIAL_BUFFER_EMPTY 0x20

	enum Ports
	{
		COM1 = 0x3F8,
		COM2 = 0x2F8,
		COM3 = 0x3E8,
		COM4 = 0x2E8,
		COM5 = 0x5F8,
		COM6 = 0x4F8,
		COM7 = 0x5E8,
		COM8 = 0x4E8,

		LPT1 = 0x378,
		LPT2 = 0x278,
		LPT3 = 0x3BC
	};

	enum SerialSpeed
	{
		RATE_50_HI = 0x09,
		RATE_50_LO = 0x00,

		RATE_300_HI = 0x01,
		RATE_300_LO = 0x80,

		RATE_600_HI = 0x00,
		RATE_600_LO = 0xC0,

		RATE_2400_HI = 0x00,
		RATE_2400_LO = 0x30,

		RATE_4800_HI = 0x00,
		RATE_4800_LO = 0x18,

		RATE_9600_HI = 0x00,
		RATE_9600_LO = 0x0C,

		RATE_19200_HI = 0x00,
		RATE_19200_LO = 0x06,

		RATE_38400_HI = 0x00,
		RATE_38400_LO = 0x03,

		RATE_57600_HI = 0x00,
		RATE_57600_LO = 0x02,

		RATE_115200_HI = 0x00,
		RATE_115200_LO = 0x01
	};

	/*
	.                          Table of Registers                         .
	/---------------------------------------------------------------------\
	| Base Address | DLAB | R/W | Abr |           Register Name           |
	|---------------------------------------------------------------------|
	|      +0      |  =0  |  W  |  -  |    Transmitter Holding Buffer     |
	|              |  =0  |  R  |  -  |          Receiver Buffer          |
	|              |  =1  | R/W |  -  |      Divisor Latch Low Byte       |
	|      +1      |  =0  | R/W | IER |     Interrupt Enable Register     |
	|              |  =1  | R/W |  -  |      Divisor Latch High Byte      |
	|      +2      |  -   |  R  | IIR | Interrupt Identification Register |
	|              |  -   |  W  | FCR |       FIFO Control Register       |
	|      +3      |  -   | R/W | LCR |       Line Control Register       |
	|      +4      |  -   | R/W | MCR |       Modem Control Register      |
	|      +5      |  -   |  R  | LSR |       Line Status Register        |
	|      +6      |  -   |  R  | MSR |       Modem Status Register       |
	|      +7      |  -   | R/W |  -  |         Scratch Register          |
	\---------------------------------------------------------------------/

	  Source:
		Interfacing the Serial / RS232 Port V5.0
		  Table 5 : Table of Registers
	*/

	/** Interrupt Enable Register */
	typedef union
	{
		struct
		{
			/* Enable Received Data Available Interrupt */
			uint8_t InterruptOnReceive : 1;

			/* Enable Transmitter Holding Register Empty Interrupt */
			uint8_t InterruptOnTransmitter : 1;

			/* Enable Receiver Line Status Interrupt */
			uint8_t LineStatusInterrupt : 1;

			/* Enable Modem Status Interrupt */
			uint8_t ModemStatusInterrupt : 1;

			/* Enables Sleep Mode (16750) */
			uint8_t SleepMode : 1;

			/* Enables Low Power Mode (16750) */
			uint8_t LowPowerMode : 1;

			/* Reserved */
			uint8_t __reserved : 2;
		};
		uint8_t raw;
	} IER;

	/** Interrupt Identification Register */
	typedef union
	{
		struct
		{
			/* Interrupt pending */
			uint8_t InterruptPending : 1;

			/**
			 * Interrupt Status
			 *
			 * 00b = Modem Status Interrupt
			 * 01b = Transmitter Holding Register Empty Interrupt
			 * 10b = Received Data Available Interrupt
			 * 11b = Receiver Line Status Interrupt
			 */
			uint8_t InterruptStatus : 2;

			/**
			 * 16550 Time-out Interrupt Pending
			 *
			 * @note Reserved on 8250, 16450
			 */
			uint8_t TimeOutIP : 1;

			/** Reserved */
			uint8_t __reserved : 1;

			/** 64 Byte Fifo Enabled (16750 only) */
			uint8_t FIFO64 : 1;

			/**
			 * Enable FIFO
			 *
			 * 00b = No FIFO
			 * 01b = FIFO Enabled but Unusable
			 * 11b = FIFO Enabled
			 */
			uint8_t FIFO : 2;
		};
		uint8_t raw;
	} IIR;

	/** First In / First Out Control Register */
	typedef union
	{
		struct
		{
			/** Enable FIFO's */
			uint8_t FIFO : 1;

			/** Clear Receive FIFO */
			uint8_t ClearRX : 1;

			/** Clear Transmit FIFO */
			uint8_t ClearTX : 1;

			/** DMA Mode Select.
			 *
			 * Change status of RXRDY & TXRDY pins from mode 1 to mode 2.
			 */
			uint8_t DMAMode : 1;

			/** Reserved */
			uint8_t __reserved : 1;

			/** Enable 64 Byte FIFO (16750 only) */
			uint8_t FIFO64 : 1;

			/** Interrupt Trigger Level
			 *
			 * 00b = 1 Byte
			 * 01b = 4 Bytes
			 * 10b = 8 Bytes
			 * 11b = 14 Bytes
			 */
			uint8_t TriggerLevel : 2;
		};
		uint8_t raw;
	} FCR;

	/** Line Control Register */
	typedef union
	{
		struct
		{
			/** Word Length
			 *
			 * 00b = 5 bits
			 * 01b = 6 bits
			 * 10b = 7 bits
			 * 11b = 8 bits
			 */
			uint8_t WordLength : 2;

			/** Length of Stop Bit
			 *
			 * 0b = One Stop Bit
			 * 1b = 2 Stop bits for words of length 6,7 or 8 bits or 1.5 Stop Bits for Word lengths of 5 bits.
			 */
			uint8_t StopBit : 1;

			/** Parity Select
			 *
			 *   0b = No Parity
			 * 001b = Odd Parity
			 * 011b = Even Parity
			 * 101b = High Parity (Sticky)
			 * 111b = Low Parity (Sticky)
			 */
			uint8_t Parity : 3;

			/** Set Break Enable */
			uint8_t SetBreak : 1;

			/**
			 * Divisor Latch Access
			 *
			 * 0b = Access to Receiver buffer, Transmitter buffer & Interrupt Enable Register
			 * 1b = Divisor Latch Access Bit
			 */
			uint8_t DLAB : 1;
		};
		uint8_t raw;
	} LCR;

	/** Modem Control Register */
	typedef union
	{
		struct
		{
			/** Force Data Terminal Ready */
			uint8_t DataTerminalReady : 1;

			/** Force Request to Send */
			uint8_t RequestToSend : 1;

			/** Auxiliary Output 1 */
			uint8_t Out1 : 1;

			/** Auxiliary Output 2 */
			uint8_t Out2 : 1;

			/** Loopback Mode */
			uint8_t Loopback : 1;

			/** Autoflow Control Enabled (16750 only) */
			uint8_t Autoflow : 1;

			/** Reserved */
			uint8_t __reserved : 2;
		};
		uint8_t raw;
	} MCR;

	/** Line Status Register */
	typedef union
	{
		struct
		{
			/** Data Ready */
			uint8_t DataReady : 1;

			/** Overrun Error */
			uint8_t OverrunError : 1;

			/** Parity Error */
			uint8_t ParityError : 1;

			/** Framing Error */
			uint8_t FramingError : 1;

			/** Break Interrupt */
			uint8_t BreakInterrupt : 1;

			/** Empty Transmitter Holding Register */
			uint8_t EmptyTransmitterHolding : 1;

			/** Empty Data Holding Registers */
			uint8_t EmptyDataHolding : 1;

			/** Error in Received FIFO */
			uint8_t ErrorReceivedFIFO : 1;
		};
		uint8_t raw;
	} LSR;

	/** Modem Status Register */
	typedef union
	{
		struct
		{
			/** Delta Clear to Send */
			uint8_t DeltaClearToSend : 1;

			/** Delta Data Set Ready */
			uint8_t DeltaDataSetReady : 1;

			/** Trailing Edge Ring Indicator */
			uint8_t TrailingEdgeRingIndicator : 1;

			/** Delta Data Carrier Detect */
			uint8_t DeltaDataCarrierDetect : 1;

			/** Clear To Send */
			uint8_t ClearToSend : 1;

			/** Data Set Ready */
			uint8_t DataSetReady : 1;

			/** Ring Indicator */
			uint8_t RingIndicator : 1;

			/** Carrier Detect */
			uint8_t CarrierDetect : 1;
		};
		uint8_t raw;
	} MSR;

	union UARTs
	{
		struct
		{
			uint8_t com1 : 1;
			uint8_t com2 : 1;
			uint8_t com3 : 1;
			uint8_t com4 : 1;
			uint8_t com5 : 1;
			uint8_t com6 : 1;
			uint8_t com7 : 1;
			uint8_t com8 : 1;

			uint8_t lpt1 : 1;
			uint8_t lpt2 : 1;
			uint8_t lpt3 : 1;

			uint8_t __reserved : 5;
		};
		uint16_t raw;
	} uart;

	bool IsDataReady(uint16_t Port)
	{
		LSR lsr;
		lsr.raw = inb(Port + 5);
		return lsr.DataReady;
	}

	bool IsTransmitEmpty(uint16_t Port)
	{
		LSR lsr;
		lsr.raw = inb(Port + 5);
		return lsr.EmptyTransmitterHolding;
	}

	char ReadSerial(uint16_t Port)
	{
		while (!IsDataReady(Port))
			v0::Yield(DriverID);
		return inb(Port);
	}

	void WriteSerial(uint16_t Port, char Character)
	{
		while (!IsTransmitEmpty(Port))
			v0::Yield(DriverID);
		outb(Port, Character);
	}

	void ReportSerialReceived(uint8_t Data)
	{
		debug("%c", Data);
	}

	void UartCOM24(CPU::TrapFrame *)
	{
		LSR lsr2, lsr4;
		do
		{
			lsr2.raw = inb(COM2 + 5);
			if (lsr2.DataReady)
				ReportSerialReceived(inb(COM2));
			lsr4.raw = inb(COM4 + 5);
			if (lsr4.DataReady)
				ReportSerialReceived(inb(COM4));
		} while (lsr2.DataReady || lsr4.DataReady);
	}

	void UartCOM13(CPU::TrapFrame *)
	{
		LSR lsr1, lsr3;
		do
		{
			lsr1.raw = inb(COM1 + 5);
			if (lsr1.DataReady)
				ReportSerialReceived(inb(COM1));
			lsr3.raw = inb(COM3 + 5);
			if (lsr3.DataReady)
				ReportSerialReceived(inb(COM3));
		} while (lsr1.DataReady || lsr3.DataReady);
	}

	bool InitializePort(uint16_t Port)
	{
		v0::CriticalState cs = v0::EnterCriticalSection(DriverID);
		LCR lcr = {};
		IER ier = {};
		FCR fcr = {};
		MCR mcr = {};

		outb(Port + 3, lcr.raw);
		outb(Port + 1, ier.raw);

		lcr.DLAB = 1;
		outb(Port + 3, lcr.raw);

		outb(Port + 0, RATE_115200_LO);
		outb(Port + 1, RATE_115200_HI);

		lcr.DLAB = 0;
		lcr.WordLength = 0b11;
		outb(Port + 3, lcr.raw);

		fcr.FIFO = 1;
		fcr.ClearRX = 1;
		fcr.ClearTX = 1;
		fcr.TriggerLevel = 0b11;
		outb(Port + 2, fcr.raw);

		mcr.DataTerminalReady = 1;
		mcr.RequestToSend = 1;
		mcr.Out2 = 1;
		mcr.Loopback = 1;
		outb(Port + 4, mcr.raw);

		/* Test the serial port */
		outb(Port + 0, 0x48);
		uint8_t result = inb(Port + 0);
		if (result != 0x48)
		{
			/* FIXME: DETECT BAUD RATE
				Do multiple test to check if the output is garbage.
				If so, reduce the baud rate until it works. */

			v0::LeaveCriticalSection(DriverID, cs);
			trace("Port %#X test failed!", Port);
			return false;
		}

		/* Set normal operation mode */
		mcr.DataTerminalReady = 1;
		mcr.RequestToSend = 1;
		mcr.Out1 = 1;
		mcr.Out2 = 1;
		mcr.Loopback = 0;
		outb(Port + 4, mcr.raw);

		/* Enable interrupts on receive */
		ier.InterruptOnReceive = 1;
		outb(Port + 1, ier.raw);
		v0::RegisterInterruptHandler(DriverID, 3, (void *)UartCOM24);
		v0::RegisterInterruptHandler(DriverID, 4, (void *)UartCOM13);

		v0::LeaveCriticalSection(DriverID, cs);
		trace("Port %#X initialized", Port);
		return true;
	}

	int DetectUART()
	{
		uart.com1 = inb(COM1) != 0xFF ? true : false;
		uart.com2 = inb(COM2) != 0xFF ? true : false;
		uart.com3 = inb(COM3) != 0xFF ? true : false;
		uart.com4 = inb(COM4) != 0xFF ? true : false;
		uart.com5 = inb(COM5) != 0xFF ? true : false;
		uart.com6 = inb(COM6) != 0xFF ? true : false;
		uart.com7 = inb(COM7) != 0xFF ? true : false;
		uart.com8 = inb(COM8) != 0xFF ? true : false;

		uart.lpt1 = inb(LPT1) != 0xFF ? true : false;
		uart.lpt2 = inb(LPT2) != 0xFF ? true : false;
		uart.lpt3 = inb(LPT3) != 0xFF ? true : false;

		if (uart.com1 == true)
			if (InitializePort(COM1) == false)
				uart.com1 = false;

		if (uart.com2 == true)
			if (InitializePort(COM2) == false)
				uart.com1 = false;

		if (uart.com3 == true)
			if (InitializePort(COM3) == false)
				uart.com1 = false;

		if (uart.com4 == true)
			if (InitializePort(COM4) == false)
				uart.com1 = false;

		if (uart.com5 == true)
			if (InitializePort(COM5) == false)
				uart.com1 = false;

		if (uart.com6 == true)
			if (InitializePort(COM6) == false)
				uart.com1 = false;

		if (uart.com7 == true)
			if (InitializePort(COM7) == false)
				uart.com1 = false;

		if (uart.com8 == true)
			if (InitializePort(COM8) == false)
				uart.com1 = false;

		if (uart.lpt1 == true)
			trace("LPT1 is present");

		if (uart.lpt2 == true)
			trace("LPT2 is present");

		if (uart.lpt3 == true)
			trace("LPT3 is present");
		return 0;
	}

	// static int once = 0;
	// static uint8_t com4 = 0xFF;
	// if (!once++)
	// 	com4 = inb(0x2E8);
	// if (com4 == 0xFF)
	// 	CPU::Halt(true);
	// char UserInputBuffer[256]{'\0'};
	// int BackSpaceLimit = 0;
	// while (true)
	// {
	// 	while ((inb(0x2E8 + 5) & 1) == 0)
	// 		CPU::Pause();
	// 	char key = inb(0x2E8);
	// 	// debug("key: %d", key);
	// 	if (key == '\x7f') /* Backspace (DEL) */
	// 	{
	// 		if (BackSpaceLimit <= 0)
	// 			continue;
	// 		char keyBuf[5] = {'\b', '\x1b', '[', 'K', '\0'};
	// 		ExPrint(keyBuf);
	// 		backspace(UserInputBuffer);
	// 		BackSpaceLimit--;
	// 		continue;
	// 	}
	// 	else if (key == '\x0d') /* Enter (CR) */
	// 	{
	// 		UserInput(UserInputBuffer);
	// 		BackSpaceLimit = 0;
	// 		UserInputBuffer[0] = '\0';
	// 		continue;
	// 	}
	// 	else if (key == '\x1b') /* Escape */
	// 	{
	// 		char tmp[16]{'\0'};
	// 		append(tmp, key);
	// 		while ((inb(0x2E8 + 5) & 1) == 0)
	// 			CPU::Pause();
	// 		char key = inb(0x2E8);
	// 		append(tmp, key);
	// 		if (key == '[')
	// 		{
	// 			// 27 91
	// 			// <  68
	// 			// > 67
	// 			// 	down 66
	// 			// 	up 65
	// 			while ((inb(0x2E8 + 5) & 1) == 0)
	// 				CPU::Pause();
	// 			key = inb(0x2E8);
	// 			append(tmp, key);
	// 			switch (key)
	// 			{
	// 			case 'A':
	// 				key = KEY_D_UP;
	// 				break;
	// 			case 'B':
	// 				key = KEY_D_DOWN;
	// 				break;
	// 			case 'C':
	// 				key = KEY_D_RIGHT;
	// 				break;
	// 			case 'D':
	// 				key = KEY_D_LEFT;
	// 				break;
	// 			default:
	// 			{
	// 				for (size_t i = 0; i < strlen(tmp); i++)
	// 				{
	// 					if ((int)sizeof(UserInputBuffer) <= BackSpaceLimit)
	// 						continue;
	// 					append(UserInputBuffer, tmp[i]);
	// 					BackSpaceLimit++;
	// 					char keyBuf[2] = {(char)tmp[i], '\0'};
	// 					ExPrint(keyBuf);
	// 				}
	// 				continue;
	// 			}
	// 			}
	// 			ArrowInput(key);
	// 			continue;
	// 		}
	// 	}
	// 	if ((int)sizeof(UserInputBuffer) <= BackSpaceLimit)
	// 		continue;
	// 	append(UserInputBuffer, key);
	// 	BackSpaceLimit++;
	// 	char keyBuf[2] = {(char)key, '\0'};
	// 	ExPrint(keyBuf);
	// }
}
