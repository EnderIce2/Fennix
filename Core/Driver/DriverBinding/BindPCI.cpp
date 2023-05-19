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
#include "../../../DAPI.hpp"
#include "../../../Fex.hpp"

namespace Driver
{
	DriverCode Driver::DriverLoadBindPCI(uintptr_t DriverAddress, size_t Size, bool IsBuiltIn)
	{
		FexExtended *DrvExtHdr = (FexExtended *)(DriverAddress + EXTENDED_SECTION_ADDRESS);
		if (IsBuiltIn)
			DrvExtHdr = (FexExtended *)(((BuiltInDriverInfo *)DriverAddress)->ExtendedHeader);

		uint16_t SizeOfVendorID = sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID) /
								  sizeof(DrvExtHdr->Driver.Bind.PCI.VendorID[0]);
		uint16_t SizeOfDeviceID = sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID) /
								  sizeof(DrvExtHdr->Driver.Bind.PCI.DeviceID[0]);

		for (uint16_t vID = 0; vID < SizeOfVendorID; vID++)
		{
			for (uint16_t dID = 0; dID < SizeOfDeviceID; dID++)
			{
				if (DrvExtHdr->Driver.Bind.PCI.VendorID[vID] == 0 ||
					DrvExtHdr->Driver.Bind.PCI.DeviceID[dID] == 0)
					continue;

				std::vector<PCI::PCIDeviceHeader *> devices =
					PCIManager->FindPCIDevice(DrvExtHdr->Driver.Bind.PCI.VendorID[vID],
											  DrvExtHdr->Driver.Bind.PCI.DeviceID[dID]);
				if (devices.size() == 0)
					continue;

				foreach (auto PCIDevice in devices)
				{
					debug("[%ld] VendorID: %#x; DeviceID: %#x",
						  devices.size(), PCIDevice->VendorID, PCIDevice->DeviceID);

					Memory::MemMgr *mem = new Memory::MemMgr(nullptr, TaskManager->GetCurrentProcess()->memDirectory);

					BuiltInDriverInfo *bidi = (BuiltInDriverInfo *)DriverAddress;
					Fex *fex = nullptr;
					if (!IsBuiltIn)
					{
						fex = (Fex *)mem->RequestPages(TO_PAGES(Size + 1));
						memcpy(fex, (void *)DriverAddress, Size);
						debug("Driver allocated at %#lx-%#lx", fex, (uintptr_t)fex + Size);
					}
					else
						fex = (Fex *)bidi->EntryPoint;
					DriverCode ret = CallDriverEntryPoint(fex, IsBuiltIn);
					if (ret != DriverCode::OK)
					{
						delete mem;
						return ret;
					}

					if (IsBuiltIn)
						fex = 0x0; /* Addresses are absolute if built-in. */

					FexExtended *fexE = IsBuiltIn ? (FexExtended *)bidi->ExtendedHeader : (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
					debug("Starting driver %s", fexE->Driver.Name);

					PCIManager->MapPCIAddresses(PCIDevice);

					switch (fexE->Driver.Type)
					{
					case FexDriverType::FexDriverType_Generic:
					case FexDriverType::FexDriverType_Display:
					case FexDriverType::FexDriverType_Network:
					case FexDriverType::FexDriverType_Storage:
					case FexDriverType::FexDriverType_FileSystem:
					case FexDriverType::FexDriverType_Input:
					case FexDriverType::FexDriverType_Audio:
					{
						FexExtended *DriverExtendedHeader = (FexExtended *)mem->RequestPages(TO_PAGES(sizeof(FexExtended) + 1));
						memcpy(DriverExtendedHeader, fexE, sizeof(FexExtended));

						DriverFile DrvFile = {
							.Enabled = true,
							.BuiltIn = IsBuiltIn,
							.DriverUID = this->DriverUIDs - 1,
							.Address = (void *)fex,
							.ExtendedHeaderAddress = (void *)DriverExtendedHeader,
							.InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexE->Driver.InterruptCallback),
							.MemTrk = mem,
						};

						if (fexE->Driver.InterruptCallback)
							DrvFile.InterruptHook[0] = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)PCIDevice)->InterruptLine), DrvFile);

						KernelCallback KCallback{};
						KCallback.RawPtr = PCIDevice;
						KCallback.Reason = CallbackReason::ConfigurationReason;
						DriverCode CallbackRet = ((DriverCode(*)(KernelCallback *))((uintptr_t)fexE->Driver.Callback + (uintptr_t)fex))(&KCallback);

						if (CallbackRet != DriverCode::OK)
						{
							error("Driver %s returned error %d", fexE->Driver.Name, CallbackRet);
							delete mem;
							return CallbackRet;
						}

						Drivers.push_back(DrvFile);
						return DriverCode::OK;
					}
					default:
					{
						warn("Unknown driver type: %d", fexE->Driver.Type);
						delete mem;
						return DriverCode::UNKNOWN_DRIVER_TYPE;
					}
					}
				}
			}
		}
		return DriverCode::PCI_DEVICE_NOT_FOUND;
	}
}
