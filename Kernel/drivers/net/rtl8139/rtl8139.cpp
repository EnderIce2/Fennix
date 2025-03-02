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

#include <driver.hpp>
#include <cpu.hpp>
#include <pci.hpp>
#include <net/net.hpp>
#include <io.h>

#include "rtl8139.hpp"

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
namespace Driver::RTL8139
{
	dev_t DriverID;

	class RTL8139Device
	{
	private:
		PCI::PCIHeader0 *Header;
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

		void OnInterruptReceived(CPU::TrapFrame *)
		{
			/* Acknowledge interrupt */
			uint16_t status = inw(RegISR);
			debug("%#lx", status);

			/* Read status */
			if (status & RecOK)
			{
				/* Get the current packet */
				uint16_t *data = (uint16_t *)(RXBuffer + CurrentPacket);
				uint16_t dataSz = *(data + 1);
				data += 2;

				// ReportNetworkPacket(ID, data, dataSz);
				/* FIXME: Implement */
				fixme("Received packet");
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

		RTL8139Device(PCI::PCIHeader0 *_Header)
			: Header(_Header)
		{
			uint32_t PCIBAR0 = Header->BAR0;
			uint32_t PCIBAR1 = Header->BAR1;
			BAR.Type = PCIBAR0 & 1;
			BAR.IOBase = (uint16_t)(PCIBAR0 & (~3));
			BAR.MemoryBase = PCIBAR1 & (~15);

			RXBuffer = (uint8_t *)v0::AllocateMemory(DriverID, TO_PAGES(BufferSize));

			/* Power on */
			outb(RegCONFIG1, 0x0);

			/* Software Reset */
			outb(RegCMD, 0x10);
			while (inb(RegCMD) & 0x10)
				v0::Yield(DriverID);

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

			((PCI::PCIDeviceHeader *)Header)->Command |= PCI::PCI_COMMAND_INTX_DISABLE;
			/* FIXME: Shutdown code */
		}
	};

	std::unordered_map<dev_t, RTL8139Device *> Drivers;

	int Open(struct Inode *, int, mode_t)
	{
		return 0;
	}

	int Close(struct Inode *)
	{
		return 0;
	}

	ssize_t Read(struct Inode *, void *, size_t, off_t)
	{
		return 0;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t)
	{
		return Drivers[Node->GetMinor()]->write((uint8_t *)Buffer, Size);
	}

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		return Drivers[Node->GetMinor()]->ioctl((NetIoctl)Request, Argp);
	}

	const struct InodeOperations ops = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = Read,
		.Write = Write,
		.Truncate = nullptr,
		.Open = Open,
		.Close = Close,
		.Ioctl = Ioctl,
		.ReadDir = nullptr,
		.MkDir = nullptr,
		.RmDir = nullptr,
		.SymLink = nullptr,
		.ReadLink = nullptr,
		.Seek = nullptr,
		.Stat = nullptr,
	};

	std::list<PCI::PCIDevice> Devices;
	int Entry()
	{
		for (auto &&dev : Devices)
		{
			PCIManager->InitializeDevice(dev, KernelPageTable);
			RTL8139Device *rtl8139 = new RTL8139Device((PCI::PCIHeader0 *)dev.Header);
			if (rtl8139->IsInitialized())
			{
				dev_t ret = v0::RegisterDevice(DriverID, NETWORK_TYPE_ETHERNET, &ops);
				Drivers[ret] = rtl8139;
			}
		}

		if (Drivers.empty())
		{
			trace("No valid RTL8139 device found.");
			return -EINVAL;
		}

		return 0;
	}

	int Final()
	{
		for (auto &&dev : Drivers)
		{
			dev_t ret = dev.first;
			v0::UnregisterDevice(DriverID, ret);
			delete dev.second;
		}
		return 0;
	}

	int Panic()
	{
		for (auto &&i : Drivers)
			i.second->Panic();
		return 0;
	}

	int Probe()
	{
		Devices = PCIManager->FindPCIDevice(
			{
				0x10EC, /* Realtek */
			},
			{
				0x8139, /* RTL8139 */
			});

		if (Devices.empty())
		{
			trace("No RTL8139 device found.");
			return -ENODEV;
		}
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(rtl8139,
							"Realtek RTL8139 Network Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
