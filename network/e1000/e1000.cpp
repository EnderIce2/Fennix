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

#include <netools.h>
#include <errno.h>
#include <regs.h>
#include <base.h>
#include <pci.h>
#include <network.h>
#include <io.h>

#include "e1000.hpp"

class E1000Device
{
private:
	PCIHeader0 *Header;
	uint16_t DeviceID;
	bool Initialized = false;

	bool EEPROMAvailable;

	struct BARData
	{
		uint8_t Type;
		uint16_t IOBase;
		uint64_t MemoryBase;
	} BAR;

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8
	RXDescriptor *RX[E1000_NUM_RX_DESC];
	TXDescriptor *TX[E1000_NUM_TX_DESC];

	uint16_t RXCurrent;
	uint16_t TXCurrent;

	const int BaseBufferSize = 8192;
	const int AdditionalBytes = 16;

	uint32_t CurrentPacket;

	void WriteCMD(uint16_t Address, uint32_t Value)
	{
		if (BAR.Type == 0)
			mmoutl((void *)(BAR.MemoryBase + Address), Value);
		else
		{
			outl(BAR.IOBase, Address);
			outl(BAR.IOBase + 4, Value);
		}
	}

	uint32_t ReadCMD(uint16_t Address)
	{
		if (BAR.Type == 0)
			return mminl((void *)(BAR.MemoryBase + Address));
		else
		{
			outl(BAR.IOBase, Address);
			return inl(BAR.IOBase + 0x4);
		}
	}

	uint32_t ReadEEPROM(uint8_t Address)
	{
		uint16_t Data = 0;
		uint32_t temp = 0;
		if (EEPROMAvailable)
		{
			WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 8));
			while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 4)))
				;
		}
		else
		{
			WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 2));
			while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 1)))
				;
		}
		Data = (uint16_t)((temp >> 16) & 0xFFFF);
		return Data;
	}

	void InitializeRX()
	{
		DebugLog("Initializing RX...");
		uintptr_t Ptr = (uintptr_t)AllocateMemory(TO_PAGES(sizeof(RXDescriptor) *
															   E1000_NUM_RX_DESC +
														   AdditionalBytes));

		for (int i = 0; i < E1000_NUM_RX_DESC; i++)
		{
			RX[i] = (RXDescriptor *)(Ptr + i * 16);
			RX[i]->Address = (uint64_t)AllocateMemory(TO_PAGES(BaseBufferSize + AdditionalBytes));
			RX[i]->Status = 0;
		}

#pragma GCC diagnostic ignored "-Wshift-count-overflow"

		WriteCMD(REG::TXDESCLO, (uint32_t)(Ptr >> 32));
		WriteCMD(REG::TXDESCHI, (uint32_t)(Ptr & 0xFFFFFFFF));

		WriteCMD(REG::RXDESCLO, (uint32_t)Ptr);
		WriteCMD(REG::RXDESCHI, 0);

		WriteCMD(REG::RXDESCLEN, E1000_NUM_RX_DESC * 16);

		WriteCMD(REG::RXDESCHEAD, 0);
		WriteCMD(REG::RXDESCTAIL, E1000_NUM_RX_DESC - 1);
		RXCurrent = 0;
		WriteCMD(REG::RCTRL, RCTL::EN | RCTL::SBP | RCTL::UPE |
								 RCTL::MPE | RCTL::LBM_NONE |
								 RTCL::RDMTS_HALF | RCTL::BAM |
								 RCTL::SECRC | RCTL::BSIZE_8192);
	}

	void InitializeTX()
	{
		DebugLog("Initializing TX...");
		uintptr_t Ptr = (uintptr_t)AllocateMemory(TO_PAGES(sizeof(TXDescriptor) *
															   E1000_NUM_RX_DESC +
														   AdditionalBytes));

		for (short i = 0; i < E1000_NUM_TX_DESC; i++)
		{
			TX[i] = (TXDescriptor *)((uintptr_t)Ptr + i * 16);
			TX[i]->Address = 0;
			TX[i]->Command = 0;
			TX[i]->Status = TSTA::DD;
		}

		WriteCMD(REG::TXDESCHI, (uint32_t)((uint64_t)Ptr >> 32));
		WriteCMD(REG::TXDESCLO, (uint32_t)((uint64_t)Ptr & 0xFFFFFFFF));

		WriteCMD(REG::TXDESCLEN, E1000_NUM_TX_DESC * 16);

		WriteCMD(REG::TXDESCHEAD, 0);
		WriteCMD(REG::TXDESCTAIL, 0);
		TXCurrent = 0;
		WriteCMD(REG::TCTRL, TCTL::EN_ | TCTL::PSP |
								 (15 << TCTL::CT_SHIFT) |
								 (64 << TCTL::COLD_SHIFT) |
								 TCTL::RTLC);

		WriteCMD(REG::TCTRL, 0b0110000000000111111000011111010);
		WriteCMD(REG::TIPG, 0x0060200A);
	}

