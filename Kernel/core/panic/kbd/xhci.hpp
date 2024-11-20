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

class CrashXHCIKeyboardDriver : public Interrupts::Handler
{
private:
	struct XHCIcap
	{
		uint8_t CAPLENGTH;
		uint8_t __reserved0;
		uint16_t HCIVERSION;
		uint32_t HCSPARAMS1;
		uint32_t HCSPARAMS2;
		uint32_t HCSPARAMS3;
		union
		{
			struct
			{
				uint32_t AC64 : 1;
				uint32_t BNC : 1;
				uint32_t CSZ : 1;
				uint32_t PPC : 1;
				uint32_t PIND : 1;
				uint32_t LHRC : 1;
				uint32_t LTC : 1;
				uint32_t NSS : 1;
				uint32_t PAE : 1;
				uint32_t SPC : 1;
				uint32_t SEC : 1;
				uint32_t CFC : 1;
				uint32_t MaxPSASize : 4;
				uint32_t xHCIExtendedCapacitiesPointer : 16;

			} __packed;
			uint32_t raw;
		} HCCPARAMS1;
		uint32_t DBOFF;
		uint32_t RTSOFF;
		uint32_t HCCPARAMS2;
	} *caps __packed;

	struct XHCIop
	{
		union
		{
			struct
			{
				uint32_t RS : 1;
				uint32_t HCRST : 1;
				uint32_t INTE : 1;
				uint32_t HSEE : 1;
				uint32_t __reserved0 : 3;
				uint32_t LHCRST : 1;
				uint32_t CSS : 1;
				uint32_t CRS : 1;
				uint32_t EWE : 1;
				uint32_t EU3S : 1;
				uint32_t __reserved1 : 1;
				uint32_t CME : 1;
				uint32_t ETE : 1;
				uint32_t TSCEN : 1;
				uint32_t VTIOEN : 1;
				uint32_t __reserved2 : 15;
			} __packed;
			uint32_t raw;
		} USBCMD;
		union
		{
			struct
			{

				uint32_t HCH : 1;
				uint32_t __reserved0 : 1;
				uint32_t HSE : 1;
				uint32_t EINT : 1;
				uint32_t PCB : 1;
				uint32_t __reserved1 : 3;
				uint32_t SSS : 1;
				uint32_t RSS : 1;
				uint32_t SRE : 1;
				uint32_t CNR : 1;
				uint32_t HCE : 1;
				uint32_t __reserved2 : 18;
			} __packed;
			uint32_t raw;
		} USBSTS;
		uint32_t PAGESIZE;
		uint8_t __reserved0[8];
		uint32_t DNCTRL;
		union
		{
			struct
			{
				uint64_t RCS : 1;
				uint64_t CS : 1;
				uint64_t CA : 1;
				uint64_t CRR : 1;
				uint64_t __reserved0 : 2;
				uint64_t CRP : 58;
			} __packed;
			uint64_t raw;
		} CRCR;
		uint8_t __reserved1[16];
		uint64_t DCBAAP;
		uint32_t CONFIG;
	} *ops __packed;

	struct XHCIport
	{
		uint32_t PORTSC;
		uint32_t PORTMSC;
		uint32_t PORTLI;
		uint32_t PORTHLPMC;
	} *port __packed;

	struct XHCIruntime
	{
		uint32_t MFINDEX;
		uint32_t __reserved0[7];
		struct XHCIinterrupter
		{
			union
			{
				struct
				{
					uint32_t IP : 1;
					uint32_t IE : 1;
					uint32_t __reserved0 : 30;
				} __packed;
				uint32_t raw;
			} IMAN;
			union
			{
				struct
				{
					uint32_t IMODI : 16;
					uint32_t IMODC : 16;
				} __packed;
				uint32_t raw;
			} IMOD;
			union
			{
				struct
				{
					uint32_t ERSTSZ : 16;
					uint32_t __reserved0 : 16;
				} __packed;
				uint32_t raw;
			} ERSTSZ;
			uint32_t __reserved;
			union
			{
				struct
				{
					uint64_t __reserved0 : 6;
					uint64_t ERSTBAR : 58;
				} __packed;
				uint64_t raw;
			} ERSTBA;
			union
			{
				struct
				{
					uint64_t DESI : 3;
					uint64_t EHB : 1;
					uint64_t ERDP : 60;
				} __packed;
				uint64_t raw;
			} ERDP;
		} Interrupter[] __packed;
	} *runtime __packed;

	union XHCIdoorbell
	{
		struct
		{
			uint32_t DBTarget : 8;
			uint32_t __reserved : 8;
			uint32_t DBTaskID : 16;
		} __packed;
		uint32_t raw;
	} *doorbell;

	struct HCExtCap
	{
		union
		{
			struct
			{
				uint32_t CapID : 8;
				uint32_t NextCapPtr : 8;
				uint32_t BIOSOwnsHC : 1;
				uint32_t __reserved0 : 7;
				uint32_t OSOwnsHC : 1;
				uint32_t __reserved1 : 7;
			} __packed;
			uint32_t raw;
		} USBLEGSUP;

		union
		{
			struct
			{
				uint32_t unknown : 32;
			} __packed;
			uint32_t raw;
		} USBLEGCTLSTS;
	} __packed;

	struct XHCIprotocol
	{
		uint8_t CAPID;
		uint8_t NextCapPtr;
		uint8_t RevisionMinor;
		uint8_t RevisionMajor;
		uint8_t Name[4];
		uint8_t CompPortOffset;
		uint8_t CompPortCount;
		uint16_t ProtocolDefined : 12;
		uint8_t PSIC : 4;
		uint8_t ProtocolSlotType : 4;
		uint32_t __reserved0 : 28;
	} __packed;

	PCI::PCIDeviceHeader *Header = nullptr;
	uintptr_t ExtendedCaps = 0;
	void *baseAddrArray = nullptr;
	std::vector<XHCIprotocol *> Protocols = {};
	XHCIruntime::XHCIinterrupter *Interrupter = nullptr;

	void OnInterruptReceived(CPU::TrapFrame *Frame);
	bool TakeOwnership();

public:
	bool Initialize();

	CrashXHCIKeyboardDriver(PCI::PCIDevice dev);
	~CrashXHCIKeyboardDriver() {}
};
