#include <driver.hpp>

#include <dumper.hpp>
#include <lock.hpp>

#include "../../kernel.h"
#include "../../Fex.hpp"
#include "api.hpp"

NewLock(DriverDisplayPrintLock);

void DriverDebugPrint(char *String, unsigned long DriverUID)
{
    SmartLock(DriverDisplayPrintLock);
    trace("[%ld] %s", DriverUID, String);
}

void DriverDisplayPrint(char *String)
{
    SmartLock(DriverDisplayPrintLock);
    for (unsigned long i = 0; i < strlen(String); i++)
        Display->Print(String[i], 0, true);
}

void *RequestPage(unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    // debug("Requesting %ld pages from the kernel...", Size);
    void *ret = KernelAllocator.RequestPages(Size);
    // debug("Got %#lx", ret);
    return ret;
}

void FreePage(void *Page, unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    debug("Freeing %ld pages from the address %#lx...", Size, (unsigned long)Page);
    KernelAllocator.FreePages(Page, Size);
}

void MapMemory(void *VirtualAddress, void *PhysicalAddress, unsigned long Flags)
{
    SmartLock(DriverDisplayPrintLock);
    debug("Mapping %#lx to %#lx with flags %#lx...", (unsigned long)VirtualAddress, (unsigned long)PhysicalAddress, Flags);
    Memory::Virtual().Map(VirtualAddress, PhysicalAddress, Flags);
}

void UnmapMemory(void *VirtualAddress)
{
    SmartLock(DriverDisplayPrintLock);
    debug("Unmapping %#lx...", (unsigned long)VirtualAddress);
    Memory::Virtual().Unmap(VirtualAddress);
}

void *Drivermemcpy(void *Destination, void *Source, unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    // debug("Copying %ld bytes from %#lx to %#lx...", Size, (unsigned long)Source, (unsigned long)Destination);
    return memcpy(Destination, Source, Size);
}

void *Drivermemset(void *Destination, int Value, unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    // debug("Setting %ld bytes from %#lx to %#x...", Size, (unsigned long)Destination, Value);
    return memset(Destination, Value, Size);
}

void DriverNetSend(unsigned int DriverID, unsigned char *Data, unsigned short Size)
{
    DumpData("DriverNetSend", Data, Size);
    UNUSED(DriverID);
}

void DriverNetReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size)
{
    DumpData("DriverNetReceive", Data, Size);
    UNUSED(DriverID);
}

void DriverAHCIDiskRead(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port)
{
    DumpData("DriverDiskRead", Data, SectorCount * 512);
    UNUSED(DriverID);
    UNUSED(Sector);
    UNUSED(Port);
}

void DriverAHCIDiskWrite(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port)
{
    DumpData("DriverDiskWrite", Data, SectorCount * 512);
    UNUSED(DriverID);
    UNUSED(Sector);
    UNUSED(Port);
}

char *DriverPCIGetDeviceName(unsigned int VendorID, unsigned int DeviceID)
{
    UNUSED(VendorID);
    UNUSED(DeviceID);
    return (char *)"Unknown";
}

KernelAPI KernelAPITemplate = {
    .Version = {
        .Major = 0,
        .Minor = 0,
        .Patch = 1},
    .Info = {
        .Offset = 0,
        .DriverUID = 0,
    },
    .Memory = {
        .PageSize = PAGE_SIZE,
        .RequestPage = RequestPage,
        .FreePage = FreePage,
        .Map = MapMemory,
        .Unmap = UnmapMemory,
    },
    .PCI = {
        .GetDeviceName = DriverPCIGetDeviceName,
    },
    .Util = {
        .DebugPrint = DriverDebugPrint,
        .DisplayPrint = DriverDisplayPrint,
        .memcpy = Drivermemcpy,
        .memset = Drivermemset,
    },
    .Command = {
        .Network = {
            .SendPacket = DriverNetSend,
            .ReceivePacket = DriverNetReceive,
        },
        .Disk = {
            .AHCI = {
                .ReadSector = DriverAHCIDiskRead,
                .WriteSector = DriverAHCIDiskWrite,
            },
        },
    },
};
