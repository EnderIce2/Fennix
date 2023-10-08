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

#include "ahci.hpp"

#include <debug.h>
#include <pci.hpp>

#include "../../mapi.hpp"
#include "../mod.hpp"

using namespace PCI;

namespace AdvancedHostControllerInterface
{
	KernelAPI KAPI;

	HBAMemory *AHBA;
	Port *Ports[32];
	uint8_t PortCount = 0;

	PCIDeviceHeader *PCIBaseAddress;

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

	Port::Port(PortType Type, HBAPort *PortPtr, uint8_t PortNumber)
	{
		this->AHCIPortType = Type;
		this->HBAPortPtr = PortPtr;
		this->Buffer = static_cast<uint8_t *>(KAPI.Memory.RequestPage(1));
		memset(this->Buffer, 0, size_t(KAPI.Memory.PageSize));
		this->PortNumber = PortNumber;
	}

	Port::~Port()
	{
		KAPI.Memory.FreePage(this->Buffer, 1);
	}

	void Port::StartCMD()
	{
		while (HBAPortPtr->CommandStatus & HBA_PxCMD_CR)
			;
		HBAPortPtr->CommandStatus |= HBA_PxCMD_FRE;
		HBAPortPtr->CommandStatus |= HBA_PxCMD_ST;
	}

	void Port::StopCMD()
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

	void Port::Configure()
	{
		StopCMD();
		void *NewBase = KAPI.Memory.RequestPage(1);
		HBAPortPtr->CommandListBase = (uint32_t)(uint64_t)NewBase;
		HBAPortPtr->CommandListBaseUpper = (uint32_t)((uint64_t)NewBase >> 32);
		memset(reinterpret_cast<void *>(HBAPortPtr->CommandListBase), 0, 1024);

		void *FISBase = KAPI.Memory.RequestPage(1);
		HBAPortPtr->FISBaseAddress = (uint32_t)(uint64_t)FISBase;
		HBAPortPtr->FISBaseAddressUpper = (uint32_t)((uint64_t)FISBase >> 32);
		memset(FISBase, 0, 256);

		HBACommandHeader *CommandHeader = (HBACommandHeader *)((uint64_t)HBAPortPtr->CommandListBase + ((uint64_t)HBAPortPtr->CommandListBaseUpper << 32));
		for (int i = 0; i < 32; i++)
		{
			CommandHeader[i].PRDTLength = 8;
			void *CommandTableAddress = KAPI.Memory.RequestPage(1);
			uint64_t Address = (uint64_t)CommandTableAddress + (i << 8);
			CommandHeader[i].CommandTableBaseAddress = (uint32_t)(uint64_t)Address;
			CommandHeader[i].CommandTableBaseAddressUpper = (uint32_t)((uint64_t)Address >> 32);
			memset(CommandTableAddress, 0, 256);
		}
		StartCMD();
	}

	bool Port::ReadWrite(uint64_t Sector, uint32_t SectorCount, uint8_t *Buffer, bool Write)
	{
		if (this->PortNumber == PortType::SATAPI && Write)
		{
			error("SATAPI port does not support write.");
			return false;
		}

		uint32_t SectorL = (uint32_t)Sector;
		uint32_t SectorH = (uint32_t)(Sector >> 32);

		HBAPortPtr->InterruptStatus = (uint32_t)-1; // Clear pending interrupt bits

		HBACommandHeader *CommandHeader = reinterpret_cast<HBACommandHeader *>(HBAPortPtr->CommandListBase);
		CommandHeader->CommandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
		if (Write)
			CommandHeader->Write = 1;
		else
			CommandHeader->Write = 0;
		CommandHeader->PRDTLength = 1;

		HBACommandTable *CommandTable = reinterpret_cast<HBACommandTable *>(CommandHeader->CommandTableBaseAddress);
		memset(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

		CommandTable->PRDTEntry[0].DataBaseAddress = (uint32_t)(uint64_t)Buffer;
		CommandTable->PRDTEntry[0].DataBaseAddressUpper = (uint32_t)((uint64_t)Buffer >> 32);

#pragma GCC diagnostic push
/* conversion from ‘uint32_t’ {aka ‘unsigned int’} to ‘unsigned int:22’ may change value */
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
			error("Port not responding.");
			return false;
		}

		HBAPortPtr->CommandIssue = 1;

		Spin = 0;
		int TryCount = 0;

		while (true)
		{
			if (Spin > 100000000)
			{
				error("Port %d not responding. (%d)", this->PortNumber, TryCount);
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
				error("Error reading/writing (%d).", Write);
				return false;
			}
		}

		return true;
	}

	int DriverEntry(void *Data)
	{
		if (!Data)
			return INVALID_KERNEL_API;
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;

		return OK;
	}

	int CallbackHandler(KernelCallback *Data)
	{
		switch (Data->Reason)
		{
		case AcknowledgeReason:
		{
			debug("Kernel acknowledged the driver.");
			break;
		}
		case ConfigurationReason:
		{
			debug("Module received configuration data.");
			PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
			AHBA = reinterpret_cast<HBAMemory *>(((PCIHeader0 *)PCIBaseAddress)->BAR5);
			KAPI.Memory.Map((void *)AHBA, (void *)AHBA, (1 << 1));

			uint32_t PortsImplemented = AHBA->PortsImplemented;
			for (int i = 0; i < 32; i++)
			{
				if (PortsImplemented & (1 << i))
				{
					PortType portType = CheckPortType(&AHBA->Ports[i]);
					if (portType == PortType::SATA || portType == PortType::SATAPI)
					{
						trace("%s drive found at port %d", PortTypeName[portType], i);
						Ports[PortCount] = new Port(portType, &AHBA->Ports[i], PortCount);
						PortCount++;
					}
					else
					{
						if (portType != PortType::None)
							warn("Unsupported drive type %s found at port %d",
								 PortTypeName[portType], i);
					}
				}
			}

			for (int i = 0; i < PortCount; i++)
				Ports[i]->Configure();
			break;
		}
		case QueryReason:
		{
			Data->DiskCallback.Fetch.Ports = PortCount;
			Data->DiskCallback.Fetch.BytesPerSector = 512;
			break;
		}
		case StopReason:
		{
			// TODO: Stop the driver.
			debug("Module stopped.");
			break;
		}
		case SendReason:
		case ReceiveReason:
		{
			Ports[Data->DiskCallback.RW.Port]->ReadWrite(Data->DiskCallback.RW.Sector,
														 (uint32_t)Data->DiskCallback.RW.SectorCount,
														 Data->DiskCallback.RW.Buffer,
														 Data->DiskCallback.RW.Write);
			break;
		}
		default:
		{
			warn("Unknown reason.");
			break;
		}
		}
		return OK;
	}

	int InterruptCallback(CPURegisters *)
	{
		/* There is no interrupt handler for AHCI. */
		return OK;
	}
}
