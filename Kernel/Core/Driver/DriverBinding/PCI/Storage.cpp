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

#include "../../api.hpp"

#include <ints.hpp>
#include <memory.hpp>
#include <task.hpp>
#include <lock.hpp>
#include <printf.h>
#include <cwalk.h>
#include <md5.h>

#include "../../../../kernel.h"
#include "../../../../DAPI.hpp"
#include "../../../../Fex.hpp"

namespace Driver
{
    DriverCode Driver::BindPCIStorage(Memory::MemMgr *mem, void *fex, PCI::PCIDeviceHeader *PCIDevice)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);

        if (fexExtended->Driver.OverrideOnConflict)
        {
            std::vector<uint64_t> DriversToRemove = std::vector<uint64_t>();
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }

                DriversToRemove.push_back(Drv.DriverUID);
            }

            foreach (auto DrvID in DriversToRemove)
            {
                if (!this->UnloadDriver(DrvID))
                {
                    error("Failed to unload conflicting driver %d", DrvID);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }
        else
        {
            foreach (auto Drv in Drivers)
            {
                FexExtended *fe = ((FexExtended *)((uintptr_t)Drv.Address + EXTENDED_SECTION_ADDRESS));

                if (fe->Driver.OverrideOnConflict)
                {
                    debug("Driver %s is conflicting with %s", fe->Driver.Name, fexExtended->Driver.Name);
                    return DriverCode::DRIVER_CONFLICT;
                }
            }
        }

        DriverFile DrvFile = {
            .Enabled = true,
            .DriverUID = this->DriverUIDs - 1,
            .Address = (void *)fex,
            .InterruptCallback = (void *)((uintptr_t)fex + (uintptr_t)fexExtended->Driver.InterruptCallback),
            .MemTrk = mem,
        };
        if (fexExtended->Driver.InterruptCallback)
            DrvFile.InterruptHook[0] = new DriverInterruptHook(((int)((PCI::PCIHeader0 *)PCIDevice)->InterruptLine), DrvFile);

        KernelCallback KCallback{};
        KCallback.RawPtr = PCIDevice;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);

        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            delete mem, mem = nullptr;
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet == DriverReturnCode::OK)
            trace("Device found for driver: %s", fexExtended->Driver.Name);
        else
        {
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }
}
