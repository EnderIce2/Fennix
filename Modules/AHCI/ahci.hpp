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

#ifndef __FENNIX_KERNEL_AHCI_H__
#define __FENNIX_KERNEL_AHCI_H__

#include <types.h>
#include "../../DAPI.hpp"

namespace AdvancedHostControllerInterface
{
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

	struct BARData
	{
		uint8_t Type;
		uint16_t IOBase;
		uint64_t MemoryBase;
	};

	class Port
	{
	public:
		PortType AHCIPortType;
		HBAPort *HBAPortPtr;
		uint8_t *Buffer;
		uint8_t PortNumber;

		Port(PortType Type, HBAPort *PortPtr, uint8_t PortNumber);
		~Port();
		void StartCMD();
		void StopCMD();
		void Configure();
		bool ReadWrite(uint64_t Sector, uint32_t SectorCount, uint8_t *Buffer, bool Write);
	};

	int DriverEntry(void *);
	int CallbackHandler(KernelCallback *);
	int InterruptCallback(CPURegisters *);
}

#endif // !__FENNIX_KERNEL_AHCI_H__
