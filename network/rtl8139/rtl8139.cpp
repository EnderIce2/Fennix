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

#include "rtl8139.hpp"

class RTL8139Device
{
private:
	PCIHeader0 *Header;
	bool Initialized = false;

	struct BARData
	{
		uint8_t Type;
		uint16_t IOBase;
		uint64_t MemoryBase;
	} BAR;

	const int BaseBufferSize = 8192;
	const int WRAPBytes = 1500;
	const int AdditionalBytes = 16;
	const int BufferSize = BaseBufferSize +
						   WRAPBytes +
						   AdditionalBytes;

	uint8_t *RXBuffer = nullptr;
	int TXCurrent = 0;
	uint16_t CurrentPacket = 0;

	uint8_t TSAD[4] = {0x20, 0x24, 0x28, 0x2C};
	uint8_t TSD[4] = {0x10, 0x14, 0x18, 0x1C};

public:
	dev_t ID;

	bool IsInitialized() { return Initialized; }

	size_t write(uint8_t *Buffer, size_t Size)
	{
		outl(TSAD[TXCurrent], (uint32_t)(reinterpret_cast<uint64_t>(Buffer)));
		outl(TSD[TXCurrent++], (uint32_t)Size);
		if (TXCurrent > 3)
			TXCurrent = 0;
		return Size;
	}

	MediaAccessControl GetMAC()
	{
		return MediaAccessControl();
	}

	int ioctl(NetIoctl req, void *)
	{
		switch (req)
		{
		case IOCTL_NET_GET_MAC:
		{
			return -ENOSYS;
		}
		default:
			return -EINVAL;
		}
		return 0;
	}

	void OnInterruptReceived(TrapFrame *)
	{
		/* Acknowledge interrupt */
		uint16_t status = inw(RegISR);
		DebugLog("%#lx", status);

		/* Read status */
		if (status & RecOK)
		{
			/* Get the current packet */
			uint16_t *data = (uint16_t *)(RXBuffer + CurrentPacket);
			uint16_t dataSz = *(data + 1);
			data += 2;

			// ReportNetworkPacket(ID, data, dataSz);
			/* FIXME: Implement */
			KernelLog("FIXME: Received packet");
			(void)data;
			(void)dataSz;

			/* Update CAPR */
#define RX_READ_PTR_MASK (~0x3)
			CurrentPacket = (uint16_t)((CurrentPacket + dataSz + 4 + 3) & RX_READ_PTR_MASK);
			if (CurrentPacket > BufferSize)
				CurrentPacket -= uint16_t(BufferSize);
			outw(RegCAPR, CurrentPacket - 0x10);
		}

		/* Clear interrupt */
		outw(RegISR, (RecOK | RecBad | SendOK | SendBad));
	}

	void Panic()
	{
	}

	RTL8139Device(PCIHeader0 *_Header)
		: Header(_Header)
	{
		uint32_t PCIBAR0 = Header->BAR0;
		uint32_t PCIBAR1 = Header->BAR1;
		BAR.Type = PCIBAR0 & 1;
		BAR.IOBase = (uint16_t)(PCIBAR0 & (~3));
		BAR.MemoryBase = PCIBAR1 & (~15);

		RXBuffer = (uint8_t *)AllocateMemory(TO_PAGES(BufferSize));

		/* Power on */
		outb(RegCONFIG1, 0x0);

		/* Software Reset */
		outb(RegCMD, 0x10);
		while (inb(RegCMD) & 0x10)
			Yield();

		/* Initialize receive buffer */
		outl(RegRBSTART, (uint32_t)(reinterpret_cast<uintptr_t>(RXBuffer)));

		/* Configure interrupt mask register */
		outw(RegIMR, (RecOK | RecBad | SendOK | SendBad));
		outl(regRCR, (RcAB | RcAM | RcAPM | RcAAP) | RcWRAP);

		/* Enable receive and transmit */
		outb(RegCMD, 0xC); /* 0xC = RE and TE bit */

		uint32_t MAC1 = inl(RegMAC);
		uint16_t MAC2 = inw(RegMAR);
		MediaAccessControl mac = {
			mac.Address[0] = (uint8_t)MAC1,
			mac.Address[1] = (uint8_t)(MAC1 >> 8),
			mac.Address[2] = (uint8_t)(MAC1 >> 16),
			mac.Address[3] = (uint8_t)(MAC1 >> 24),
			mac.Address[4] = (uint8_t)MAC2,
			mac.Address[5] = (uint8_t)(MAC2 >> 8)};

		Initialized = true;
	}

	~RTL8139Device()
	{
		if (!Initialized)
			return;

		/* FIXME: Shutdown code */
	}
};

RTL8139Device *Drivers[4] = {nullptr};
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
	uint16_t VendorIDs[] = {0x10EC, /* Realtek */
							PCI_END};
	uint16_t DeviceIDs[] = {0x8139, /* RTL8139 */
							PCI_END};
	Devices = GetPCIDevices(VendorIDs, DeviceIDs);
	if (Devices == nullptr)
	{
		KernelLog("No RTL8139 device found.");
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
		if (Count > sizeof(Drivers) / sizeof(RTL8139Device *))
			break;

		InitializePCI(ctx->Device);

		Drivers[Count] = new RTL8139Device((PCIHeader0 *)ctx->Device->Header);

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
		KernelLog("No valid RTL8139 device found.");
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
		if (Count++ > sizeof(Drivers) / sizeof(RTL8139Device *))
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
