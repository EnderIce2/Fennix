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

#define ATA_DEV_DRQ 0x08
#define ATA_DEV_BUSY 0x80
#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_CMD_IDENTIFY 0xEC

#define ATAPI_CMD_IDENTIFY_PACKET 0xA1

#define HBA_PORT_IPM_ACTIVE 0x1
#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PxIS_TFES (1 << 30)

#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_PM 0x96690101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_ATAPI 0xEB140101

#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000

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

/* https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ata/ns-ata-_identify_device_data */
struct __attribute__((packed)) ATA_IDENTIFY
{
	struct __attribute__((packed))
	{
		uint16_t Reserved1 : 1;
		uint16_t Retired3 : 1;
		uint16_t ResponseIncomplete : 1;
		uint16_t Retired2 : 3;
		uint16_t FixedDevice : 1;
		uint16_t RemovableMedia : 1;
		uint16_t Retired1 : 7;
		uint16_t DeviceType : 1;
	} GeneralConfiguration;
	uint16_t NumCylinders;
	uint16_t SpecificConfiguration;
	uint16_t NumHeads;
	uint16_t Retired1[2];
	uint16_t NumSectorsPerTrack;
	uint16_t VendorUnique1[3];
	uint8_t SerialNumber[20];
	uint16_t Retired2[2];
	uint16_t Obsolete1;
	uint8_t FirmwareRevision[8];
	uint8_t ModelNumber[40];
	uint8_t MaximumBlockTransfer;
	uint8_t VendorUnique2;
	struct __attribute__((packed))
	{
		uint16_t FeatureSupported : 1;
		uint16_t Reserved : 15;
	} TrustedComputing;
	struct __attribute__((packed))
	{
		uint8_t CurrentLongPhysicalSectorAlignment : 2;
		uint8_t ReservedByte49 : 6;
		uint8_t DmaSupported : 1;
		uint8_t LbaSupported : 1;
		uint8_t IordyDisable : 1;
		uint8_t IordySupported : 1;
		uint8_t Reserved1 : 1;
		uint8_t StandybyTimerSupport : 1;
		uint8_t Reserved2 : 2;
		uint16_t ReservedWord50;
	} Capabilities;
	uint16_t ObsoleteWords51[2];
	uint16_t TranslationFieldsValid : 3;
	uint16_t Reserved3 : 5;
	uint16_t FreeFallControlSensitivity : 8;
	uint16_t NumberOfCurrentCylinders;
	uint16_t NumberOfCurrentHeads;
	uint16_t CurrentSectorsPerTrack;
	uint32_t CurrentSectorCapacity;
	uint8_t CurrentMultiSectorSetting;
	uint8_t MultiSectorSettingValid : 1;
	uint8_t ReservedByte59 : 3;
	uint8_t SanitizeFeatureSupported : 1;
	uint8_t CryptoScrambleExtCommandSupported : 1;
	uint8_t OverwriteExtCommandSupported : 1;
	uint8_t BlockEraseExtCommandSupported : 1;
	uint32_t UserAddressableSectors;
	uint16_t ObsoleteWord62;
	uint16_t MultiWordDMASupport : 8;
	uint16_t MultiWordDMAActive : 8;
	uint16_t AdvancedPIOModes : 8;
	uint16_t ReservedByte64 : 8;
	uint16_t MinimumMWXferCycleTime;
	uint16_t RecommendedMWXferCycleTime;
	uint16_t MinimumPIOCycleTime;
	uint16_t MinimumPIOCycleTimeIORDY;
	struct __attribute__((packed))
	{
		uint16_t ZonedCapabilities : 2;
		uint16_t NonVolatileWriteCache : 1;
		uint16_t ExtendedUserAddressableSectorsSupported : 1;
		uint16_t DeviceEncryptsAllUserData : 1;
		uint16_t ReadZeroAfterTrimSupported : 1;
		uint16_t Optional28BitCommandsSupported : 1;
		uint16_t IEEE1667 : 1;
		uint16_t DownloadMicrocodeDmaSupported : 1;
		uint16_t SetMaxSetPasswordUnlockDmaSupported : 1;
		uint16_t WriteBufferDmaSupported : 1;
		uint16_t ReadBufferDmaSupported : 1;
		uint16_t DeviceConfigIdentifySetDmaSupported : 1;
		uint16_t LPSAERCSupported : 1;
		uint16_t DeterministicReadAfterTrimSupported : 1;
		uint16_t CFastSpecSupported : 1;
	} AdditionalSupported;
	uint16_t ReservedWords70[5];
	uint16_t QueueDepth : 5;
	uint16_t ReservedWord75 : 11;
	struct __attribute__((packed))
	{
		uint16_t Reserved0 : 1;
		uint16_t SataGen1 : 1;
		uint16_t SataGen2 : 1;
		uint16_t SataGen3 : 1;
		uint16_t Reserved1 : 4;
		uint16_t NCQ : 1;
		uint16_t HIPM : 1;
		uint16_t PhyEvents : 1;
		uint16_t NcqUnload : 1;
		uint16_t NcqPriority : 1;
		uint16_t HostAutoPS : 1;
		uint16_t DeviceAutoPS : 1;
		uint16_t ReadLogDMA : 1;
		uint16_t Reserved2 : 1;
		uint16_t CurrentSpeed : 3;
		uint16_t NcqStreaming : 1;
		uint16_t NcqQueueMgmt : 1;
		uint16_t NcqReceiveSend : 1;
		uint16_t DEVSLPtoReducedPwrState : 1;
		uint16_t Reserved3 : 8;
	} SerialAtaCapabilities;
	struct __attribute__((packed))
	{
		uint16_t Reserved0 : 1;
		uint16_t NonZeroOffsets : 1;
		uint16_t DmaSetupAutoActivate : 1;
		uint16_t DIPM : 1;
		uint16_t InOrderData : 1;
		uint16_t HardwareFeatureControl : 1;
		uint16_t SoftwareSettingsPreservation : 1;
		uint16_t NCQAutosense : 1;
		uint16_t DEVSLP : 1;
		uint16_t HybridInformation : 1;
		uint16_t Reserved1 : 6;
	} SerialAtaFeaturesSupported;
	struct __attribute__((packed))
	{
		uint16_t Reserved0 : 1;
		uint16_t NonZeroOffsets : 1;
		uint16_t DmaSetupAutoActivate : 1;
		uint16_t DIPM : 1;
		uint16_t InOrderData : 1;
		uint16_t HardwareFeatureControl : 1;
		uint16_t SoftwareSettingsPreservation : 1;
		uint16_t DeviceAutoPS : 1;
		uint16_t DEVSLP : 1;
		uint16_t HybridInformation : 1;
		uint16_t Reserved1 : 6;
	} SerialAtaFeaturesEnabled;
	uint16_t MajorRevision;
	uint16_t MinorRevision;
	struct __attribute__((packed))
	{
		uint16_t SmartCommands : 1;
		uint16_t SecurityMode : 1;
		uint16_t RemovableMediaFeature : 1;
		uint16_t PowerManagement : 1;
		uint16_t Reserved1 : 1;
		uint16_t WriteCache : 1;
		uint16_t LookAhead : 1;
		uint16_t ReleaseInterrupt : 1;
		uint16_t ServiceInterrupt : 1;
		uint16_t DeviceReset : 1;
		uint16_t HostProtectedArea : 1;
		uint16_t Obsolete1 : 1;
		uint16_t WriteBuffer : 1;
		uint16_t ReadBuffer : 1;
		uint16_t Nop : 1;
		uint16_t Obsolete2 : 1;
		uint16_t DownloadMicrocode : 1;
		uint16_t DmaQueued : 1;
		uint16_t Cfa : 1;
		uint16_t AdvancedPm : 1;
		uint16_t Msn : 1;
		uint16_t PowerUpInStandby : 1;
		uint16_t ManualPowerUp : 1;
		uint16_t Reserved2 : 1;
		uint16_t SetMax : 1;
		uint16_t Acoustics : 1;
		uint16_t BigLba : 1;
		uint16_t DeviceConfigOverlay : 1;
		uint16_t FlushCache : 1;
		uint16_t FlushCacheExt : 1;
		uint16_t WordValid83 : 2;
		uint16_t SmartErrorLog : 1;
		uint16_t SmartSelfTest : 1;
		uint16_t MediaSerialNumber : 1;
		uint16_t MediaCardPassThrough : 1;
		uint16_t StreamingFeature : 1;
		uint16_t GpLogging : 1;
		uint16_t WriteFua : 1;
		uint16_t WriteQueuedFua : 1;
		uint16_t WWN64Bit : 1;
		uint16_t URGReadStream : 1;
		uint16_t URGWriteStream : 1;
		uint16_t ReservedForTechReport : 2;
		uint16_t IdleWithUnloadFeature : 1;
		uint16_t WordValid : 2;
	} CommandSetSupport;
	struct __attribute__((packed))
	{
		uint16_t SmartCommands : 1;
		uint16_t SecurityMode : 1;
		uint16_t RemovableMediaFeature : 1;
		uint16_t PowerManagement : 1;
		uint16_t Reserved1 : 1;
		uint16_t WriteCache : 1;
		uint16_t LookAhead : 1;
		uint16_t ReleaseInterrupt : 1;
		uint16_t ServiceInterrupt : 1;
		uint16_t DeviceReset : 1;
		uint16_t HostProtectedArea : 1;
		uint16_t Obsolete1 : 1;
		uint16_t WriteBuffer : 1;
		uint16_t ReadBuffer : 1;
		uint16_t Nop : 1;
		uint16_t Obsolete2 : 1;
		uint16_t DownloadMicrocode : 1;
		uint16_t DmaQueued : 1;
		uint16_t Cfa : 1;
		uint16_t AdvancedPm : 1;
		uint16_t Msn : 1;
		uint16_t PowerUpInStandby : 1;
		uint16_t ManualPowerUp : 1;
		uint16_t Reserved2 : 1;
		uint16_t SetMax : 1;
		uint16_t Acoustics : 1;
		uint16_t BigLba : 1;
		uint16_t DeviceConfigOverlay : 1;
		uint16_t FlushCache : 1;
		uint16_t FlushCacheExt : 1;
		uint16_t Resrved3 : 1;
		uint16_t Words119_120Valid : 1;
		uint16_t SmartErrorLog : 1;
		uint16_t SmartSelfTest : 1;
		uint16_t MediaSerialNumber : 1;
		uint16_t MediaCardPassThrough : 1;
		uint16_t StreamingFeature : 1;
		uint16_t GpLogging : 1;
		uint16_t WriteFua : 1;
		uint16_t WriteQueuedFua : 1;
		uint16_t WWN64Bit : 1;
		uint16_t URGReadStream : 1;
		uint16_t URGWriteStream : 1;
		uint16_t ReservedForTechReport : 2;
		uint16_t IdleWithUnloadFeature : 1;
		uint16_t Reserved4 : 2;
	} CommandSetActive;
	uint16_t UltraDMASupport : 8;
	uint16_t UltraDMAActive : 8;
	struct __attribute__((packed))
	{
		uint16_t TimeRequired : 15;
		uint16_t ExtendedTimeReported : 1;
	} NormalSecurityEraseUnit;
	struct __attribute__((packed))
	{
		uint16_t TimeRequired : 15;
		uint16_t ExtendedTimeReported : 1;
	} EnhancedSecurityEraseUnit;
	uint16_t CurrentAPMLevel : 8;
	uint16_t ReservedWord91 : 8;
	uint16_t MasterPasswordID;
	uint16_t HardwareResetResult;
	uint16_t CurrentAcousticValue : 8;
	uint16_t RecommendedAcousticValue : 8;
	uint16_t StreamMinRequestSize;
	uint16_t StreamingTransferTimeDMA;
	uint16_t StreamingAccessLatencyDMAPIO;
	uint32_t StreamingPerfGranularity;
	uint32_t Max48BitLBA[2];
	uint16_t StreamingTransferTime;
	uint16_t DsmCap;
	struct __attribute__((packed))
	{
		uint16_t LogicalSectorsPerPhysicalSector : 4;
		uint16_t Reserved0 : 8;
		uint16_t LogicalSectorLongerThan256Words : 1;
		uint16_t MultipleLogicalSectorsPerPhysicalSector : 1;
		uint16_t Reserved1 : 2;
	} PhysicalLogicalSectorSize;
	uint16_t InterSeekDelay;
	uint16_t WorldWideName[4];
	uint16_t ReservedForWorldWideName128[4];
	uint16_t ReservedForTlcTechnicalReport;
	uint16_t WordsPerLogicalSector[2];
	struct __attribute__((packed))
	{
		uint16_t ReservedForDrqTechnicalReport : 1;
		uint16_t WriteReadVerify : 1;
		uint16_t WriteUncorrectableExt : 1;
		uint16_t ReadWriteLogDmaExt : 1;
		uint16_t DownloadMicrocodeMode3 : 1;
		uint16_t FreefallControl : 1;
		uint16_t SenseDataReporting : 1;
		uint16_t ExtendedPowerConditions : 1;
		uint16_t Reserved0 : 6;
		uint16_t WordValid : 2;
	} CommandSetSupportExt;
	struct __attribute__((packed))
	{
		uint16_t ReservedForDrqTechnicalReport : 1;
		uint16_t WriteReadVerify : 1;
		uint16_t WriteUncorrectableExt : 1;
		uint16_t ReadWriteLogDmaExt : 1;
		uint16_t DownloadMicrocodeMode3 : 1;
		uint16_t FreefallControl : 1;
		uint16_t SenseDataReporting : 1;
		uint16_t ExtendedPowerConditions : 1;
		uint16_t Reserved0 : 6;
		uint16_t Reserved1 : 2;
	} CommandSetActiveExt;
	uint16_t ReservedForExpandedSupportandActive[6];
	uint16_t MsnSupport : 2;
	uint16_t ReservedWord127 : 14;
	struct __attribute__((packed))
	{
		uint16_t SecuritySupported : 1;
		uint16_t SecurityEnabled : 1;
		uint16_t SecurityLocked : 1;
		uint16_t SecurityFrozen : 1;
		uint16_t SecurityCountExpired : 1;
		uint16_t EnhancedSecurityEraseSupported : 1;
		uint16_t Reserved0 : 2;
		uint16_t SecurityLevel : 1;
		uint16_t Reserved1 : 7;
	} SecurityStatus;
	uint16_t ReservedWord129[31];
	struct __attribute__((packed))
	{
		uint16_t MaximumCurrentInMA : 12;
		uint16_t CfaPowerMode1Disabled : 1;
		uint16_t CfaPowerMode1Required : 1;
		uint16_t Reserved0 : 1;
		uint16_t Word160Supported : 1;
	} CfaPowerMode1;
	uint16_t ReservedForCfaWord161[7];
	uint16_t NominalFormFactor : 4;
	uint16_t ReservedWord168 : 12;
	struct __attribute__((packed))
	{
		uint16_t SupportsTrim : 1;
		uint16_t Reserved0 : 15;
	} DataSetManagementFeature;
	uint16_t AdditionalProductID[4];
	uint16_t ReservedForCfaWord174[2];
	uint16_t CurrentMediaSerialNumber[30];
	struct __attribute__((packed))
	{
		uint16_t Supported : 1;
		uint16_t Reserved0 : 1;
		uint16_t WriteSameSuported : 1;
		uint16_t ErrorRecoveryControlSupported : 1;
		uint16_t FeatureControlSuported : 1;
		uint16_t DataTablesSuported : 1;
		uint16_t Reserved1 : 6;
		uint16_t VendorSpecific : 4;
	} SCTCommandTransport;
	uint16_t ReservedWord207[2];
	struct __attribute__((packed))
	{
		uint16_t AlignmentOfLogicalWithinPhysical : 14;
		uint16_t Word209Supported : 1;
		uint16_t Reserved0 : 1;
	} BlockAlignment;
	uint16_t WriteReadVerifySectorCountMode3Only[2];
	uint16_t WriteReadVerifySectorCountMode2Only[2];
	struct __attribute__((packed))
	{
		uint16_t NVCachePowerModeEnabled : 1;
		uint16_t Reserved0 : 3;
		uint16_t NVCacheFeatureSetEnabled : 1;
		uint16_t Reserved1 : 3;
		uint16_t NVCachePowerModeVersion : 4;
		uint16_t NVCacheFeatureSetVersion : 4;
	} NVCacheCapabilities;
	uint16_t NVCacheSizeLSW;
	uint16_t NVCacheSizeMSW;
	uint16_t NominalMediaRotationRate;
	uint16_t ReservedWord218;
	struct __attribute__((packed))
	{
		uint8_t NVCacheEstimatedTimeToSpinUpInSeconds;
		uint8_t Reserved;
	} NVCacheOptions;
	uint16_t WriteReadVerifySectorCountMode : 8;
	uint16_t ReservedWord220 : 8;
	uint16_t ReservedWord221;
	struct __attribute__((packed))
	{
		uint16_t MajorVersion : 12;
		uint16_t TransportType : 4;
	} TransportMajorVersion;
	uint16_t TransportMinorVersion;
	uint16_t ReservedWord224[6];
	uint32_t ExtendedNumberOfUserAddressableSectors[2];
	uint16_t MinBlocksPerDownloadMicrocodeMode03;
	uint16_t MaxBlocksPerDownloadMicrocodeMode03;
	uint16_t ReservedWord236[19];
	uint16_t Signature : 8;
	uint16_t CheckSum : 8;
};

