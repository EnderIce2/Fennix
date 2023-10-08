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

#include <module.hpp>

#include <dumper.hpp>
#include <lock.hpp>

#include "../../kernel.h"
#include "../../Fex.hpp"
#include "api.hpp"

// show debug messages
// #define DEBUG_MODULE_API 1

#ifdef DEBUG_MODULE_API
#define modbg(m, ...) debug(m, ##__VA_ARGS__)
#else
#define modbg(m, ...)
#endif

NewLock(ModuleDisplayPrintLock);

void ModuleDebugPrint(char *String, __UINT64_TYPE__ modUniqueID)
{
	trace("[%ld] %s", modUniqueID, String);
}

void ModuleDisplayPrint(char *String)
{
	SmartLock(ModuleDisplayPrintLock);
	for (__UINT64_TYPE__ i = 0; i < strlen(String); i++)
		Display->Print(String[i], 0, true);
}

void *RequestPage(__UINT64_TYPE__ Size)
{
	void *ret = KernelAllocator.RequestPages(size_t(Size + 1));
	modbg("Allocated %ld pages (%#lx-%#lx)",
		   Size, (__UINT64_TYPE__)ret,
		   (__UINT64_TYPE__)ret + FROM_PAGES(Size));
	return ret;
}

void FreePage(void *Page, __UINT64_TYPE__ Size)
{
	modbg("Freeing %ld pages (%#lx-%#lx)",
		   Size, (__UINT64_TYPE__)Page,
		   (__UINT64_TYPE__)Page + FROM_PAGES(Size));
	KernelAllocator.FreePages(Page, size_t(Size + 1));
}

void MapMemory(void *VirtualAddress, void *PhysicalAddress, __UINT64_TYPE__ Flags)
{
	SmartLock(ModuleDisplayPrintLock);
	modbg("Mapping %#lx to %#lx with flags %#lx...",
		   (__UINT64_TYPE__)VirtualAddress,
		   (__UINT64_TYPE__)PhysicalAddress, Flags);
	Memory::Virtual(KernelPageTable).Map(VirtualAddress, PhysicalAddress, Flags);
}

void UnmapMemory(void *VirtualAddress)
{
	SmartLock(ModuleDisplayPrintLock);
	modbg("Unmapping %#lx...",
		   (__UINT64_TYPE__)VirtualAddress);
	Memory::Virtual(KernelPageTable).Unmap(VirtualAddress);
}

void *Modulememcpy(void *Destination, void *Source, __UINT64_TYPE__ Size)
{
	SmartLock(ModuleDisplayPrintLock);
	modbg("Copying %ld bytes from %#lx-%#lx to %#lx-%#lx...", Size,
		   (__UINT64_TYPE__)Source, (__UINT64_TYPE__)Source + Size,
		   (__UINT64_TYPE__)Destination, (__UINT64_TYPE__)Destination + Size);
	return memcpy(Destination, Source, size_t(Size));
}

void *Modulememset(void *Destination, int Value, __UINT64_TYPE__ Size)
{
	SmartLock(ModuleDisplayPrintLock);
	modbg("Setting value %#x at %#lx-%#lx (%ld bytes)...", Value,
		   (__UINT64_TYPE__)Destination,
		   (__UINT64_TYPE__)Destination + Size, Size);
	return memset(Destination, Value, size_t(Size));
}

void ModuleNetSend(__UINT32_TYPE__ ModuleID,
				   __UINT8_TYPE__ *Data,
				   __UINT16_TYPE__ Size)
{
	// This is useless I guess...
	if (NIManager)
		NIManager->DrvSend(ModuleID, Data, Size);
}

void ModuleNetReceive(__UINT32_TYPE__ ModuleID,
					  __UINT8_TYPE__ *Data,
					  __UINT16_TYPE__ Size)
{
	if (NIManager)
		NIManager->DrvReceive(ModuleID, Data, Size);
}

void ModuleAHCIDiskRead(__UINT32_TYPE__ ModuleID,
						__UINT64_TYPE__ Sector,
						__UINT8_TYPE__ *Data,
						__UINT32_TYPE__ SectorCount,
						__UINT8_TYPE__ Port)
{
	DumpData("ModuleDiskRead", Data, SectorCount * 512);
	UNUSED(ModuleID);
	UNUSED(Sector);
	UNUSED(Port);
}

void ModuleAHCIDiskWrite(__UINT32_TYPE__ ModuleID,
						 __UINT64_TYPE__ Sector,
						 __UINT8_TYPE__ *Data,
						 __UINT32_TYPE__ SectorCount,
						 __UINT8_TYPE__ Port)
{
	DumpData("ModuleDiskWrite",
			 Data, SectorCount * 512);
	UNUSED(ModuleID);
	UNUSED(Sector);
	UNUSED(Port);
}

char *ModulePCIGetDeviceName(__UINT32_TYPE__ VendorID,
							 __UINT32_TYPE__ DeviceID)
{
	UNUSED(VendorID);
	UNUSED(DeviceID);
	return (char *)"Unknown";
}

__UINT32_TYPE__ ModuleGetWidth()
{
	return Display->GetBuffer(0)->Width;
}

__UINT32_TYPE__ ModuleGetHeight()
{
	return Display->GetBuffer(0)->Height;
}

void ModuleSleep(__UINT64_TYPE__ Milliseconds)
{
	SmartLock(ModuleDisplayPrintLock);
	modbg("Sleeping for %ld milliseconds...", Milliseconds);
	if (TaskManager)
		TaskManager->Sleep(Milliseconds);
	else
		TimeManager->Sleep(size_t(Milliseconds),
						   Time::Units::Milliseconds);
}

int Modulesprintf(char *Buffer, const char *Format, ...)
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
		.modUniqueID = 0,
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
		.GetDeviceName = ModulePCIGetDeviceName,
	},
	.Util = {
		.DebugPrint = ModuleDebugPrint,
		.DisplayPrint = ModuleDisplayPrint,
		.memcpy = Modulememcpy,
		.memset = Modulememset,
		.Sleep = ModuleSleep,
		.sprintf = Modulesprintf,
	},
	.Command = {
		.Network = {
			.SendPacket = ModuleNetSend,
			.ReceivePacket = ModuleNetReceive,
		},
		.Disk = {
			.AHCI = {
				.ReadSector = ModuleAHCIDiskRead,
				.WriteSector = ModuleAHCIDiskWrite,
			},
		},
	},
	.Display = {
		.GetWidth = ModuleGetWidth,
		.GetHeight = ModuleGetHeight,
	},
};
