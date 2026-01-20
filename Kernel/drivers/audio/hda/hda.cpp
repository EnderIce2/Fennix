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
#include "hda.hpp"

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
namespace Driver::HighDefinitionAudio
{
	dev_t DriverID;

	class HDADevice
	{
	private:
		PCI::PCIHeader0 *Header;
		bool Initialized = false;

		ControllerRegisters *CTL;

		uint32_t *CORB;
		uint64_t *RIRB;

	public:
		bool IsInitialized() { return Initialized; }

		size_t write(uint8_t *, size_t Size)
		{
			return Size;
		}

		int ioctl(AudioIoctl, void *)
		{
			return 0;
		}

		int OnInterruptReceived(CPU::TrapFrame *)
		{
			return EOK;
		}

		void Panic()
		{
		}

		HDADevice(PCI::PCIHeader0 *_Header)
			: Header(_Header),
			  CORB((uint32_t *)(uintptr_t)DriverManager->AllocateMemory(DriverID, 1)),
			  RIRB((uint64_t *)DriverManager->AllocateMemory(DriverID, 1))
		{
			CTL = (ControllerRegisters *)(uintptr_t)Header->BAR[0];
			fixme("Unimplemented HDA driver");
			return;
			Initialized = true;
		}

		~HDADevice()
		{
			if (!Initialized)
				return;
		}
	};

	std::unordered_map<dev_t, HDADevice *> Drivers;

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		return Drivers[Node->GetMinor()]->ioctl((AudioIoctl)Request, Argp);
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		return Drivers[Node->GetMinor()]->write((uint8_t *)Buffer, Size);
	}

	const struct InodeOperations ops = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = nullptr,
		.Write = Write,
		.Truncate = nullptr,
		.Open = nullptr,
		.Close = nullptr,
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
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)dev.Header;
			uint8_t type = hdr0->BAR[0] & 1;
			if (type == 1)
			{
				debug("Device %x:%x.%d BAR0 is I/O.",
					  hdr0->Header.VendorID,
					  hdr0->Header.DeviceID,
					  hdr0->Header.ProgIF);
				continue;
			}

			PCIManager->InitializeDevice(dev, KernelPageTable);

			HDADevice *driver = new HDADevice((PCI::PCIHeader0 *)dev.Header);

			if (driver->IsInitialized())
			{
				dev_t ret = v0::RegisterDevice(DriverID, AUDIO_TYPE_PCM, &ops);
				Drivers[ret] = driver;
			}
		}

		if (Drivers.empty())
		{
			debug("No valid HDA device found.");
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
				0x8086, /* Intel */
				0x15AD, /* VMware */
			},
			{
				0x9D71, /* Sunrise Point-LP HD Audio */
				0x2668, /* ICH6 */
				0x293E, /* ICH9 */
			});

		if (Devices.empty())
		{
			trace("No HDA device found.");
			return -ENODEV;
		}

		for (auto &&dev : Devices)
		{
			PCI::PCIHeader0 *PCIBaseAddress = (PCI::PCIHeader0 *)dev.Header;
			uint32_t PCIBAR0 = PCIBaseAddress->BAR[0];
			uint8_t Type = PCIBAR0 & 1;
			if (Type == 1)
			{
				debug("Device %x:%x.%d BAR0 is I/O.",
					  PCIBaseAddress->Header.VendorID,
					  PCIBaseAddress->Header.DeviceID,
					  PCIBaseAddress->Header.ProgIF);
				continue;
			}
		}

		return 0;
	}

	REGISTER_BUILTIN_DRIVER(hda,
							"Intel High Definition Audio Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