public:
	dev_t ID;

	bool IsInitialized() { return Initialized; }

	size_t write(uint8_t *Buffer, size_t Size)
	{
		TX[TXCurrent]->Address = (uint64_t)Buffer;
		TX[TXCurrent]->Length = (uint16_t)Size;
		TX[TXCurrent]->Command = CMD::EOP | CMD::IFCS | CMD::RS;
		TX[TXCurrent]->Status = 0;
		uint16_t OldTXCurrent = TXCurrent;
		TXCurrent = (uint16_t)((TXCurrent + 1) % E1000_NUM_TX_DESC);
		WriteCMD(REG::TXDESCTAIL, TXCurrent);
		while (!(TX[OldTXCurrent]->Status & 0xFF))
			Yield();
		return Size;
	}

	MediaAccessControl GetMAC()
	{
		MediaAccessControl mac;
		if (EEPROMAvailable)
		{
			uint32_t temp;
			temp = ReadEEPROM(0);
			mac.Address[0] = temp & 0xff;
			mac.Address[1] = (uint8_t)(temp >> 8);
			temp = ReadEEPROM(1);
			mac.Address[2] = temp & 0xff;
			mac.Address[3] = (uint8_t)(temp >> 8);
			temp = ReadEEPROM(2);
			mac.Address[4] = temp & 0xff;
			mac.Address[5] = (uint8_t)(temp >> 8);
		}
		else
		{
			uint8_t *BaseMac8 = (uint8_t *)(BAR.MemoryBase + 0x5400);
			uint32_t *BaseMac32 = (uint32_t *)(BAR.MemoryBase + 0x5400);
			if (BaseMac32[0] != 0)
				for (int i = 0; i < 6; i++)
					mac.Address[i] = BaseMac8[i];
			else
			{
				KernelLog("No MAC address found.");
				return MediaAccessControl();
			}
		}

		return mac;
	}

	int ioctl(NetIoctl req, void *arg)
	{
		switch (req)
		{
		case IOCTL_NET_GET_MAC:
		{
			MediaAccessControl mac = GetMAC();
			*((uint48_t *)arg) = mac.ToHex(); /* UNTESTED */
			return 0;
		}
		default:
			return -EINVAL;
		}
		return 0;
	}

	void OnInterruptReceived(TrapFrame *)
	{
		WriteCMD(REG::IMASK, 0x1);
		uint32_t status = ReadCMD(0xC0);
		UNUSED(status);

		while ((RX[RXCurrent]->Status & 0x1))
		{
			uint8_t *data = (uint8_t *)RX[RXCurrent]->Address;
			uint16_t dataSz = RX[RXCurrent]->Length;

			// ReportNetworkPacket(ID, data, dataSz);
			/* FIXME: Implement */
			KernelLog("FIXME: Received packet");
			(void)data;
			(void)dataSz;

			RX[RXCurrent]->Status = 0;
			uint16_t OldRXCurrent = RXCurrent;
			RXCurrent = (uint16_t)((RXCurrent + 1) % E1000_NUM_RX_DESC);
			WriteCMD(REG::RXDESCTAIL, OldRXCurrent);
		}
	}

	void Panic()
	{
		WriteCMD(REG::IMASK, 0x00000000);
		WriteCMD(REG::ITR, 0x00000000);
		WriteCMD(REG::IAM, 0x00000000);
	}

	E1000Device(PCIHeader0 *_Header, uint16_t _DeviceID)
		: Header(_Header),
		  DeviceID(_DeviceID)
	{
		uint32_t PCIBAR0 = Header->BAR0;
		uint32_t PCIBAR1 = Header->BAR1;
		BAR.Type = PCIBAR0 & 1;
		BAR.IOBase = (uint16_t)(PCIBAR0 & (~3));
		BAR.MemoryBase = PCIBAR1 & (~15);

		switch (DeviceID)
		{
		case 0x100E:
		{
			KernelLog("Found Intel 82540EM Gigabit Ethernet Controller.");

			/* Detect EEPROM */
			WriteCMD(REG::EEPROM, 0x1);
			for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
				if (ReadCMD(REG::EEPROM) & 0x10)
					EEPROMAvailable = true;
				else
					EEPROMAvailable = false;

			if (!GetMAC().Valid())
			{
				KernelLog("Failed to get MAC");
				return;
			}

			/* Start link */
			uint32_t cmdret = ReadCMD(REG::CTRL);
			WriteCMD(REG::CTRL, cmdret | ECTRL::SLU);

			for (int i = 0; i < 0x80; i++)
				WriteCMD((uint16_t)(0x5200 + i * 4), 0);

			WriteCMD(REG::IMASK, 0x1F6DC);
			WriteCMD(REG::IMASK, 0xFF & ~4);
			ReadCMD(0xC0);

			InitializeRX();
			InitializeTX();
			break;
		}
		default:
		{
			KernelLog("Unimplemented E1000 device.");
			return;
		}
		}

		Initialized = true;
	}

	~E1000Device()
	{
		if (!Initialized)
			return;

		switch (DeviceID)
		{
		case 0x100E:
		{
			// Clearing Enable bit in Receive Control Register
			uint32_t cmdret = ReadCMD(REG::RCTRL);
			WriteCMD(REG::RCTRL, cmdret & ~RCTL::EN);

			// Masking Interrupt Mask, Interrupt Throttling Rate & Interrupt Auto-Mask
			WriteCMD(REG::IMASK, 0x00000000);
			WriteCMD(REG::ITR, 0x00000000);
			WriteCMD(REG::IAM, 0x00000000);

			// Clearing SLU bit in Device Control Register
			cmdret = ReadCMD(REG::CTRL);
			WriteCMD(REG::CTRL, cmdret & ~ECTRL::SLU);

			// Clear the Interrupt Cause Read register by reading it
			ReadCMD(REG::ICR);

			// Powering down the device (?)
			WriteCMD(REG::CTRL, PCTRL::POWER_DOWN);
			/* TODO: Stop link; further testing required */
			break;
		}
		default:
		{
			KernelLog("Unimplemented E1000 device.");
			return;
		}
		}
	}
};

