#include <netools.h>
#include <pci.h>
#include <io.h>

#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(void *Data);
int CallbackHandler(KernelCallback *Data);

HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "Intel Gigabit Ethernet Controller",
        .Type = FexDriverType_Network,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x8086},
                .DeviceID = {0x100E, 0x100F, 0x10D3, 0x10EA, 0x153A},
                .Class = 0x2,
                .SubClass = 0x0,
                .ProgIF = 0x0,
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

enum REG
{
    CTRL = 0x0000,
    STATUS = 0x0008,
    ICR = 0x000C,
    EEPROM = 0x0014,
    CTRL_EXT = 0x0018,
    ITR = 0x00C4,
    IMASK = 0x00D0,
    IAM = 0x00D8,
    RCTRL = 0x0100,
    RXDESCLO = 0x2800,
    RXDESCHI = 0x2804,
    RXDESCLEN = 0x2808,
    RXDESCHEAD = 0x2810,
    RXDESCTAIL = 0x2818,
    TCTRL = 0x0400,
    TXDESCLO = 0x3800,
    TXDESCHI = 0x3804,
    TXDESCLEN = 0x3808,
    TXDESCHEAD = 0x3810,
    TXDESCTAIL = 0x3818,
    RDTR = 0x2820,
    RXDCTL = 0x3828,
    RADV = 0x282C,
    RSRPD = 0x2C00,
    TIPG = 0x0410
};

enum PCTRL
{
    RESERVED = 0b000000,                 // bits 5:0
    SPEED_SELECTION_MSB = 0b010000,      // bit 6
    UPDATE_COLLISION_TEST = 0b001000,    // bit 7
    DUPLEX_MODE = 0b000100,              // bit 8
    RESTART_AUTO_NEGOTIATION = 0b000010, // bit 9
    ISOLATE = 0b000001,                  // bit 10
    POWER_DOWN = 0b100000,               // bit 11
    SPEED_SELECTION_LSB = 0b100000,      // bit 13
};

enum ECTRL
{
    SLU = 0x40
};

enum RTCL
{
    RDMTS_HALF = (0 << 8),
    RDMTS_QUARTER = (1 << 8),
    RDMTS_EIGHTH = (2 << 8)
};

enum RCTL
{
    EN = (1 << 1),
    SBP = (1 << 2),
    UPE = (1 << 3),
    MPE = (1 << 4),
    LPE = (1 << 5),
    LBM_NONE = (0 << 6),
    LBM_PHY = (3 << 6),
    MO_36 = (0 << 12),
    MO_35 = (1 << 12),
    MO_34 = (2 << 12),
    MO_32 = (3 << 12),
    BAM = (1 << 15),
    VFE = (1 << 18),
    CFIEN = (1 << 19),
    CFI = (1 << 20),
    DPF = (1 << 22),
    PMCF = (1 << 23),
    SECRC = (1 << 26),
    BSIZE_256 = (3 << 16),
    BSIZE_512 = (2 << 16),
    BSIZE_1024 = (1 << 16),
    BSIZE_2048 = (0 << 16),
    BSIZE_4096 = ((3 << 16) | (1 << 25)),
    BSIZE_8192 = ((2 << 16) | (1 << 25)),
    BSIZE_16384 = ((1 << 16) | (1 << 25))
};

enum CMD
{
    EOP = (1 << 0),
    IFCS = (1 << 1),
    IC = (1 << 2),
    RS = (1 << 3),
    RPS = (1 << 4),
    VLE = (1 << 6),
    IDE = (1 << 7)
};

enum TCTL
{
    EN_ = (1 << 1),
    PSP = (1 << 3),
    CT_SHIFT = 4,
    COLD_SHIFT = 12,
    SWXOFF = (1 << 22),
    RTLC = (1 << 24)
};

enum TSTA
{
    DD = (1 << 0),
    EC = (1 << 1),
    LC = (1 << 2)
};

enum LSTA
{
    LSTA_TU = (1 << 3)
};

struct RXDescriptor
{
    volatile uint64_t Address;
    volatile uint16_t Length;
    volatile uint16_t Checksum;
    volatile uint8_t Status;
    volatile uint8_t Errors;
    volatile uint16_t Special;
} __attribute__((packed));

