#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(void *Data);
int CallbackHandler(KernelCallback *Data);

/*                           The driver is
 *       This is a driver     for Fennix     Driver Entry Extended Header
 *              *                   *              *             */
HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

// Ignore the warning about missing field initializers
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// Extended header which is used to give additional information to the kernel
__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "Example Driver",
        .Type = FexDriverType_Generic,
        .Callback = CallbackHandler,
        .Bind = {
            .Type = BIND_INTERRUPT,
            .Interrupt = {
                .Vector = {0xFE},
            }}}};

// Global variable that holds the kernel API
KernelAPI *KAPI;

/* --------------------------------------------------------------------------------------------------------- */

// Driver entry point. This is called at initialization. "Data" argument points to the kernel API structure.
int DriverEntry(void *Data)
{
    // Check if kernel API is valid
    if (!Data)
        return INVALID_KERNEL_API;

    // Set the global variable to the kernel API
    KAPI = (KernelAPI *)Data;

    // Check if kernel API version is valid. this is important because the kernel API may change in the future.
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    // We print "Hello World!" to UART.
    KAPI->Util.DebugPrint(((char *)"Hello World!" + KAPI->Info.Offset), KAPI->Info.DriverUID);
    return OK;
}

// This is called when the driver is bound to an interrupt, process, or PCI device or when the kernel wants to send a message to the driver.
int CallbackHandler(KernelCallback *Data)
{
    switch (Data->Reason)
    {
    case AcknowledgeReason:
    {
        KAPI->Util.DebugPrint(((char *)"Kernel acknowledged the driver." + KAPI->Info.Offset), KAPI->Info.DriverUID);
        break;
    }
    case InterruptReason:
    {
        KAPI->Util.DebugPrint(((char *)"Interrupt received." + KAPI->Info.Offset), KAPI->Info.DriverUID);
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
