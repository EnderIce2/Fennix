#include <netools.h>
#include <pci.h>
#include <io.h>

#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(void *Data);
int CallbackHandler(KernelCallback *Data);
int InterruptCallback(CPURegisters *Registers);

HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "AHCI",
        .Type = FexDriverType_Storage,
        .Callback = CallbackHandler,
        .InterruptCallback = InterruptCallback,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x8086, 0x15AD},
                .DeviceID = {0x2922, 0x2829, 0x07E0},
                .Class = 0x1,
                .SubClass = 0x6,
                .ProgIF = 0x1,
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

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
    uint64_t IOBase;
    uint64_t MemoryBase;
};

typedef enum
{
    _URC_NO_REASON = 0,
    _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
    _URC_FATAL_PHASE2_ERROR = 2,
    _URC_FATAL_PHASE1_ERROR = 3,
    _URC_NORMAL_STOP = 4,
    _URC_END_OF_STACK = 5,
    _URC_HANDLER_FOUND = 6,
    _URC_INSTALL_CONTEXT = 7,
    _URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

struct _Unwind_Context;
typedef unsigned _Unwind_Exception_Class __attribute__((__mode__(__DI__)));
typedef unsigned _Unwind_Word __attribute__((__mode__(__unwind_word__)));
typedef void (*_Unwind_Exception_Cleanup_Fn)(_Unwind_Reason_Code, struct _Unwind_Exception *);
typedef int _Unwind_Action;

struct _Unwind_Exception
{
    _Unwind_Exception_Class exception_class;
    _Unwind_Exception_Cleanup_Fn exception_cleanup;
#if !defined(__USING_SJLJ_EXCEPTIONS__) && defined(__SEH__)
    _Unwind_Word private_[6];
#else
    _Unwind_Word private_1;
    _Unwind_Word private_2;
#endif
} __attribute__((__aligned__));

extern "C" _Unwind_Reason_Code __gxx_personality_v0(int, _Unwind_Action, _Unwind_Exception_Class, _Unwind_Exception *, _Unwind_Context *)
{
    print("__gxx_personality_v0");
    return _URC_NO_REASON;
}

extern "C" void _Unwind_Resume(_Unwind_Exception *) { print("_Unwind_Resume"); }

void *operator new(size_t Size) { return KAPI->Memory.RequestPage(Size / KAPI->Memory.PageSize + 1); }
void operator delete(void *Ptr) { KAPI->Memory.FreePage(Ptr, 1); } // Potential memory leak
void operator delete(void *Ptr, size_t Size) { KAPI->Memory.FreePage(Ptr, Size / KAPI->Memory.PageSize + 1); }

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
        this->Buffer = static_cast<uint8_t *>(KAPI->Memory.RequestPage(1));
        KAPI->Util.memset(this->Buffer, 0, KAPI->Memory.PageSize);
        this->PortNumber = PortNumber;
    }

    ~Port() { KAPI->Memory.FreePage(Buffer, 1); }

    void StartCMD()
    {
        while (HBAPortPtr->CommandStatus & HBA_PxCMD_CR)
            ;
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
        StopCMD();
        void *NewBase = KAPI->Memory.RequestPage(1);
        HBAPortPtr->CommandListBase = (uint32_t)(uint64_t)NewBase;
        HBAPortPtr->CommandListBaseUpper = (uint32_t)((uint64_t)NewBase >> 32);
        KAPI->Util.memset(reinterpret_cast<void *>(HBAPortPtr->CommandListBase), 0, 1024);

        void *FISBase = KAPI->Memory.RequestPage(1);
        HBAPortPtr->FISBaseAddress = (uint32_t)(uint64_t)FISBase;
        HBAPortPtr->FISBaseAddressUpper = (uint32_t)((uint64_t)FISBase >> 32);
        KAPI->Util.memset(FISBase, 0, 256);

        HBACommandHeader *CommandHeader = (HBACommandHeader *)((uint64_t)HBAPortPtr->CommandListBase + ((uint64_t)HBAPortPtr->CommandListBaseUpper << 32));
        for (int i = 0; i < 32; i++)
        {
            CommandHeader[i].PRDTLength = 8;
            void *CommandTableAddress = KAPI->Memory.RequestPage(1);
            uint64_t Address = (uint64_t)CommandTableAddress + (i << 8);
            CommandHeader[i].CommandTableBaseAddress = (uint32_t)(uint64_t)Address;
            CommandHeader[i].CommandTableBaseAddressUpper = (uint32_t)((uint64_t)Address >> 32);
            KAPI->Util.memset(CommandTableAddress, 0, 256);
        }
        StartCMD();
    }

    bool ReadWrite(uint64_t Sector, uint32_t SectorCount, uint8_t *Buffer, bool Write)
    {
        if (this->PortNumber == PortType::SATAPI && Write)
        {
            // err("SATAPI port does not support write.");
            print("SATAPI port does not support write.");
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
        KAPI->Util.memset(CommandTable, 0, sizeof(HBACommandTable) + (CommandHeader->PRDTLength - 1) * sizeof(HBAPRDTEntry));

        CommandTable->PRDTEntry[0].DataBaseAddress = (uint32_t)(uint64_t)Buffer;
        CommandTable->PRDTEntry[0].DataBaseAddressUpper = (uint32_t)((uint64_t)Buffer >> 32);
        CommandTable->PRDTEntry[0].ByteCount = (SectorCount << 9) - 1; // 512 bytes per sector
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
            // err("Port not responding.");
            print("Port not responding.");
            return false;
        }

        HBAPortPtr->CommandIssue = 1;

        Spin = 0;
        int TryCount = 0;

        while (true)
        {
            if (Spin > 100000000)
            {
                // err("Port %d not responding. (%d)", this->PortNumber, TryCount);
                print("Port not responding.");
                Spin = 0;
                TryCount++;
                if (TryCount > 10)
                    return false;
            }
            if ((HBAPortPtr->CommandIssue == 0))
                break;
            Spin++;
            if (HBAPortPtr->InterruptStatus & HBA_PxIS_TFES)
            {
                // err("Error reading/writing (%d).", Write);
                print("Error reading/writing.");
                return false;
            }
        }

        return true;
    }
};

