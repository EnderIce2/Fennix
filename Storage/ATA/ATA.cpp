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
        .Name = "ATA",
        .Type = FexDriverType_Storage,
        .Callback = CallbackHandler,
        .InterruptCallback = InterruptCallback,
        .Bind = {
            .Type = BIND_INTERRUPT,
            .Interrupt = {
                .Vector = {14, 15}, // IRQ14, IRQ15
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

bool IsATAPresent()
{
    outb(0x1F0 + 2, 0);
    outb(0x1F0 + 3, 0);
    outb(0x1F0 + 4, 0);
    outb(0x1F0 + 5, 0);
    outb(0x1F0 + 7, 0xEC);
    if (inb(0x1F0 + 7) == 0 || inb(0x1F0 + 1) != 0)
        return false;
    return true;
}

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    if (!IsATAPresent())
        return NOT_AVAILABLE;
    print("ATA device found.");

    return NOT_IMPLEMENTED;
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
        break;
    }
    case FetchReason:
    {
        break;
    }
    case SendReason:
    case ReceiveReason:
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

int InterruptCallback(CPURegisters *Registers)
{
    if (Registers->InterruptNumber == 0xE)
    {
        print("IRQ14");
    }
    else if (Registers->InterruptNumber == 0xF)
    {
        print("IRQ15");
    }
    return OK;
}