E1000Device *Drivers[4] = {nullptr};
dev_t NetID[4] = {(dev_t)-1};

#define OIR(x) OIR_##x
#define CREATE_OIR(x) \
	void OIR_##x(TrapFrame *f) { Drivers[x]->OnInterruptReceived(f); }

CREATE_OIR(0);
CREATE_OIR(1);
CREATE_OIR(2);
CREATE_OIR(3);

int __fs_Open(struct Inode *, int, mode_t) { return 0; }
int __fs_Close(struct Inode *) { return 0; }
ssize_t __fs_Read(struct Inode *, void *, size_t, off_t) { return 0; }

ssize_t __fs_Write(struct Inode *Node, const void *Buffer, size_t Size, off_t)
{
	return Drivers[NetID[Node->GetMinor()]]->write((uint8_t *)Buffer, Size);
}

int __fs_Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
{
	return Drivers[NetID[Node->GetMinor()]]->ioctl((NetIoctl)Request, Argp);
}

const struct InodeOperations NetOps = {
	.Lookup = nullptr,
	.Create = nullptr,
	.Remove = nullptr,
	.Rename = nullptr,
	.Read = __fs_Read,
	.Write = __fs_Write,
	.Truncate = nullptr,
	.Open = __fs_Open,
	.Close = __fs_Close,
	.Ioctl = __fs_Ioctl,
	.ReadDir = nullptr,
	.MkDir = nullptr,
	.RmDir = nullptr,
	.SymLink = nullptr,
	.ReadLink = nullptr,
	.Seek = nullptr,
	.Stat = nullptr,
};

