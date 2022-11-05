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
        .Name = "ATA",
        .Type = FexDriverType_Storage,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_INTERRUPT,
            .Interrupt = {
                .Vector = {0xE, 0xF}, // IRQ14, IRQ15
            }}}};

KernelAPI *KAPI;

/* --------------------------------------------------------------------------------------------------------- */

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
        KAPI->Util.DebugPrint(((char *)"Kernel acknowledged the driver." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        break;
    }
    case ConfigurationReason:
    {
        KAPI->Util.DebugPrint(((char *)"Kernel received configuration data." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        break;
    }
    case FetchReason:
    {
        break;
    }
    case InterruptReason:
    {
        if (Data->InterruptInfo.Vector == 0xE)
        {
            KAPI->Util.DebugPrint(((char *)"IRQ14" + KAPI->Info.Offset), KAPI->Info.DriverUID);
        }
        else if (Data->InterruptInfo.Vector == 0xF)
        {
            KAPI->Util.DebugPrint(((char *)"IRQ15" + KAPI->Info.Offset), KAPI->Info.DriverUID);
        }
        break;
    }
    case SendReason:
    case ReceiveReason:
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
