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
#include <interface/block.h>
#include <cpu.hpp>
#include <pci.hpp>

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
EXTERNC void KPrint(const char *Format, ...);
namespace Driver::AHCI
{
	dev_t DriverID;

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
	struct __packed ATA_IDENTIFY
	{
		struct __packed
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
		struct __packed
		{
			uint16_t FeatureSupported : 1;
			uint16_t Reserved : 15;
		} TrustedComputing;
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
		{
			uint16_t TimeRequired : 15;
			uint16_t ExtendedTimeReported : 1;
		} NormalSecurityEraseUnit;
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
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
		struct __packed
		{
			uint16_t SupportsTrim : 1;
			uint16_t Reserved0 : 15;
		} DataSetManagementFeature;
		uint16_t AdditionalProductID[4];
		uint16_t ReservedForCfaWord174[2];
		uint16_t CurrentMediaSerialNumber[30];
		struct __packed
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
		struct __packed
		{
			uint16_t AlignmentOfLogicalWithinPhysical : 14;
			uint16_t Word209Supported : 1;
			uint16_t Reserved0 : 1;
		} BlockAlignment;
		uint16_t WriteReadVerifySectorCountMode3Only[2];
		uint16_t WriteReadVerifySectorCountMode2Only[2];
		struct __packed
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
		struct __packed
		{
			uint8_t NVCacheEstimatedTimeToSpinUpInSeconds;
			uint8_t Reserved;
		} NVCacheOptions;
		uint16_t WriteReadVerifySectorCountMode : 8;
		uint16_t ReservedWord220 : 8;
		uint16_t ReservedWord221;
		struct __packed
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
		uint32_t BlockSize;
		uint32_t BlockCount;
		size_t Size;
		ATA_IDENTIFY *IdentifyData;

		Port(PortType Type, HBAPort *PortPtr, uint8_t PortNumber)
		{
			this->AHCIPortType = Type;
			this->HBAPortPtr = PortPtr;
			this->Buffer = static_cast<uint8_t *>(v0::AllocateMemory(DriverID, 1));
			memset(this->Buffer, 0, PAGE_SIZE);
			this->IdentifyData = static_cast<ATA_IDENTIFY *>(v0::AllocateMemory(DriverID, 1));
			memset(this->IdentifyData, 0, PAGE_SIZE);
			this->PortNumber = PortNumber;
		}

		~Port()
		{
			v0::FreeMemory(DriverID, this->Buffer, 1);
		}

		void StartCMD()
		{
			while (HBAPortPtr->CommandStatus & HBA_PxCMD_CR)
				v0::Yield(DriverID);
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
			debug("Configuring port %d", PortNumber);
			this->StopCMD();
			void *CmdBase = v0::AllocateMemory(DriverID, 1);
			HBAPortPtr->CommandListBase = (uint32_t)(uint64_t)CmdBase;
			HBAPortPtr->CommandListBaseUpper = (uint32_t)((uint64_t)CmdBase >> 32);
			memset(reinterpret_cast<void *>(HBAPortPtr->CommandListBase), 0, 1024);

			void *FISBase = v0::AllocateMemory(DriverID, 1);
			HBAPortPtr->FISBaseAddress = (uint32_t)(uint64_t)FISBase;
			HBAPortPtr->FISBaseAddressUpper = (uint32_t)((uint64_t)FISBase >> 32);
			memset(FISBase, 0, 256);

			HBACommandHeader *CommandHeader = (HBACommandHeader *)((uint64_t)HBAPortPtr->CommandListBase + ((uint64_t)HBAPortPtr->CommandListBaseUpper << 32));
			for (int i = 0; i < 32; i++)
			{
				CommandHeader[i].PRDTLength = 8;
				void *CommandTableAddress = v0::AllocateMemory(DriverID, 1);
				uint64_t Address = (uint64_t)CommandTableAddress + (i << 8);
				CommandHeader[i].CommandTableBaseAddress = (uint32_t)(uint64_t)Address;
				CommandHeader[i].CommandTableBaseAddressUpper = (uint32_t)((uint64_t)Address >> 32);
				memset(CommandTableAddress, 0, 256);
			}
			this->StartCMD();

			Identify();

			if (IdentifyData->CommandSetSupport.BigLba)
			{
				if ((IdentifyData->CommandSetActive.Words119_120Valid & 0x1) != 0)
				{
					uint32_t wordsPerLogicalSector = (IdentifyData->WordsPerLogicalSector[1] << 16) | IdentifyData->WordsPerLogicalSector[0];
					if (wordsPerLogicalSector != 0)
						this->BlockSize = wordsPerLogicalSector * 2;
				}
			}
			this->BlockSize = 512;

			this->BlockCount = this->IdentifyData->UserAddressableSectors;
			this->Size = this->BlockCount * this->BlockSize;

			trace("Port %d \"%x %x %x %x\" configured", PortNumber,
				  HBAPortPtr->Vendor[0], HBAPortPtr->Vendor[1],
				  HBAPortPtr->Vendor[2], HBAPortPtr->Vendor[3]);
		}

