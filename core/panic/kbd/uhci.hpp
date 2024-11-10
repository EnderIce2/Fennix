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

#include <ints.hpp>

class CrashUHCIKeyboardDriver : public Interrupts::Handler
{
public:
	struct IORegisters
	{
		union
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
			uint16_t raw;
		} USBCMD;
		union
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
			uint16_t raw;
		} USBSTS;
		union
		{
			struct
			{
				uint16_t TOCRC : 1;
				uint16_t RIE : 1;
				uint16_t IOCE : 1;
				uint16_t SPIE : 1;
				uint16_t __reserved0 : 12;
			} __packed;
			uint16_t raw;
		} USBINTR;
		union
		{
			struct
			{
				uint16_t FN : 9;
				uint16_t __reserved0 : 7;
			} __packed;
			uint16_t raw;
		} FRNUM;
		union
		{
			struct
			{
				uint32_t __reserved0 : 12;
				uint32_t BA : 20;
			} __packed;
			uint32_t raw;
		} FRBASEADD;
		union
		{
			struct
			{
				uint8_t SOFTVAL : 7;
				uint8_t __reserved0 : 1;
			} __packed;
			uint8_t raw;
		} SOFMOD;
		union
		{
			struct
			{
				uint16_t CCS : 1;
				uint16_t CSC : 1;
				uint16_t PE : 1;
				uint16_t PEC : 1;
				uint16_t LS : 1;
				uint16_t RD : 1;
				uint16_t __reserved0 : 1; // always 1
				uint16_t LSDA : 1;
				uint16_t PR : 1;
				uint16_t __reserved1 : 2;
				uint16_t SUS : 1;
				uint16_t __reserved2 : 4;
			} __packed;
			uint16_t raw;
		} PORTSC[2];
	} __packed;

	struct TD
	{
		union
		{
			struct
			{
				uint32_t T : 1;
				uint32_t Q : 1;
				uint32_t Vf : 1;
				uint32_t __reserved0 : 1;
				uint32_t LP : 28;
			} __packed;
			uint32_t raw;
		} LINK;
		union
		{
			struct
			{
				uint32_t ActLen : 11;
				uint32_t __reserved0 : 5;
				uint32_t STATUS : 8;
				uint32_t IOC : 1;
				uint32_t IOS : 1;
				uint32_t LS : 1;
				uint32_t unknown_0 : 2; /* missing in the spec @ 3.2.2 */
				uint32_t SPD : 1;
				uint32_t __reserved1 : 2;
			} __packed;
			uint32_t raw;
		} CS;
		union
		{
			struct
			{
				uint32_t PID : 8;
				uint32_t DeviceAddress : 6;
				uint32_t EndPt : 4;
				uint32_t D : 1;
				uint32_t __reserved0 : 1;
				uint32_t MaxLen : 11;
			} __packed;
			uint32_t raw;
		} TOKEN;
		union
		{
			struct
			{
				uint32_t Addr : 32;
			} __packed;
			uint32_t raw;
		} BUFFER;

		/* The last 4 DWords of the
		   Transfer Descriptor are
		   reserved for use by software.
			  - UHCI Design Guide 1.1 @ 3.2.5
		*/

		uint32_t __padding[4];
	} *td __aligned(16) __packed;

	struct QH
	{
		union
		{
			struct
			{
				uint32_t T : 1;
				uint32_t Q : 1;
				uint32_t __reserved0 : 2; /* must be 0 */
				uint32_t QHLP : 28;
			} __packed;
			uint32_t raw;
		} HEAD;
		union
		{
			struct
			{
				uint32_t T : 1;
				uint32_t Q : 1;
				uint32_t __reserved0 : 1;
				uint32_t __reserved1 : 1; /* must be 0 */
				uint32_t QELP : 28;
			} __packed;
			uint32_t raw;
		} ELEMENT;

		/* FIXME: It is the same with TD? */
	} *qh __aligned(16) __packed;

	PCI::PCIDeviceHeader *Header = nullptr;
	IORegisters *io = nullptr;
	uint32_t *FrameList = nullptr;

	void OnInterruptReceived(CPU::TrapFrame *Frame);
	bool Initialize();
	CrashUHCIKeyboardDriver(PCI::PCIDevice dev);
	~CrashUHCIKeyboardDriver() = default;
};
