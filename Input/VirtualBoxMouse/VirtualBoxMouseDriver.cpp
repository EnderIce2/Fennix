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
        .Name = "VMware Virtual Mouse Driver",
        .Type = FexDriverType_Input,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x80EE},
                .DeviceID = {0xCAFE},
                .Class = 0x2,
                .SubClass = 0x0,
                .ProgIF = 0x0,
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    print("Not implemented!");
    return NOT_IMPLEMENTED;

    // return OK;
}

int MouseX = 0, MouseY = 0, MouseZ = 0;
int MouseButton = 0;

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
        break;
    }
    case FetchReason:
    {
        Data->InputCallback.Mouse.X = MouseX;
        Data->InputCallback.Mouse.Y = MouseY;
        Data->InputCallback.Mouse.Z = MouseZ;
        Data->InputCallback.Mouse.Buttons.Left = MouseButton & 0x20;
        Data->InputCallback.Mouse.Buttons.Right = MouseButton & 0x10;
        Data->InputCallback.Mouse.Buttons.Middle = MouseButton & 0x08;
        break;
    }
    case StopReason:
    {
        break;
    }
    case InterruptReason:
    {
        break;
    }

    default:
    {
        break;
    }
    }
    return OK;
}
