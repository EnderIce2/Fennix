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

#include <errno.h>
#include <block.h>
#include <regs.h>
#include <base.h>
#include <pci.h>

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_CMD_READ_DMA_EX 0x25
#define HBA_PxIS_TFES (1 << 30)

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

#define HBA_PxCMD_CR 0x8000
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FR 0x4000

enum PortType
{
	None = 0,
	SATA = 1,
	SEMB = 2,
	PM = 3,
	SATAPI = 4,
};

enum FIS_TYPE
{
	FIS_TYPE_REG_H2D = 0x27,
	FIS_TYPE_REG_D2H = 0x34,
	FIS_TYPE_DMA_ACT = 0x39,
	FIS_TYPE_DMA_SETUP = 0x41,
	FIS_TYPE_DATA = 0x46,
	FIS_TYPE_BIST = 0x58,
	FIS_TYPE_PIO_SETUP = 0x5F,
	FIS_TYPE_DEV_BITS = 0xA1,
};

struct HBAPort
{
	uint32_t CommandListBase;
	uint32_t CommandListBaseUpper;
	uint32_t FISBaseAddress;
	uint32_t FISBaseAddressUpper;
	uint32_t InterruptStatus;
	uint32_t InterruptEnable;
	uint32_t CommandStatus;
	uint32_t Reserved0;
	uint32_t TaskFileData;
	uint32_t Signature;
	uint32_t SataStatus;
	uint32_t SataControl;
	uint32_t SataError;
	uint32_t SataActive;
	uint32_t CommandIssue;
	uint32_t SataNotification;
	uint32_t FISSwitchControl;
	uint32_t Reserved1[11];
	uint32_t Vendor[4];
};

struct HBAMemory
{
	uint32_t HostCapability;
	uint32_t GlobalHostControl;
	uint32_t InterruptStatus;
	uint32_t PortsImplemented;
	uint32_t Version;
	uint32_t CCCControl;
	uint32_t CCCPorts;
	uint32_t EnclosureManagementLocation;
	uint32_t EnclosureManagementControl;
	uint32_t HostCapabilitiesExtended;
	uint32_t BIOSHandoffControlStatus;
	uint8_t Reserved0[0x74];
	uint8_t Vendor[0x60];
	HBAPort Ports[1];
};

struct HBACommandHeader
{
	uint8_t CommandFISLength : 5;
	uint8_t ATAPI : 1;
	uint8_t Write : 1;
	uint8_t Preferable : 1;
	uint8_t Reset : 1;
	uint8_t BIST : 1;
	uint8_t ClearBusy : 1;
	uint8_t Reserved0 : 1;
	uint8_t PortMultiplier : 4;
	uint16_t PRDTLength;
	uint32_t PRDBCount;
	uint32_t CommandTableBaseAddress;
	uint32_t CommandTableBaseAddressUpper;
	uint32_t Reserved1[4];
};

struct HBAPRDTEntry
{
	uint32_t DataBaseAddress;
	uint32_t DataBaseAddressUpper;
	uint32_t Reserved0;
	uint32_t ByteCount : 22;
	uint32_t Reserved1 : 9;
	uint32_t InterruptOnCompletion : 1;
};

struct HBACommandTable
{
	uint8_t CommandFIS[64];
	uint8_t ATAPICommand[16];
	uint8_t Reserved[48];
	HBAPRDTEntry PRDTEntry[];
};

struct FIS_REG_H2D
{
	uint8_t FISType;
	uint8_t PortMultiplier : 4;
	uint8_t Reserved0 : 3;
	uint8_t CommandControl : 1;
	uint8_t Command;
	uint8_t FeatureLow;
	uint8_t LBA0;
	uint8_t LBA1;
	uint8_t LBA2;
	uint8_t DeviceRegister;
	uint8_t LBA3;
	uint8_t LBA4;
	uint8_t LBA5;
	uint8_t FeatureHigh;
	uint8_t CountLow;
	uint8_t CountHigh;
	uint8_t ISOCommandCompletion;
	uint8_t Control;
	uint8_t Reserved1[4];
};

