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

#include <errno.h>
#include <audio.h>
#include <regs.h>
#include <base.h>
#include <pci.h>
#include <io.h>

#include "hda.hpp"

class HDADevice
{
private:
	PCIHeader0 *Header;
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

	void OnInterruptReceived(TrapFrame *)
	{
	}

	void Panic()
	{
	}

	HDADevice(PCIHeader0 *_Header)
		: Header(_Header),
		  CORB((uint32_t *)(uintptr_t)AllocateMemory(1)),
		  RIRB((uint64_t *)AllocateMemory(1))
	{
		CTL = (ControllerRegisters *)(uintptr_t)Header->BAR0;
		Log("Unimplemented HDA driver");
		return;
		Initialized = true;
	}

	~HDADevice()
	{
		if (!Initialized)
			return;
	}
};

HDADevice *Drivers[4] = {nullptr};
dev_t AudioID[4] = {0};

#define OIR(x) OIR_##x
#define CREATE_OIR(x) \
	void OIR_##x(TrapFrame *f) { Drivers[x]->OnInterruptReceived(f); }

CREATE_OIR(0);
CREATE_OIR(1);
CREATE_OIR(2);
CREATE_OIR(3);

int drvOpen(dev_t, dev_t, int, mode_t) { return 0; }
int drvClose(dev_t, dev_t) { return 0; }
size_t drvRead(dev_t, dev_t, uint8_t *, size_t, off_t) { return 0; }

size_t drvWrite(dev_t, dev_t min, uint8_t *Buffer, size_t Size, off_t)
{
	return Drivers[AudioID[min]]->write(Buffer, Size);
}

int drvIoctl(dev_t, dev_t min, unsigned long Request, void *Argp)
{
	return Drivers[AudioID[min]]->ioctl((AudioIoctl)Request, Argp);
}

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
							0x15AD, /* VMware */
							PCI_END};
	uint16_t DeviceIDs[] = {0x9D71 /* Sunrise Point-LP HD Audio */,
							0x2668 /* ICH6 */,
							0x293E /* ICH9 */,
							PCI_END};
	Devices = FindPCIDevices(VendorIDs, DeviceIDs);
	if (Devices == nullptr)
	{
		Log("No HDA device found.");
		return -ENODEV;
	}

	PCIArray *ctx = Devices;
	bool Found = false;
	size_t Count = 0;
	while (ctx != nullptr)
	{
		if (Count++ > sizeof(Drivers) / sizeof(HDADevice *))
			break;

		PCIHeader0 *PCIBaseAddress = (PCIHeader0 *)ctx->Device->Header;
		uint32_t PCIBAR0 = PCIBaseAddress->BAR0;
		uint8_t Type = PCIBAR0 & 1;
		if (Type == 1)
		{
			Log("Device %x:%x.%d BAR0 is I/O.",
				PCIBaseAddress->Header.VendorID,
				PCIBaseAddress->Header.DeviceID,
				PCIBaseAddress->Header.ProgIF);
			continue;
		}

		Found = true;
		ctx = (PCIArray *)ctx->Next;
	}

	if (!Found)
	{
		Log("No valid HDA device found.");
		return -EINVAL;
	}
	return 0;
}

EXTERNC int cxx_Initialize()
{
	PCIArray *ctx = Devices;
	size_t Count = 0;
	while (ctx != nullptr)
	{
		if (Count > sizeof(Drivers) / sizeof(HDADevice *))
			break;

		PCIHeader0 *PCIBaseAddress = (PCIHeader0 *)ctx->Device->Header;
		uint32_t PCIBAR0 = PCIBaseAddress->BAR0;
		uint8_t Type = PCIBAR0 & 1;
		if (Type == 1)
		{
			Log("Device %x:%x.%d BAR0 is I/O.",
				PCIBaseAddress->Header.VendorID,
				PCIBaseAddress->Header.DeviceID,
				PCIBaseAddress->Header.ProgIF);
			continue;
		}

		InitializePCI(ctx->Device);

		Drivers[Count] = new HDADevice((PCIHeader0 *)ctx->Device->Header);

		if (Drivers[Count]->IsInitialized())
		{
			dev_t ret = RegisterAudioDevice(ddt_Audio,
											drvOpen, drvClose,
											drvRead, drvWrite,
											drvIoctl);
			AudioID[Count] = ret;
			Count++;
		}
		ctx = (PCIArray *)ctx->Next;
	}

	if (Count == 0)
	{
		Log("No valid HDA device found.");
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
		if (Count++ > sizeof(Drivers) / sizeof(HDADevice *))
			break;

		PCIHeader0 *PCIBaseAddress = (PCIHeader0 *)ctx->Device->Header;
		uint32_t PCIBAR0 = PCIBaseAddress->BAR0;
		uint8_t Type = PCIBAR0 & 1;
		if (Type == 1)
		{
			Log("Device %x:%x.%d BAR0 is I/O.",
				PCIBaseAddress->Header.VendorID,
				PCIBaseAddress->Header.DeviceID,
				PCIBaseAddress->Header.ProgIF);
			continue;
		}

		delete Drivers[Count++];
		ctx = (PCIArray *)ctx->Next;
	}

	return 0;
}