class Port
{
public:
	PortType AHCIPortType;
	HBAPort *HBAPortPtr;
	uint8_t *Buffer;
	uint8_t PortNumber;
	ATA_IDENTIFY *IdentifyData;

	Port(PortType Type, HBAPort *PortPtr, uint8_t PortNumber)
	{
		this->AHCIPortType = Type;
		this->HBAPortPtr = PortPtr;
		this->Buffer = static_cast<uint8_t *>(AllocateMemory(1));
		MemorySet(this->Buffer, 0, PAGE_SIZE);
		this->IdentifyData = static_cast<ATA_IDENTIFY *>(AllocateMemory(1));
		MemorySet(this->IdentifyData, 0, PAGE_SIZE);
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

		Identify();

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

	void Identify()
	{
		MemorySet(this->IdentifyData, 0, sizeof(ATA_IDENTIFY));
		HBACommandHeader *CommandHeader = reinterpret_cast<HBACommandHeader *>(HBAPortPtr->CommandListBase);
		CommandHeader->CommandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
		CommandHeader->Write = 0;
		CommandHeader->PRDTLength = 1;

		HBACommandTable *CommandTable = reinterpret_cast<HBACommandTable *>(CommandHeader->CommandTableBaseAddress);
		MemorySet(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

		CommandTable->PRDTEntry[0].DataBaseAddress = (uint32_t)(uint64_t)this->IdentifyData;
		CommandTable->PRDTEntry[0].DataBaseAddressUpper = (uint32_t)((uint64_t)this->IdentifyData >> 32);
		CommandTable->PRDTEntry[0].ByteCount = 511;
		CommandTable->PRDTEntry[0].InterruptOnCompletion = 1;

		FIS_REG_H2D *CommandFIS = (FIS_REG_H2D *)(&CommandTable->CommandFIS);
		CommandFIS->FISType = FIS_TYPE_REG_H2D;
		CommandFIS->CommandControl = 1;
		CommandFIS->Command = this->AHCIPortType == PortType::SATAPI ? ATAPI_CMD_IDENTIFY_PACKET : ATA_CMD_IDENTIFY;

		HBAPortPtr->CommandIssue = 1;

		while (HBAPortPtr->CommandIssue)
			Yield();

		if (HBAPortPtr->InterruptStatus & HBA_PxIS_TFES)
		{
			Log("Error reading IDENTIFY command.");
			return;
		}

		auto swap = [](uint16_t *data, size_t size)
		{
			for (size_t i = 0; i < size; i++)
				data[i] = (data[i] >> 8) | (data[i] << 8);
		};

		char *Model = (char *)this->IdentifyData->ModelNumber;
		char ModelSwap[41];
		for (size_t i = 0; i < 40; i += 2)
		{
			ModelSwap[i] = Model[i + 1];
			ModelSwap[i + 1] = Model[i];
		}
		ModelSwap[40] = 0;

		Log("Port %d \"%s\" identified", PortNumber,
			ModelSwap);
		Log("Port %d is %s (%d rotation rate)", PortNumber,
			IdentifyData->NominalMediaRotationRate == 1 ? "SSD" : "HDD",
			IdentifyData->NominalMediaRotationRate);

		if (IdentifyData->Signature != 0xA5)
		{
			Log("Port %d has no validity signature.", PortNumber);
			return;
		}

		uint8_t *ptr = (uint8_t *)IdentifyData;
		uint8_t sum = 0;
		for (size_t i = 0; i < 512; i++)
			sum += ptr[i];
		if (sum != 0)
			Log("Port %d has invalid checksum.", PortNumber);
		else
			Log("Port %d has valid checksum.", PortNumber);
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