class Port
{
public:
	PortType AHCIPortType;
	HBAPort *HBAPortPtr;
	uint8_t *Buffer;
	uint8_t PortNumber;

	Port(PortType Type, HBAPort *PortPtr, uint8_t PortNumber)
	{
		this->AHCIPortType = Type;
		this->HBAPortPtr = PortPtr;
		this->Buffer = static_cast<uint8_t *>(AllocateMemory(1));
		MemorySet(this->Buffer, 0, PAGE_SIZE);
		this->PortNumber = PortNumber;
	}

	~Port()
	{
		FreeMemory(this->Buffer, 1);
	}

	void StartCMD()
	{
		while (HBAPortPtr->CommandStatus & HBA_PxCMD_CR)
			Yield();
		HBAPortPtr->CommandStatus |= HBA_PxCMD_FRE;
		HBAPortPtr->CommandStatus |= HBA_PxCMD_ST;
	}

	void StopCMD()
	{
		HBAPortPtr->CommandStatus &= ~HBA_PxCMD_ST;
		HBAPortPtr->CommandStatus &= ~HBA_PxCMD_FRE;
		while (true)
		{
			if (HBAPortPtr->CommandStatus & HBA_PxCMD_FR)
				continue;
			if (HBAPortPtr->CommandStatus & HBA_PxCMD_CR)
				continue;
			break;
		}
	}

	void Configure()
	{
		this->StopCMD();
		void *CmdBase = AllocateMemory(1);
		HBAPortPtr->CommandListBase = (uint32_t)(uint64_t)CmdBase;
		HBAPortPtr->CommandListBaseUpper = (uint32_t)((uint64_t)CmdBase >> 32);
		MemorySet(reinterpret_cast<void *>(HBAPortPtr->CommandListBase), 0, 1024);

		void *FISBase = AllocateMemory(1);
		HBAPortPtr->FISBaseAddress = (uint32_t)(uint64_t)FISBase;
		HBAPortPtr->FISBaseAddressUpper = (uint32_t)((uint64_t)FISBase >> 32);
		MemorySet(FISBase, 0, 256);

		HBACommandHeader *CommandHeader = (HBACommandHeader *)((uint64_t)HBAPortPtr->CommandListBase + ((uint64_t)HBAPortPtr->CommandListBaseUpper << 32));
		for (int i = 0; i < 32; i++)
		{
			CommandHeader[i].PRDTLength = 8;
			void *CommandTableAddress = AllocateMemory(1);
			uint64_t Address = (uint64_t)CommandTableAddress + (i << 8);
			CommandHeader[i].CommandTableBaseAddress = (uint32_t)(uint64_t)Address;
			CommandHeader[i].CommandTableBaseAddressUpper = (uint32_t)((uint64_t)Address >> 32);
			MemorySet(CommandTableAddress, 0, 256);
		}
		this->StartCMD();

		Log("Port %d \"%x %x %x %x\" configured", PortNumber,
			HBAPortPtr->Vendor[0], HBAPortPtr->Vendor[1],
			HBAPortPtr->Vendor[2], HBAPortPtr->Vendor[3]);
	}

