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
#include <fs.h>

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
		KernelLog("Unimplemented HDA driver");
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
dev_t AudioID[4] = {(dev_t)-1};

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
	return Drivers[AudioID[Node->GetMinor()]]->write((uint8_t *)Buffer, Size);
}

int __fs_Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
{
	return Drivers[AudioID[Node->GetMinor()]]->ioctl((AudioIoctl)Request, Argp);
}

const struct InodeOperations AudioOps = {
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
							0x15AD, /* VMware */
							PCI_END};
	uint16_t DeviceIDs[] = {0x9D71 /* Sunrise Point-LP HD Audio */,
							0x2668 /* ICH6 */,
							0x293E /* ICH9 */,
							PCI_END};
	Devices = GetPCIDevices(VendorIDs, DeviceIDs);
	if (Devices == nullptr)
	{
		KernelLog("No HDA device found.");
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
			KernelLog("Device %x:%x.%d BAR0 is I/O.",
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
		KernelLog("No valid HDA device found.");
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
			KernelLog("Device %x:%x.%d BAR0 is I/O.",
					  PCIBaseAddress->Header.VendorID,
					  PCIBaseAddress->Header.DeviceID,
					  PCIBaseAddress->Header.ProgIF);
			continue;
		}

		InitializePCI(ctx->Device);

		Drivers[Count] = new HDADevice((PCIHeader0 *)ctx->Device->Header);

		if (Drivers[Count]->IsInitialized())
		{
			dev_t ret = RegisterDevice(AUDIO_TYPE_PCM, &AudioOps);
			AudioID[Count] = ret;
			Count++;
		}
		ctx = (PCIArray *)ctx->Next;
	}

	if (Count == 0)
	{
		KernelLog("No valid HDA device found.");
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
			KernelLog("Device %x:%x.%d BAR0 is I/O.",
					  PCIBaseAddress->Header.VendorID,
					  PCIBaseAddress->Header.DeviceID,
					  PCIBaseAddress->Header.ProgIF);
			continue;
		}

		delete Drivers[Count++];
		ctx = (PCIArray *)ctx->Next;
	}

	for (size_t i = 0; i < sizeof(AudioID) / sizeof(dev_t); i++)
	{
		if (AudioID[i] != (dev_t)-1)
			UnregisterDevice(AudioID[i]);
	}

	return 0;
}