PCIArray *Devices;
EXTERNC int cxx_Panic()
{
	PCIArray *ctx = Devices;
	short Count = 0;
	while (ctx != nullptr)
	{
		if (Drivers[Count] != nullptr)
			Drivers[Count]->Panic();
		Count++;
		ctx = (PCIArray *)ctx->Next;
	}

	return 0;
}

EXTERNC int cxx_Probe()
{
	uint16_t VendorIDs[] = {0x8086, /* Intel */
							PCI_END};
	uint16_t DeviceIDs[] = {0x100E, /* 82540EM */
							0x100F, /* 82545EM */
							0x10D3, /* 82574L */
							0x10EA, /* I217-LM */
							0x153A, /* 82577LM */
							PCI_END};
	Devices = GetPCIDevices(VendorIDs, DeviceIDs);
	if (Devices == nullptr)
	{
		KernelLog("No E1000 device found.");
		return -ENODEV;
	}
	return 0;
}

EXTERNC int cxx_Initialize()
{
	PCIArray *ctx = Devices;
	size_t Count = 0;
	while (ctx != nullptr)
	{
		if (Count > sizeof(Drivers) / sizeof(E1000Device *))
			break;

		InitializePCI(ctx->Device);

		Drivers[Count] = new E1000Device((PCIHeader0 *)ctx->Device->Header,
										 ctx->Device->Header->DeviceID);

		if (Drivers[Count]->IsInitialized())
		{
			dev_t ret = RegisterDevice(NETWORK_TYPE_ETHERNET, &NetOps);
			NetID[Count] = ret;
			Drivers[Count]->ID = ret;

			/* FIXME: bad code */
			switch (Count)
			{
			case 0:
				RegisterInterruptHandler(iLine(ctx->Device), (void *)OIR(0));
				break;
			case 1:
				RegisterInterruptHandler(iLine(ctx->Device), (void *)OIR(1));
				break;
			case 2:
				RegisterInterruptHandler(iLine(ctx->Device), (void *)OIR(2));
				break;
			case 3:
				RegisterInterruptHandler(iLine(ctx->Device), (void *)OIR(3));
				break;
			default:
				break;
			}

			Count++;
		}
		ctx = (PCIArray *)ctx->Next;
	}

	if (Count == 0)
	{
		KernelLog("No valid E1000 device found.");
		return -EINVAL;
	}

	return 0;
}

EXTERNC int cxx_Finalize()
{
	PCIArray *ctx = Devices;
	size_t Count = 0;
	while (ctx != nullptr)
	{
		if (Count++ > sizeof(Drivers) / sizeof(E1000Device *))
			break;

		delete Drivers[Count++];
		ctx->Device->Header->Command |= PCI_COMMAND_INTX_DISABLE;
		ctx = (PCIArray *)ctx->Next;
	}

	for (size_t i = 0; i < sizeof(NetID) / sizeof(dev_t); i++)
	{
		if (NetID[i] != (dev_t)-1)
			UnregisterDevice(NetID[i]);
	}

	return 0;
}