struct TXDescriptor
{
    volatile uint64_t Address;
    volatile uint16_t Length;
    volatile uint8_t cso;
    volatile uint8_t Command;
    volatile uint8_t Status;
    volatile uint8_t css;
    volatile uint16_t Special;
} __attribute__((packed));

struct BARData
{
    uint8_t Type;
    uint64_t IOBase;
    uint64_t MemoryBase;
};

PCIDeviceHeader *PCIBaseAddress;
uint32_t CurrentPacket;
BARData BAR;
bool EEPROMAvailable;

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

uint16_t RXCurrent;
uint16_t TXCurrent;
RXDescriptor *RX[E1000_NUM_RX_DESC];
TXDescriptor *TX[E1000_NUM_TX_DESC];

MediaAccessControl MAC;
InternetProtocol4 IP;

void WriteCMD(uint16_t Address, uint32_t Value)
{
    if (BAR.Type == 0)
        mmioout32(BAR.MemoryBase + Address, Value);
    else
    {
        outportl(BAR.IOBase, Address);
        outportl(BAR.IOBase + 4, Value);
    }
}

uint32_t ReadCMD(uint16_t Address)
{
    if (BAR.Type == 0)
        return mmioin32(BAR.MemoryBase + Address);
    else
    {
        outportl(BAR.IOBase, Address);
        return inportl(BAR.IOBase + 0x4);
    }
}

uint32_t ReadEEPROM(uint8_t Address)
{
    uint16_t Data = 0;
    uint32_t temp = 0;
    if (EEPROMAvailable)
    {
        WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 8));
        while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 4)))
            ;
    }
    else
    {
        WriteCMD(REG::EEPROM, (1) | ((uint32_t)(Address) << 2));
        while (!((temp = ReadCMD(REG::EEPROM)) & (1 << 1)))
            ;
    }
    Data = (uint16_t)((temp >> 16) & 0xFFFF);
    return Data;
}

MediaAccessControl GetMAC()
{
    MediaAccessControl mac;
    if (EEPROMAvailable)
    {
        uint32_t temp;
        temp = ReadEEPROM(0);
        mac.Address[0] = temp & 0xff;
        mac.Address[1] = temp >> 8;
        temp = ReadEEPROM(1);
        mac.Address[2] = temp & 0xff;
        mac.Address[3] = temp >> 8;
        temp = ReadEEPROM(2);
        mac.Address[4] = temp & 0xff;
        mac.Address[5] = temp >> 8;
    }
    else
    {
        uint8_t *BaseMac8 = (uint8_t *)(BAR.MemoryBase + 0x5400);
        uint32_t *BaseMac32 = (uint32_t *)(BAR.MemoryBase + 0x5400);
        if (BaseMac32[0] != 0)
            for (int i = 0; i < 6; i++)
                mac.Address[i] = BaseMac8[i];
        else
        {
            print("No MAC address found.");
            return MediaAccessControl();
        }
    }
    return mac;
}

void InitializeRX()
{
    print("Initializing RX...");
    uint8_t *Ptr = (uint8_t *)KAPI->Memory.RequestPage((((sizeof(RXDescriptor) * E1000_NUM_RX_DESC + 16)) / KAPI->Memory.PageSize) + 1);
    RXDescriptor *Descriptor = (RXDescriptor *)Ptr;

    for (int i = 0; i < E1000_NUM_RX_DESC; i++)
    {
        RX[i] = (RXDescriptor *)((uint8_t *)Descriptor + i * 16);
        RX[i]->Address = (uint64_t)(uint8_t *)KAPI->Memory.RequestPage(((8192 + 16) / KAPI->Memory.PageSize) + 1);
        RX[i]->Status = 0;
    }

    WriteCMD(REG::TXDESCLO, (uint32_t)((uint64_t)Ptr >> 32));
    WriteCMD(REG::TXDESCHI, (uint32_t)((uint64_t)Ptr & 0xFFFFFFFF));

    WriteCMD(REG::RXDESCLO, (uint64_t)Ptr);
    WriteCMD(REG::RXDESCHI, 0);

    WriteCMD(REG::RXDESCLEN, E1000_NUM_RX_DESC * 16);

    WriteCMD(REG::RXDESCHEAD, 0);
    WriteCMD(REG::RXDESCTAIL, E1000_NUM_RX_DESC - 1);
    RXCurrent = 0;
    WriteCMD(REG::RCTRL, RCTL::EN | RCTL::SBP | RCTL::UPE | RCTL::MPE | RCTL::LBM_NONE | RTCL::RDMTS_HALF | RCTL::BAM | RCTL::SECRC | RCTL::BSIZE_8192);
}

