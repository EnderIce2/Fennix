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
    DriverCode Driver::BindInputInput(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
        KernelCallback KCallback{};

        fixme("Input driver: %s", fexExtended->Driver.Name);
        KCallback.RawPtr = nullptr;
        KCallback.Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(&KCallback);
        if (CallbackRet == DriverReturnCode::NOT_IMPLEMENTED)
        {
            delete mem, mem = nullptr;
            error("Driver %s is not implemented", fexExtended->Driver.Name);
            return DriverCode::NOT_IMPLEMENTED;
        }
        else if (CallbackRet != DriverReturnCode::OK)
        {
            delete mem, mem = nullptr;
            error("Driver %s returned error %d", fexExtended->Driver.Name, CallbackRet);
            return DriverCode::DRIVER_RETURNED_ERROR;
        }

        fixme("Input driver: %s", fexExtended->Driver.Name);

        DriverFile DrvFile = {
            .Enabled = true,
            .DriverUID = this->DriverUIDs - 1,
            .Address = (void *)fex,
            .MemTrk = mem,
        };
        Drivers.push_back(DrvFile);
        return DriverCode::OK;
    }
}
