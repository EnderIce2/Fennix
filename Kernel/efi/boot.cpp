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

#include <efi.h>

#include <boot/binfo.h>
#include <debug.h>

extern struct BootInfo bInfo;

VOID SearchSMBIOS(EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_GUID Smbios3TableGuid = {0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94}};
	EFI_GUID SmbiosTableGuid = {0xEB9D2D31, 0x2D88, 0x11D3, {0x9A, 0x16, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};

	for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++)
	{
		EFI_CONFIGURATION_TABLE *config = &SystemTable->ConfigurationTable[i];

		if (CompareGuid(&config->VendorGuid, &Smbios3TableGuid) ||
			CompareGuid(&config->VendorGuid, &SmbiosTableGuid))
		{
			bInfo.SMBIOSPtr = config->VendorTable;
			debug("SMBIOS EPS found at address: %#lx", bInfo.SMBIOSPtr);
			return;
		}
	}
}

VOID SearchRSDP(EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_GUID AcpiTableGuid = {0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};

	for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++)
	{
		EFI_CONFIGURATION_TABLE *config = &SystemTable->ConfigurationTable[i];
		if (CompareGuid(&config->VendorGuid, &AcpiTableGuid))
		{
			bInfo.RSDP = (BootInfo::RSDPInfo *)config->VendorTable;
			debug("RSDP found at address: %#lx", bInfo.RSDP);
			break;
		}
	}
}

VOID InitializeMemoryEntries(EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN NumberOfEntries, UINTN DescriptorSize)
{
	debug("Memory map: %#lx", MemoryMap);
	debug("Number of entries: %d", NumberOfEntries);
	debug("Descriptor size: %d", DescriptorSize);

	for (UINTN i = 0; i < NumberOfEntries; i++)
	{
		EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)MemoryMap + i * DescriptorSize);

#ifdef DEBUG
		const char *EFI_MEMORY_TYPE_STRINGS[] = {
			"ReservedMemoryType",
			"LoaderCode",
			"LoaderData",
			"BootServicesCode",
			"BootServicesData",
			"RuntimeServicesCode",
			"RuntimeServicesData",
			"ConventionalMemory",
			"UnusableMemory",
			"ACPIReclaimMemory",
			"ACPIMemoryNVS",
			"MemoryMappedIO",
			"MemoryMappedIOPortSpace",
			"PalCode",
			"PersistentMemory",
			"MaxMemoryType"};

		debug("Entry %d: Type: %s, PhysicalStart: %p, VirtualStart: %p, NumberOfPages: %lu, Attribute: %lx",
			  i, EFI_MEMORY_TYPE_STRINGS[desc->Type], desc->PhysicalStart, desc->VirtualStart, desc->NumberOfPages, desc->Attribute);
#endif
	}
}

VOID InitializeMemory(EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_MEMORY_DESCRIPTOR *MemoryMap = (EFI_MEMORY_DESCRIPTOR *)bInfo.EFI.MemoryMap.BaseAddress;
	UINTN NumberOfEntries = bInfo.EFI.MemoryMap.NumberOfEntries;
	UINTN DescriptorSize = bInfo.EFI.MemoryMap.DescriptorSize;

	EFI_STATUS Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, NumberOfEntries * DescriptorSize, (void **)&MemoryMap);
	if (EFI_ERROR(Status))
	{
		error("Failed to allocate memory for memory map: %#lx", Status);
		return;
	}

	Status = SystemTable->BootServices->GetMemoryMap(&NumberOfEntries, MemoryMap, &DescriptorSize, NULL, NULL);
	if (EFI_ERROR(Status))
	{
		error("Failed to get memory map: %#lx", Status);
		return;
	}

	InitializeMemoryEntries(MemoryMap, NumberOfEntries, DescriptorSize);
}

VOID InitializeMemoryNoBS()
{
	EFI_MEMORY_DESCRIPTOR *MemoryMap = (EFI_MEMORY_DESCRIPTOR *)bInfo.EFI.MemoryMap.BaseAddress;
	UINTN NumberOfEntries = bInfo.EFI.MemoryMap.NumberOfEntries;
	UINTN DescriptorSize = bInfo.EFI.MemoryMap.DescriptorSize;
	InitializeMemoryEntries(MemoryMap, NumberOfEntries, DescriptorSize);
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	if (bInfo.EFI.Info.ST == 1)
	{
		SearchSMBIOS(SystemTable);
		SearchRSDP(SystemTable);
	}

	if (bInfo.EFI.Info.BS == 0 && bInfo.EFI.Info.MemoryMap == 1)
		InitializeMemoryNoBS();
	else if (bInfo.EFI.Info.BS == 1)
		InitializeMemory(SystemTable);

	return EFI_SUCCESS;
}