	bool ReadWrite(uint64_t Sector, uint32_t SectorCount, uint8_t *Buffer, bool Write)
	{
		if (this->AHCIPortType == PortType::SATAPI &&
			Write == true)
		{
			Log("SATAPI port does not support write.");
			return false;
		}

		uint32_t SectorL = (uint32_t)Sector;
		uint32_t SectorH = (uint32_t)(Sector >> 32);

		HBAPortPtr->InterruptStatus = 0xFFFFFFFF; /* Clear pending interrupt bits */

		HBACommandHeader *CommandHeader = reinterpret_cast<HBACommandHeader *>(HBAPortPtr->CommandListBase);
		CommandHeader->CommandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
		if (Write)
			CommandHeader->Write = 1;
		else
			CommandHeader->Write = 0;
		CommandHeader->PRDTLength = 1;

		HBACommandTable *CommandTable = reinterpret_cast<HBACommandTable *>(CommandHeader->CommandTableBaseAddress);
		MemorySet(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

		CommandTable->PRDTEntry[0].DataBaseAddress = (uint32_t)(uint64_t)Buffer;
		CommandTable->PRDTEntry[0].DataBaseAddressUpper = (uint32_t)((uint64_t)Buffer >> 32);

#pragma GCC diagnostic push
/* conversion from 'uint32_t' {aka 'unsigned int'} to 'unsigned int:22' may change value */
#pragma GCC diagnostic ignored "-Wconversion"
		CommandTable->PRDTEntry[0].ByteCount = (SectorCount << 9) - 1; /* 512 bytes per sector */
#pragma GCC diagnostic pop

		CommandTable->PRDTEntry[0].InterruptOnCompletion = 1;

		FIS_REG_H2D *CommandFIS = (FIS_REG_H2D *)(&CommandTable->CommandFIS);

		CommandFIS->FISType = FIS_TYPE_REG_H2D;
		CommandFIS->CommandControl = 1;
		if (Write)
			CommandFIS->Command = ATA_CMD_WRITE_DMA_EX;
		else
			CommandFIS->Command = ATA_CMD_READ_DMA_EX;

		CommandFIS->LBA0 = (uint8_t)SectorL;
		CommandFIS->LBA1 = (uint8_t)(SectorL >> 8);
		CommandFIS->LBA2 = (uint8_t)(SectorL >> 16);
		CommandFIS->LBA3 = (uint8_t)SectorH;
		CommandFIS->LBA4 = (uint8_t)(SectorH >> 8);
		CommandFIS->LBA5 = (uint8_t)(SectorH >> 16);

		CommandFIS->DeviceRegister = 1 << 6; // LBA mode
		CommandFIS->CountLow = SectorCount & 0xFF;
		CommandFIS->CountHigh = (SectorCount >> 8) & 0xFF;

		uint64_t Spin = 0;

		while ((HBAPortPtr->TaskFileData & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && Spin < 1000000)
			Spin++;

		if (Spin == 1000000)
		{
			Log("Port not responding.");
			return false;
		}

		HBAPortPtr->CommandIssue = 1;

		Spin = 0;
		int TryCount = 0;

		while (true)
		{
			if (Spin > 100000000)
			{
				Log("Port %d not responding. (%d)",
					this->PortNumber, TryCount);

				Spin = 0;
				TryCount++;
				if (TryCount > 10)
					return false;
			}

			if (HBAPortPtr->CommandIssue == 0)
				break;
			Spin++;

			if (HBAPortPtr->InterruptStatus & HBA_PxIS_TFES)
			{
				Log("Error reading/writing (%d).", Write);
				return false;
			}
		}

		return true;
	}
};

Port *Ports[64];
int PortCount = 0;

const char *PortTypeName[] = {"None",
							  "SATA",
							  "SEMB",
							  "PM",
							  "SATAPI"};

PortType CheckPortType(HBAPort *Port)
{
	uint32_t SataStatus = Port->SataStatus;
	uint8_t InterfacePowerManagement = (SataStatus >> 8) & 0b111;
	uint8_t DeviceDetection = SataStatus & 0b111;

	if (DeviceDetection != HBA_PORT_DEV_PRESENT)
		return PortType::None;
	if (InterfacePowerManagement != HBA_PORT_IPM_ACTIVE)
		return PortType::None;

	switch (Port->Signature)
	{
	case SATA_SIG_ATAPI:
		return PortType::SATAPI;
	case SATA_SIG_ATA:
		return PortType::SATA;
	case SATA_SIG_PM:
		return PortType::PM;
	case SATA_SIG_SEMB:
		return PortType::SEMB;
	default:
		return PortType::None;
	}
}

size_t drvRead(dev_t, dev_t min,
			   uint8_t *Buffer, size_t Size, off_t Offset)
{
	bool ok = Ports[min]->ReadWrite(Offset / 512,
									uint32_t(Size / 512),
									Buffer,
									false);
	return ok ? Size : 0;
}

size_t drvWrite(dev_t, dev_t min,
				uint8_t *Buffer, size_t Size, off_t Offset)
{
	bool ok = Ports[min]->ReadWrite(Offset / 512,
									uint32_t(Size / 512),
									Buffer,
									true);
	return ok ? Size : 0;
}

void OnInterruptReceived(TrapFrame *)
{
}

EXTERNC int cxx_Panic()
{
	for (int i = 0; i < PortCount; i++)
		Ports[i]->StopCMD();
	return 0;
}

PCIArray *Devices;
EXTERNC int cxx_Probe()
{
	uint16_t VendorIDs[] = {0x8086, /* Intel */
							0x15AD, /* VMware */
							PCI_END};
	uint16_t DeviceIDs[] = {0x2922, /* ICH9 */
							0x2829, /* ICH8 */
							0x07E0, /* SATA AHCI (VMware) */
							PCI_END};
	Devices = FindPCIDevices(VendorIDs, DeviceIDs);
	if (Devices == nullptr)
	{
		Log("No AHCI device found.");
		return -ENODEV;
	}
	return 0;
}

EXTERNC int cxx_Initialize()
{
	PCIArray *ctx = Devices;

	/**
	 * We loop through all the devices and initialize them
	 */
	while (ctx != nullptr)
	{
		/* We don't use the interrupt handler now... maybe we will in the future */
		// RegisterInterruptHandler(iLine(ctx->Device), (void *)OnInterruptReceived);

		InitializePCI(ctx->Device);
		HBAMemory *HBA = (HBAMemory *)(uintptr_t)GetBAR(5, ctx->Device);

		uint32_t PortsImplemented = HBA->PortsImplemented;
		Log("AHCI ports implemented: %x", PortsImplemented);
		for (int i = 0; i < 32; i++)
		{
			if (PortCount > 64)
			{
				Log("There are more than 64 AHCI ports implemented");
				break;
			}

			if (PortsImplemented & (1 << i))
			{
				Log("Port %d implemented", i);
				PortType portType = CheckPortType(&HBA->Ports[i]);
				if (portType == PortType::SATA || portType == PortType::SATAPI)
				{
					KPrint("%s drive found at port %d", PortTypeName[portType], i);
					Ports[PortCount] = new Port(portType, &HBA->Ports[i], PortCount);
					dev_t ret = RegisterBlockDevice(ddt_SATA,
													nullptr, nullptr,
													drvRead, drvWrite,
													nullptr);
					if (ret != (dev_t)PortCount)
					{
						KPrint("Failed to register block device %d", ret);
						return -EBADSLT;
					}

					PortCount++;
					Ports[PortCount] = nullptr;
				}
				else
				{
					if (portType != PortType::None)
					{
						KPrint("Unsupported drive type %s found at port %d",
							   PortTypeName[portType], i);
					}
				}
			}
		}

		ctx = (PCIArray *)ctx->Next;
	}

	Log("Initializing AHCI ports");
	for (int i = 0; i < PortCount; i++)
		Ports[i]->Configure();

	return PortCount > 0 ? 0 : -ENODEV;
}

EXTERNC int cxx_Finalize()
{
	for (int i = 0; i < PortCount; i++)
	{
		Ports[i]->StopCMD();
		delete Ports[i];
	}

	PortCount = 0;

	do
	{
		UnregisterBlockDevice(PortCount, ddt_SATA);
		PortCount--;
	} while (PortCount >= 0);

	PCIArray *ctx = Devices;

	while (ctx != nullptr)
	{
		ctx->Device->Header->Command |= PCI_COMMAND_INTX_DISABLE;
		ctx = (PCIArray *)ctx->Next;
	}

	// std::list<PCI::PCIDevice> Devices = PCIManager->FindPCIDevice(VendorIDs, DeviceIDs);
	// foreach (auto dev in Devices)
	// 	Interrupts::RemoveHandler(OnInterruptReceived, iLine(dev));
	return 0;
}
