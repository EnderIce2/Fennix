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
        .Name = "RTL8139",
        .Type = FexDriverType_Network,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x10EC},
                .DeviceID = {0x8139},
                .Class = 0x2,
                .SubClass = 0x0,
                .ProgIF = 0x0,
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

struct BARData
{
    uint8_t Type;
    uint64_t IOBase;
    uint64_t MemoryBase;
};

PCIDeviceHeader *PCIBaseAddress;
BARData BAR;

uint8_t *RXBuffer;
int TXCurrent;
uint32_t CurrentPacket;

MediaAccessControl MAC;
InternetProtocol4 IP;

uint8_t TSAD[4] = {0x20, 0x24, 0x28, 0x2C};
uint8_t TSD[4] = {0x10, 0x14, 0x18, 0x1C};

void RTLOB(uint16_t Address, uint8_t Value)
{
    if (BAR.Type == 0)
        mmoutb(reinterpret_cast<void *>(BAR.MemoryBase + Address), Value);
    else
        outportb(BAR.IOBase + Address, Value);
}

void RTLOW(uint16_t Address, uint16_t Value)
{
    if (BAR.Type == 0)
        mmoutw(reinterpret_cast<void *>(BAR.MemoryBase + Address), Value);
    else
        outportw(BAR.IOBase + Address, Value);
}

void RTLOL(uint16_t Address, uint32_t Value)
{
    if (BAR.Type == 0)
        mmoutl(reinterpret_cast<void *>(BAR.MemoryBase + Address), Value);
    else
        outportl(BAR.IOBase + Address, Value);
}

uint8_t RTLIB(uint16_t Address)
{
    if (BAR.Type == 0)
        return mminb(reinterpret_cast<void *>(BAR.MemoryBase + Address));
    else
        return inportb(BAR.IOBase + Address);
}

uint16_t RTLIW(uint16_t Address)
{
    if (BAR.Type == 0)
        return mminw(reinterpret_cast<void *>(BAR.MemoryBase + Address));
    else
        return inportw(BAR.IOBase + Address);
}

uint32_t RTLIL(uint16_t Address)
{
    if (BAR.Type == 0)
        return mminl(reinterpret_cast<void *>(BAR.MemoryBase + Address));
    else
        return inportl(BAR.IOBase + Address);
}

MediaAccessControl GetMAC()
{
    uint32_t MAC1 = RTLIL(0x0);
    uint16_t MAC2 = RTLIW(0x4);
    MediaAccessControl mac = {
        mac.Address[0] = MAC1,
        mac.Address[1] = (MAC1 >> 8),
        mac.Address[2] = (MAC1 >> 16),
        mac.Address[3] = (MAC1 >> 24),
        mac.Address[4] = MAC2,
        mac.Address[5] = (MAC2 >> 8)};
    return mac;
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
        print("Kernel received configuration data.");
        PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
        if (PCIBaseAddress->VendorID == 0x10EC && PCIBaseAddress->DeviceID == 0x8139)
        {
            print("Found RTL-8139.");

            PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
            uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
            uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

            BAR.Type = PCIBAR1 & 1;
            BAR.IOBase = PCIBAR0 & (~3);
            BAR.MemoryBase = PCIBAR1 & (~15);

            RXBuffer = (uint8_t *)KAPI->Memory.RequestPage(2);
            RTLOB(0x52, 0x0);
            RTLOB(0x37, (1 << 4));
            while ((RTLIB(0x37) & (1 << 4)))
                ;
            RTLOL(0x30, static_cast<uint32_t>(reinterpret_cast<uint64_t>(RXBuffer)));
            RTLOW(0x3C, ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) |
                         (1 << 4) | (1 << 5) | (1 << 6) | (1 << 13) |
                         (1 << 14) | (1 << 15)));
            RTLOL(0x44, ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 7)));
            RTLOB(0x37, 0x0C);
            MAC = GetMAC();
        }
        else
            return DEVICE_NOT_SUPPORTED;
        break;
    }
    case FetchReason:
    {
        KAPI->Util.memcpy(Data->NetworkCallback.Fetch.Name, (void *)"RTL-8139", 9);
        Data->NetworkCallback.Fetch.MAC = MAC.ToHex();
        break;
    }
    case InterruptReason:
    {
        uint16_t Status = RTLIW(0x3E);
        UNUSED(Status);

        uint16_t *Data = (uint16_t *)(RXBuffer + CurrentPacket);
        uint16_t DataLength = *(Data + 1);
        Data = Data + 2;
        KAPI->Command.Network.ReceivePacket(KAPI->Info.DriverUID, (uint8_t *)Data, DataLength);
        CurrentPacket = (CurrentPacket + DataLength + 4 + 3) & (~3);
        if (CurrentPacket > 8192)
            CurrentPacket -= 8192;
        RTLOW(0x38, CurrentPacket - 0x10);

        RTLOW(0x3E, (1 << 0) | (1 << 2));
        break;
    }
    case SendReason:
    {
        RTLOL(TSAD[TXCurrent], static_cast<uint32_t>(reinterpret_cast<uint64_t>(Data->NetworkCallback.Send.Data)));
        RTLOL(TSD[TXCurrent++], Data->NetworkCallback.Send.Length);
        if (TXCurrent > 3)
            TXCurrent = 0;
        break;
    }
    case StopReason:
    {
        // TODO: Stop the driver.
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