void InitializeTX()
{
    print("Initializing TX...");
    uint8_t *Ptr = (uint8_t *)KAPI->Memory.RequestPage(((sizeof(TXDescriptor) * E1000_NUM_RX_DESC + 16) / KAPI->Memory.PageSize) + 1);
    TXDescriptor *Descriptor = (TXDescriptor *)Ptr;

    for (int i = 0; i < E1000_NUM_TX_DESC; i++)
    {
        TX[i] = (TXDescriptor *)((uint8_t *)Descriptor + i * 16);
        TX[i]->Address = 0;
        TX[i]->Command = 0;
        TX[i]->Status = TSTA::DD;
    }

    WriteCMD(REG::TXDESCHI, (uint32_t)((uint64_t)Ptr >> 32));
    WriteCMD(REG::TXDESCLO, (uint32_t)((uint64_t)Ptr & 0xFFFFFFFF));

    WriteCMD(REG::TXDESCLEN, E1000_NUM_TX_DESC * 16);

    WriteCMD(REG::TXDESCHEAD, 0);
    WriteCMD(REG::TXDESCTAIL, 0);
    TXCurrent = 0;
    WriteCMD(REG::TCTRL, TCTL::EN_ | TCTL::PSP | (15 << TCTL::CT_SHIFT) | (64 << TCTL::COLD_SHIFT) | TCTL::RTLC);

    WriteCMD(REG::TCTRL, 0b0110000000000111111000011111010);
    WriteCMD(REG::TIPG, 0x0060200A);
}

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;
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
        switch (PCIBaseAddress->DeviceID)
        {
        case 0x100E:
        {
            print("Found Intel 82540EM Gigabit Ethernet Controller.");

            PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
            uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
            uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

            BAR.Type = PCIBAR0 & 1;
            BAR.IOBase = PCIBAR1 & (~3);
            BAR.MemoryBase = PCIBAR0 & (~15);

            // Detect EEPROM
            WriteCMD(REG::EEPROM, 0x1);
            for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
                if (ReadCMD(REG::EEPROM) & 0x10)
                    EEPROMAvailable = true;
                else
                    EEPROMAvailable = false;

            // Get MAC address
            if (!GetMAC().Valid())
                return NOT_AVAILABLE;
            else
                print("MAC address found.");
            MAC = GetMAC();

            // Start link
            uint32_t cmdret = ReadCMD(REG::CTRL);
            WriteCMD(REG::CTRL, cmdret | ECTRL::SLU);

            for (int i = 0; i < 0x80; i++)
                WriteCMD(0x5200 + i * 4, 0);

            WriteCMD(REG::IMASK, 0x1F6DC);
            WriteCMD(REG::IMASK, 0xFF & ~4);
            ReadCMD(0xC0);

            InitializeRX();
            InitializeTX();
            return OK;
        }
        case 0x100F:
        {
            print("Found Intel 82545EM Gigabit Ethernet Controller.");
            PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
            uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
            uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

            BAR.Type = PCIBAR0 & 1;
            BAR.IOBase = PCIBAR1 & (~3);
            BAR.MemoryBase = PCIBAR0 & (~15);

            // Detect EEPROM
            WriteCMD(REG::EEPROM, 0x1);
            for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
                if (ReadCMD(REG::EEPROM) & 0x10)
                    EEPROMAvailable = true;
                else
                    EEPROMAvailable = false;

            // Get MAC address
            if (!GetMAC().Valid())
                return NOT_AVAILABLE;
            else
                print("MAC address found.");
            MAC = GetMAC();

            return NOT_IMPLEMENTED;
        }
        case 0x10D3:
        {
            print("Found Intel 82574L Gigabit Ethernet Controller.");

            PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
            uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
            uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

            BAR.Type = PCIBAR0 & 1;
            BAR.IOBase = PCIBAR1 & (~3);
            BAR.MemoryBase = PCIBAR0 & (~15);

            // Detect EEPROM
            WriteCMD(REG::EEPROM, 0x1);
            for (int i = 0; i < 1000 && !EEPROMAvailable; i++)
                if (ReadCMD(REG::EEPROM) & 0x10)
                    EEPROMAvailable = true;
                else
                    EEPROMAvailable = false;

            // Get MAC address
            if (!GetMAC().Valid())
                return NOT_AVAILABLE;
            else
                print("MAC address found.");
            MAC = GetMAC();

            return NOT_IMPLEMENTED;
        }
        case 0x10EA:
        {
            print("Found Intel I217-LM Gigabit Ethernet Controller.");
            return NOT_IMPLEMENTED;
        }
        case 0x153A:
        {
            print("Found Intel 82577LM Gigabit Ethernet Controller.");
            return NOT_IMPLEMENTED;
        }
        default:
        {
            print("Unsupported Intel Ethernet Controller.");
            return DEVICE_NOT_SUPPORTED;
        }
        }
        return ERROR;
    }
    case FetchReason:
    {
        KAPI->Util.memcpy(Data->NetworkCallback.Fetch.Name, (void *)"Intel Gigabit Ethernet Controller", 34);
        Data->NetworkCallback.Fetch.MAC = MAC.ToHex();
        break;
    }
    case InterruptReason:
    {
        WriteCMD(REG::IMASK, 0x1);
        uint32_t status = ReadCMD(0xC0);
        UNUSED(status);

        while ((RX[RXCurrent]->Status & 0x1))
        {
            uint8_t *Data = (uint8_t *)RX[RXCurrent]->Address;
            uint16_t DataLength = RX[RXCurrent]->Length;
            KAPI->Command.Network.ReceivePacket(KAPI->Info.DriverUID, Data, DataLength);
            RX[RXCurrent]->Status = 0;
            uint16_t OldRXCurrent = RXCurrent;
            RXCurrent = (RXCurrent + 1) % E1000_NUM_RX_DESC;
            WriteCMD(REG::RXDESCTAIL, OldRXCurrent);
        }
        break;
    }
    case SendReason:
    {
        TX[TXCurrent]->Address = (uint64_t)Data->NetworkCallback.Send.Data;
        TX[TXCurrent]->Length = Data->NetworkCallback.Send.Length;
        TX[TXCurrent]->Command = CMD::EOP | CMD::IFCS | CMD::RS;
        TX[TXCurrent]->Status = 0;
        uint8_t OldTXCurrent = TXCurrent;
        TXCurrent = (TXCurrent + 1) % E1000_NUM_TX_DESC;
        WriteCMD(REG::TXDESCTAIL, TXCurrent);
        while (!(TX[OldTXCurrent]->Status & 0xFF))
            ;
        break;
    }
    case StopReason:
    {
        // Clearing Enable bit in Receive Control Register
        uint64_t cmdret = ReadCMD(REG::RCTRL);
        WriteCMD(REG::RCTRL, cmdret & ~RCTL::EN);

        // Masking Interrupt Mask, Interrupt Throttling Rate & Interrupt Auto-Mask
        WriteCMD(REG::IMASK, 0x00000000);
        WriteCMD(REG::ITR, 0x00000000);
        WriteCMD(REG::IAM, 0x00000000);

        // Clearing SLU bit in Device Control Register
        cmdret = ReadCMD(REG::CTRL);
        WriteCMD(REG::CTRL, cmdret & ~ECTRL::SLU);

        // Clear the Interrupt Cause Read register by reading it
        ReadCMD(REG::ICR);

        // Powering down the device (?)
        WriteCMD(REG::CTRL, PCTRL::POWER_DOWN);
        /* TODO: Stop link; further testing required */
        print("Driver stopped.");
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