HBAMemory *ABAR;
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

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    if (KAPI->Info.KernelDebug) /* FIXME: TCG doesn't like this driver. */
        return NOT_AVAILABLE;

    return OK;
}

int CallbackHandler(KernelCallback *Data)
{
    switch (Data->Reason)
    {
    case AcknowledgeReason:
    {
        print("Kernel acknowledged the driver.");
        break;
    }
    case ConfigurationReason:
    {
        print("Driver received configuration data.");
        PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
        ABAR = reinterpret_cast<HBAMemory *>(((PCIHeader0 *)PCIBaseAddress)->BAR5);
        KAPI->Memory.Map((void *)ABAR, (void *)ABAR, (1 << 1));

        uint32_t PortsImplemented = ABAR->PortsImplemented;
        for (int i = 0; i < 32; i++)
        {
            if (PortsImplemented & (1 << i))
            {
                PortType portType = CheckPortType(&ABAR->Ports[i]);
                if (portType == PortType::SATA || portType == PortType::SATAPI)
                {
                    // trace("%s drive found at port %d", PortTypeName[portType], i);
                    print("SATA drive found.");
                    Ports[PortCount] = new Port(portType, &ABAR->Ports[i], PortCount);
                    PortCount++;
                }
                else
                {
                    if (portType != PortType::None)
                        print("Unsupported port type found.");
                    // warn("Unsupported drive type %s found at port %d", PortTypeName[portType], i);
                }
            }
        }

        for (int i = 0; i < PortCount; i++)
            Ports[i]->Configure();
        break;
    }
    case FetchReason:
    {
        Data->DiskCallback.Fetch.Ports = PortCount;
        Data->DiskCallback.Fetch.BytesPerSector = 512;
        break;
    }
    case StopReason:
    {
        // TODO: Stop the driver.
        print("Driver stopped.");
        break;
    }
    case SendReason:
    case ReceiveReason:
    {
        Ports[Data->DiskCallback.RW.Port]->ReadWrite(Data->DiskCallback.RW.Sector,
                                                     Data->DiskCallback.RW.SectorCount,
                                                     Data->DiskCallback.RW.Buffer,
                                                     Data->DiskCallback.RW.Write);
        break;
    }
    default:
    {
        print("Unknown reason.");
        break;
    }
    }
    return OK;
}

int InterruptCallback(CPURegisters *)
{
    /* There's no need to do anything here. */
    return OK;
}
