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
	struct Capability
	{
		uint8_t CAPLENGTH;	 // 00h - Capability Register Length
		uint8_t rsvd0;		 // 01h - Reserved
		uint16_t HCIVERSION; // 02h - Interface Version Number
		uint32_t HCSPARAMS1; // 04h - Structural Parameters 1
		uint32_t HCSPARAMS2; // 08h - Structural Parameters 2
		uint32_t HCSPARAMS3; // 0Ch - Structural Parameters 3
		uint32_t HCCPARAMS1; // 10h - Capability Parameters 1
		uint32_t DBOFF;		 // 14h - Doorbell Offset
		uint32_t RTSOFF;	 // 18h - Runtime Register Space Offset
		uint32_t HCCPARAMS2; // 1Ch - Capability Parameters 2
		uint32_t rsvd1;		 // CAPLENGTH-20h - Reserved
	} __packed;

	static_assert(offsetof(Capability, CAPLENGTH) == 0x00);
	static_assert(offsetof(Capability, HCIVERSION) == 0x02);
	static_assert(offsetof(Capability, HCSPARAMS1) == 0x04);
	static_assert(offsetof(Capability, HCSPARAMS2) == 0x08);
	static_assert(offsetof(Capability, HCSPARAMS3) == 0x0C);
	static_assert(offsetof(Capability, HCCPARAMS1) == 0x10);
	static_assert(offsetof(Capability, DBOFF) == 0x14);
	static_assert(offsetof(Capability, RTSOFF) == 0x18);
	static_assert(offsetof(Capability, HCCPARAMS2) == 0x1C);

	struct Operational
	{
		uint32_t USBCMD;	// 00h - USB Command
		uint32_t USBSTS;	// 04h - USB Status
		uint32_t PAGESIZE;	// 08h - Page Size
		uint32_t rsvd0[2];	// 0C-13h - Reserved
		uint32_t DNCTRL;	// 14h - Device Notification Control
		uint32_t CRCR;		// 18h - Command Ring Control
		uint32_t rsvd1[5];	// 20-2Fh - Reserved
		uint32_t DCBAAP[2]; // 30h - Device Context Base Address Array Pointer
		uint32_t CONFIG;	// 38h - Configure
		uint32_t rsvd2[13]; // 3C-3FFh - Reserved

		// Port Register Set at 400-13FFh
	} __packed;

	static_assert(offsetof(Operational, USBCMD) == 0x00);
	static_assert(offsetof(Operational, USBSTS) == 0x04);
	static_assert(offsetof(Operational, PAGESIZE) == 0x08);
	static_assert(offsetof(Operational, DNCTRL) == 0x14);
	static_assert(offsetof(Operational, CRCR) == 0x18);
	static_assert(offsetof(Operational, DCBAAP) == 0x30);
	static_assert(offsetof(Operational, CONFIG) == 0x38);

	struct PortRegister
	{
		uint32_t PORTSC;	// 0h - Port Status and Control
		uint32_t PORTPMSC;	// 4h - Port Power Management Status and Control
		uint32_t PORTLI;	// 8h - Port Link Info
		uint32_t PORTHLPMC; // Ch - Port Hardware LPM Control
	} __packed;

	static_assert(offsetof(PortRegister, PORTSC) == 0x00);
	static_assert(offsetof(PortRegister, PORTPMSC) == 0x04);
	static_assert(offsetof(PortRegister, PORTLI) == 0x08);
	static_assert(offsetof(PortRegister, PORTHLPMC) == 0x0C);

	struct Runtime
	{
		uint32_t MFINDEX;  // 0000h - Microframe Index
		uint32_t rsvd0[7]; // 0004h-001Fh - Reserved
						   // Interrupter Register Sets at 0020h onwards (IR0-IR1023)
	} __packed;

	static_assert(offsetof(Runtime, MFINDEX) == 0x0000);

	struct Doorbell
	{
	} __packed;

	class HCD : public Interrupts::Handler, public USBController
	{
	private:
		PCI::PCIDevice Header;
		uintptr_t MMIOBase;
		Capability *Cap;
		Operational *Op;
		Runtime *Rt;
		PortRegister *Ports;
		Doorbell *Db;

		void OnInterruptReceived(CPU::TrapFrame *Frame) final;

	public:
		int Reset();
		int Start(bool WaitForStart);
		int Stop();
		int Detect();
		int Poll();
		HCD(PCI::PCIDevice &pciHeader);
		virtual ~HCD();
	};
}
