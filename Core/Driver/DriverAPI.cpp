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

void DriverDebugPrint(char *String, __UINT64_TYPE__ DriverUID)
{
	trace("[%ld] %s", DriverUID, String);
}

void DriverDisplayPrint(char *String)
{
	SmartLock(DriverDisplayPrintLock);
	for (__UINT64_TYPE__ i = 0; i < strlen(String); i++)
		Display->Print(String[i], 0, true);
}

void *RequestPage(__UINT64_TYPE__ Size)
{
	void *ret = KernelAllocator.RequestPages(size_t(Size + 1));
	drvdbg("Allocated %ld pages (%#lx-%#lx)",
		   Size, (__UINT64_TYPE__)ret,
		   (__UINT64_TYPE__)ret + FROM_PAGES(Size));
	return ret;
}

void FreePage(void *Page, __UINT64_TYPE__ Size)
{
	drvdbg("Freeing %ld pages (%#lx-%#lx)",
		   Size, (__UINT64_TYPE__)Page,
		   (__UINT64_TYPE__)Page + FROM_PAGES(Size));
	KernelAllocator.FreePages(Page, size_t(Size + 1));
}

void MapMemory(void *VirtualAddress, void *PhysicalAddress, __UINT64_TYPE__ Flags)
{
	SmartLock(DriverDisplayPrintLock);
	drvdbg("Mapping %#lx to %#lx with flags %#lx...",
		   (__UINT64_TYPE__)VirtualAddress,
		   (__UINT64_TYPE__)PhysicalAddress, Flags);
	Memory::Virtual(KernelPageTable).Map(VirtualAddress, PhysicalAddress, Flags);
}

void UnmapMemory(void *VirtualAddress)
{
	SmartLock(DriverDisplayPrintLock);
	drvdbg("Unmapping %#lx...",
		   (__UINT64_TYPE__)VirtualAddress);
	Memory::Virtual(KernelPageTable).Unmap(VirtualAddress);
}

void *Drivermemcpy(void *Destination, void *Source, __UINT64_TYPE__ Size)
{
	SmartLock(DriverDisplayPrintLock);
	drvdbg("Copying %ld bytes from %#lx-%#lx to %#lx-%#lx...", Size,
		   (__UINT64_TYPE__)Source, (__UINT64_TYPE__)Source + Size,
		   (__UINT64_TYPE__)Destination, (__UINT64_TYPE__)Destination + Size);
	return memcpy(Destination, Source, size_t(Size));
}

void *Drivermemset(void *Destination, int Value, __UINT64_TYPE__ Size)
{
	SmartLock(DriverDisplayPrintLock);
	drvdbg("Setting value %#x at %#lx-%#lx (%ld bytes)...", Value,
		   (__UINT64_TYPE__)Destination,
		   (__UINT64_TYPE__)Destination + Size, Size);
	return memset(Destination, Value, size_t(Size));
}

void DriverNetSend(__UINT32_TYPE__ DriverID,
				   __UINT8_TYPE__ *Data,
				   __UINT16_TYPE__ Size)
{
	// This is useless I guess...
	if (NIManager)
		NIManager->DrvSend(DriverID, Data, Size);
}

void DriverNetReceive(__UINT32_TYPE__ DriverID,
					  __UINT8_TYPE__ *Data,
					  __UINT16_TYPE__ Size)
{
	if (NIManager)
		NIManager->DrvReceive(DriverID, Data, Size);
}

void DriverAHCIDiskRead(__UINT32_TYPE__ DriverID,
						__UINT64_TYPE__ Sector,
						__UINT8_TYPE__ *Data,
						__UINT32_TYPE__ SectorCount,
						__UINT8_TYPE__ Port)
{
	DumpData("DriverDiskRead", Data, SectorCount * 512);
	UNUSED(DriverID);
	UNUSED(Sector);
	UNUSED(Port);
}

void DriverAHCIDiskWrite(__UINT32_TYPE__ DriverID,
						 __UINT64_TYPE__ Sector,
						 __UINT8_TYPE__ *Data,
						 __UINT32_TYPE__ SectorCount,
						 __UINT8_TYPE__ Port)
{
	DumpData("DriverDiskWrite",
			 Data, SectorCount * 512);
	UNUSED(DriverID);
	UNUSED(Sector);
	UNUSED(Port);
}

char *DriverPCIGetDeviceName(__UINT32_TYPE__ VendorID,
							 __UINT32_TYPE__ DeviceID)
{
	UNUSED(VendorID);
	UNUSED(DeviceID);
	return (char *)"Unknown";
}

__UINT32_TYPE__ DriverGetWidth()
{
	/* TODO: We won't rely only on display buffers,
		what about graphics drivers and changing resolutions? */
	return Display->GetBuffer(0)->Width;
}

__UINT32_TYPE__ DriverGetHeight()
{
	/* TODO: We won't rely only on display buffers,
		what about graphics drivers and changing resolutions? */
	return Display->GetBuffer(0)->Height;
}

void DriverSleep(__UINT64_TYPE__ Milliseconds)
{
	SmartLock(DriverDisplayPrintLock);
	drvdbg("Sleeping for %ld milliseconds...", Milliseconds);
	if (TaskManager)
		TaskManager->Sleep(Milliseconds);
	else
		TimeManager->Sleep(size_t(Milliseconds),
						   Time::Units::Milliseconds);
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