		int ReadWrite(uint64_t Sector, uint32_t SectorCount, void *Buffer, bool Write)
		{
			if (this->AHCIPortType == PortType::SATAPI && Write == true)
			{
				trace("SATAPI port does not support write.");
				return ENOTSUP;
			}

			debug("%s op on port %d, sector %d, count %d", Write ? "Write" : "Read", this->PortNumber, Sector, SectorCount);

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
			memset(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

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

			CommandFIS->DeviceRegister = 1 << 6; /* LBA mode */
			CommandFIS->CountLow = SectorCount & 0xFF;
			CommandFIS->CountHigh = (SectorCount >> 8) & 0xFF;

			uint64_t spinLock = 0;
			while ((HBAPortPtr->TaskFileData & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spinLock < 1000000)
				spinLock++;

			if (spinLock == 1000000)
			{
				trace("Port not responding.");
				return ETIMEDOUT;
			}

			HBAPortPtr->CommandIssue = 1;

			spinLock = 0;
			int retries = 0;
			while (true)
			{
				if (spinLock > 100000000)
				{
					trace("Port %d not responding. (%d)",
						  this->PortNumber, retries);

					spinLock = 0;
					retries++;
					if (retries > 10)
						return ETIMEDOUT;
				}

				if (HBAPortPtr->CommandIssue == 0)
					break;
				spinLock++;

				if (HBAPortPtr->InterruptStatus & HBA_PxIS_TFES)
				{
					trace("Error reading/writing (%d).", Write);
					return EIO;
				}
			}

			return 0;
		}

		void Identify()
		{
			memset(this->IdentifyData, 0, sizeof(ATA_IDENTIFY));
			HBACommandHeader *CommandHeader = reinterpret_cast<HBACommandHeader *>(HBAPortPtr->CommandListBase);
			CommandHeader->CommandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
			CommandHeader->Write = 0;
			CommandHeader->PRDTLength = 1;

			HBACommandTable *CommandTable = reinterpret_cast<HBACommandTable *>(CommandHeader->CommandTableBaseAddress);
			memset(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

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
				v0::Yield(DriverID);

			if (HBAPortPtr->InterruptStatus & HBA_PxIS_TFES)
			{
				trace("Error reading IDENTIFY command.");
				return;
			}

			if (IdentifyData->Signature != 0xA5)
				trace("Port %d has no validity signature.", PortNumber);
			else
			{
				uint8_t *ptr = (uint8_t *)IdentifyData;
				uint8_t sum = 0;
				for (size_t i = 0; i < sizeof(ATA_IDENTIFY); i++)
					sum += ptr[i];
				if (sum != 0)
				{
					trace("Port %d has invalid checksum.", PortNumber);
					return;
				}
				else
					trace("Port %d has valid checksum.", PortNumber);
			}

			char *Model = (char *)this->IdentifyData->ModelNumber;
			char ModelSwap[41];
			for (size_t i = 0; i < 40; i += 2)
			{
				ModelSwap[i] = Model[i + 1];
				ModelSwap[i + 1] = Model[i];
			}
			ModelSwap[40] = 0;

			trace("Port %d \"%s\" identified", PortNumber,
				  ModelSwap);
			trace("Port %d is %s (%d rotation rate)", PortNumber,
				  IdentifyData->NominalMediaRotationRate == 1 ? "SSD" : "HDD",
				  IdentifyData->NominalMediaRotationRate);
		}
	};

	std::unordered_map<dev_t, Port *> PortDevices;

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

	ssize_t Read(struct Inode *Node, void *Buffer, size_t Size, off_t Offset)
	{
		Port *port = static_cast<Port *>(Node->PrivateData);
		if ((Offset % port->BlockSize) != 0 || (Size % port->BlockSize) != 0)
		{
			trace("Read offset or size not aligned to block size (BlockSize=%u)", port->BlockSize);
			return -EINVAL;
		}

		uint64_t sector = Offset / port->BlockSize;
		uint32_t sectorCount = uint32_t(Size / port->BlockSize);
		if (sectorCount == 0)
		{
			trace("Attempt to read 0 sectors");
			return 0;
		}

		bool status = port->ReadWrite(sector, sectorCount, Buffer, false);
		if (status != 0)
		{
			trace("Error '%s' reading from port %d", strerror(status), port->PortNumber);
			return status;
		}
		return Size;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t Offset)
	{
		Port *port = static_cast<Port *>(Node->PrivateData);
		if ((Offset % port->BlockSize) != 0 || (Size % port->BlockSize) != 0)
		{
			trace("Read offset or size not aligned to block size (BlockSize=%u)", port->BlockSize);
			return -EINVAL;
		}

		uint64_t sector = Offset / port->BlockSize;
		uint32_t sectorCount = uint32_t(Size / port->BlockSize);
		if (sectorCount == 0)
		{
			trace("Attempt to write 0 sectors");
			return 0;
		}

		bool status = port->ReadWrite(sector, sectorCount, (void *)Buffer, true);
		if (status != 0)
		{
			trace("Error '%s' writing to port %d", strerror(status), port->PortNumber);
			return status;
		}
		return Size;
	}

	int Open(struct Inode *, int, mode_t) { return 0; }
	int Close(struct Inode *) { return 0; }

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
		.Ioctl = nullptr,
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
		/* We loop through all the devices and initialize them */
		for (auto &&dev : Devices)
		{
			PCIManager->InitializeDevice(dev, KernelPageTable);
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)dev.Header;
			HBAMemory *hba = (HBAMemory *)(uintptr_t)hdr0->BAR5;
			uint32_t portsImplemented = hba->PortsImplemented;
			trace("AHCI ports implemented: %x", portsImplemented);

			for (int i = 0; i < 32; i++)
			{
				if (!(portsImplemented & (1 << i)))
					continue;

				trace("Port %d implemented", i);

				PortType portType = CheckPortType(&hba->Ports[i]);
				switch (portType)
				{
				case PortType::SATA:
				case PortType::SATAPI:
				{
					KPrint("%s drive found at port %d", PortTypeName[portType], i);
					Port *port = new Port(portType, &hba->Ports[i], i);
					port->Configure();

					BlockDevice *dev = new BlockDevice;
					dev->Name = "ahci";
					dev->BlockSize = port->BlockSize;
					dev->BlockCount = port->BlockCount;
					dev->Size = port->Size;
					dev->Ops = &ops;
					dev->PrivateData = port;
					dev_t ret = v0::RegisterBlockDevice(DriverID, dev);
					PortDevices[ret] = port;
					debug("Port %d \"%s\" registered as %d", i, port->IdentifyData->ModelNumber, ret);
					break;
				}
				case PortType::SEMB:
				case PortType::PM:
				{
					KPrint("Unimplemented drive type %s found at port %d",
						   PortTypeName[portType], i);
					break;
				}
				default:
				{
					trace("Unsupported drive type %s found at port %d",
						  PortTypeName[portType], i);
					break;
				}
				}
			}
		}

		if (PortDevices.empty())
		{
			info("No valid AHCI device found.");
			return -ENODEV;
		}

		/* We don't use the interrupt handler now... maybe we will in the future */
		// RegisterInterruptHandler(iLine(ctx->Device), (void *)OnInterruptReceived);

		return 0;
	}

	int Final()
	{
		for (auto &&p : PortDevices)
		{
			p.second->StopCMD();
			v0::UnregisterBlockDevice(DriverID, p.first);
			delete p.second;
		}

		/* Making sure that PortDevices is empty */
		PortDevices.clear();
		// ctx->Device->Header->Command |= PCI::PCI_COMMAND_INTX_DISABLE;

		// std::list<PCI::PCIDevice> Devices = PCIManager->FindPCIDevice(VendorIDs, DeviceIDs);
		// for (auto dev : Devices)
		// 	Interrupts::RemoveHandler(OnInterruptReceived, iLine(dev));
		return 0;
	}

	int Panic()
	{
		for (auto &&p : PortDevices)
			p.second->StopCMD();
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
				0x2922, /* ICH9 */
				0x2829, /* ICH8 */
				0x07E0, /* SATA AHCI (VMware) */
			});

		if (Devices.empty())
		{
			trace("No AHCI device found.");
			return -ENODEV;
		}
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(ahci,
							"Advanced Host Controller Interface Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}
