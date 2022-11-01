#include <netools.h>
#include <pci.h>
#include <io.h>

#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(KernelAPI *Data);
int CallbackHandler(KernelCallback *Data);

HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "AHCI",
        .Type = FexDriverType_Storage,
        .Callback = CallbackHandler,
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

/* --------------------------------------------------------------------------------------------------------- */

struct BARData
{
    uint8_t Type;
    uint64_t IOBase;
    uint64_t MemoryBase;
};

PCIDeviceHeader *PCIBaseAddress;

int DriverEntry(KernelAPI *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    if (Data->Version.Major < 0 || Data->Version.Minor < 0 || Data->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;
    KAPI = Data;
    return OK;
}

int CallbackHandler(KernelCallback *Data)
{
    switch (Data->Reason)
    {
    case AcknowledgeReason:
    {
        KAPI->Util.DebugPrint(((char *)"Kernel acknowledged the driver." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        break;
    }
    case ConfigurationReason:
    {
        KAPI->Util.DebugPrint(((char *)"Kernel received configuration data." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
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
    default:
    {
        KAPI->Util.DebugPrint(((char *)"Unknown reason." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        break;
    }
    }
    return OK;
}
