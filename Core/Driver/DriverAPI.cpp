/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <driver.hpp>

#include <dumper.hpp>
#include <lock.hpp>

#include "../../kernel.h"
#include "../../Fex.hpp"
#include "api.hpp"

// show debug messages
// #define DEBUG_DRIVER_API 1

#ifdef DEBUG_DRIVER_API
#define drvdbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define drvdbg(m, ...)
#endif

NewLock(DriverDisplayPrintLock);

void DriverDebugPrint(char *String, unsigned long DriverUID) { trace("[%ld] %s", DriverUID, String); }

void DriverDisplayPrint(char *String)
{
    SmartLock(DriverDisplayPrintLock);
    for (unsigned long i = 0; i < strlen(String); i++)
        Display->Print(String[i], 0, true);
}

void *RequestPage(unsigned long Size)
{
    void *ret = KernelAllocator.RequestPages(Size + 1);
    drvdbg("Allocated %ld pages (%#lx-%#lx)", Size, (unsigned long)ret, (unsigned long)ret + FROM_PAGES(Size));
    return ret;
}

void FreePage(void *Page, unsigned long Size)
{
    drvdbg("Freeing %ld pages (%#lx-%#lx)", Size, (unsigned long)Page, (unsigned long)Page + FROM_PAGES(Size));
    KernelAllocator.FreePages(Page, Size + 1);
}

void MapMemory(void *VirtualAddress, void *PhysicalAddress, unsigned long Flags)
{
    SmartLock(DriverDisplayPrintLock);
    drvdbg("Mapping %#lx to %#lx with flags %#lx...", (unsigned long)VirtualAddress, (unsigned long)PhysicalAddress, Flags);
    Memory::Virtual(KernelPageTable).Map(VirtualAddress, PhysicalAddress, Flags);
}

void UnmapMemory(void *VirtualAddress)
{
    SmartLock(DriverDisplayPrintLock);
    drvdbg("Unmapping %#lx...", (unsigned long)VirtualAddress);
    Memory::Virtual(KernelPageTable).Unmap(VirtualAddress);
}

void *Drivermemcpy(void *Destination, void *Source, unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    drvdbg("Copying %ld bytes from %#lx-%#lx to %#lx-%#lx...", Size,
           (unsigned long)Source, (unsigned long)Source + Size,
           (unsigned long)Destination, (unsigned long)Destination + Size);
    return memcpy(Destination, Source, Size);
}

void *Drivermemset(void *Destination, int Value, unsigned long Size)
{
    SmartLock(DriverDisplayPrintLock);
    drvdbg("Setting value %#x at %#lx-%#lx (%ld bytes)...", Value,
           (unsigned long)Destination, (unsigned long)Destination + Size,
           Size);
    return memset(Destination, Value, Size);
}

void DriverNetSend(unsigned int DriverID, unsigned char *Data, unsigned short Size)
{
    // This is useless I guess...
    if (NIManager)
        NIManager->DrvSend(DriverID, Data, Size);
}

void DriverNetReceive(unsigned int DriverID, unsigned char *Data, unsigned short Size)
{
    if (NIManager)
        NIManager->DrvReceive(DriverID, Data, Size);
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

unsigned int DriverGetWidth()
{
    /* TODO: We won't rely only on display buffers, what about graphics drivers and changing resolutions? */
    return Display->GetBuffer(0)->Width;
}

unsigned int DriverGetHeight()
{
    /* TODO: We won't rely only on display buffers, what about graphics drivers and changing resolutions? */
    return Display->GetBuffer(0)->Height;
}

void DriverSleep(unsigned long Milliseconds)
{
    SmartLock(DriverDisplayPrintLock);
    drvdbg("Sleeping for %ld milliseconds...", Milliseconds);
    if (TaskManager)
        TaskManager->Sleep(Milliseconds);
    else
        TimeManager->Sleep(Milliseconds, Time::Units::Milliseconds);
}

int Driversprintf(char *Buffer, const char *Format, ...)
{
    va_list args;
    va_start(args, Format);
    int ret = vsprintf(Buffer, Format, args);
    va_end(args);
    return ret;
}

KernelAPI KernelAPITemplate = {
    .Version = {
        .Major = 0,
        .Minor = 0,
        .Patch = 1},
    .Info = {
        .Offset = 0,
        .DriverUID = 0,
        .KernelDebug = false,
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
        .Sleep = DriverSleep,
        .sprintf = Driversprintf,
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
    .Display = {
        .GetWidth = DriverGetWidth,
        .GetHeight = DriverGetHeight,
    },
};
