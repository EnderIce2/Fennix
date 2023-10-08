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

#include "../api.hpp"

#include <ints.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../../kernel.h"
#include "../../../mapi.hpp"
#include "../../../Fex.hpp"

namespace Module
{
	ModuleCode Module::ModuleLoadBindPCI(uintptr_t ModuleAddress, size_t Size, bool IsBuiltIn)
	{
		FexExtended *DrvExtHdr = (FexExtended *)(ModuleAddress + EXTENDED_SECTION_ADDRESS);
		if (IsBuiltIn)
			DrvExtHdr = (FexExtended *)(((BuiltInModuleInfo *)ModuleAddress)->ExtendedHeader);

		uint16_t SizeOfVendorID = sizeof(DrvExtHdr->Module.Bind.PCI.VendorID) /
								  sizeof(DrvExtHdr->Module.Bind.PCI.VendorID[0]);
		uint16_t SizeOfDeviceID = sizeof(DrvExtHdr->Module.Bind.PCI.DeviceID) /
								  sizeof(DrvExtHdr->Module.Bind.PCI.DeviceID[0]);

		for (uint16_t vID = 0; vID < SizeOfVendorID; vID++)
		{
			for (uint16_t dID = 0; dID < SizeOfDeviceID; dID++)
			{
				if (DrvExtHdr->Module.Bind.PCI.VendorID[vID] == 0 ||
					DrvExtHdr->Module.Bind.PCI.DeviceID[dID] == 0)
					continue;

				std::vector<PCI::PCIDevice> devices =
					PCIManager->FindPCIDevice(DrvExtHdr->Module.Bind.PCI.VendorID[vID],
											  DrvExtHdr->Module.Bind.PCI.DeviceID[dID]);
				if (devices.size() == 0)
					continue;

				foreach (auto Device in devices)
				{
					debug("[%ld] VendorID: %#x; DeviceID: %#x",
						  devices.size(), Device.Header->VendorID,
						  Device.Header->DeviceID);

					Memory::VirtualMemoryArea *vma = new Memory::VirtualMemoryArea(nullptr);

					BuiltInModuleInfo *bidi = (BuiltInModuleInfo *)ModuleAddress;
					Fex *fex = nullptr;
					if (!IsBuiltIn)
					{
						fex = (Fex *)vma->RequestPages(TO_PAGES(Size + 1));
						memcpy(fex, (void *)ModuleAddress, Size);
						debug("Module allocated at %#lx-%#lx", fex, (uintptr_t)fex + Size);
					}
					else
						fex = (Fex *)bidi->EntryPoint;
					ModuleCode ret = CallModuleEntryPoint(fex, IsBuiltIn);
					if (ret != ModuleCode::OK)
					{
						delete vma;
						return ret;
					}

					if (IsBuiltIn)
						fex = 0x0; /* Addresses are absolute if built-in. */

					FexExtended *fexE = IsBuiltIn ? (FexExtended *)bidi->ExtendedHeader : (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
					debug("Starting driver %s", fexE->Module.Name);

					PCIManager->MapPCIAddresses(Device);

					switch (fexE->Module.Type)
					{
					case FexModuleType::FexModuleType_Generic:
					case FexModuleType::FexModuleType_Display:
					case FexModuleType::FexModuleType_Network:
					case FexModuleType::FexModuleType_Storage:
					case FexModuleType::FexModuleType_FileSystem:
					case FexModuleType::FexModuleType_Input:
					case FexModuleType::FexModuleType_Audio:
					{
						FexExtended *DriverExtendedHeader = (FexExtended *)vma->RequestPages(TO_PAGES(sizeof(FexExtended) + 1));
						memcpy(DriverExtendedHeader, fexE, sizeof(FexExtended));

						ModuleFile DrvFile = {
							.Enabled = true,
							.BuiltIn = IsBuiltIn,
							.modUniqueID = this->modUIDs - 1,
							.Address = (void *)fex,
							.ExtendedHeaderAddress = (void *)DriverExtendedHeader,
							.InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexE->Module.InterruptCallback),
							.vma = vma,
						};

						if (fexE->Module.InterruptCallback)
							DrvFile.InterruptHook[0] = new ModuleInterruptHook(((int)((PCI::PCIHeader0 *)Device.Header)->InterruptLine), DrvFile);

						KernelCallback KCallback{};
						KCallback.RawPtr = Device.Header;
						KCallback.Reason = CallbackReason::ConfigurationReason;
						ModuleCode CallbackRet = ((ModuleCode(*)(KernelCallback *))((uintptr_t)fexE->Module.Callback + (uintptr_t)fex))(&KCallback);

						if (CallbackRet != ModuleCode::OK)
						{
							error("Module %s returned error %d", fexE->Module.Name, CallbackRet);
							delete vma;
							return CallbackRet;
						}

						Modules.push_back(DrvFile);
						return ModuleCode::OK;
					}
					default:
					{
						warn("Unknown driver type: %d", fexE->Module.Type);
						delete vma;
						return ModuleCode::UNKNOWN_MODULE_TYPE;
					}
					}
				}
			}
		}
		return ModuleCode::PCI_DEVICE_NOT_FOUND;
	}
}
