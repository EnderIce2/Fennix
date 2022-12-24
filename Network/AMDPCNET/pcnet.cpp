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
        .Name = "AMD PCNET",
        .Type = FexDriverType_Network,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x1022},
                .DeviceID = {0x2000},
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

MediaAccessControl MAC;
InternetProtocol4 IP;

void WriteRAP32(uint32_t Value) { outportl(BAR.IOBase + 0x14, Value); }
void WriteRAP16(uint16_t Value) { outportw(BAR.IOBase + 0x12, Value); }

uint32_t ReadCSR32(uint32_t CSR)
{
    WriteRAP32(CSR);
    return inportl(BAR.IOBase + 0x10);
}

uint16_t ReadCSR16(uint16_t CSR)
{
    WriteRAP32(CSR);
    return inportw(BAR.IOBase + 0x10);
}

void WriteCSR32(uint32_t CSR, uint32_t Value)
{
    WriteRAP32(CSR);
    outportl(BAR.IOBase + 0x10, Value);
}

void WriteCSR16(uint16_t CSR, uint16_t Value)
{
    WriteRAP16(CSR);
    outportw(BAR.IOBase + 0x10, Value);
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
        if (PCIBaseAddress->VendorID == 0x1022 && PCIBaseAddress->DeviceID == 0x2000)
        {
            print("Found AMD PCNET.");
            uint32_t PCIBAR = ((PCIHeader0 *)PCIBaseAddress)->BAR0;
            BAR.Type = PCIBAR & 1;
            BAR.IOBase = PCIBAR & (~3);
            BAR.MemoryBase = PCIBAR & (~15);
        }
        else
            return DEVICE_NOT_SUPPORTED;
        break;
    }
    case FetchReason:
    {
        KAPI->Util.memcpy(Data->NetworkCallback.Fetch.Name, (void *)"AMD PCNET", 10);
        Data->NetworkCallback.Fetch.MAC = MAC.ToHex();
        break;
    }
    case InterruptReason:
    {
        break;
    }
    case SendReason:
    {
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
