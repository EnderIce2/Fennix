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
    DriverCode Driver::BindInputGeneric(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInputDisplay(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInputNetwork(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInputStorage(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInputFileSystem(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindInputInput(Memory::MemMgr *mem, void *fex)
    {
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
        KernelCallback *KCallback = (KernelCallback *)mem->RequestPages(TO_PAGES(sizeof(KernelCallback) + 1));

        fixme("Input driver: %s", fexExtended->Driver.Name);
        KCallback->RawPtr = nullptr;
        KCallback->Reason = CallbackReason::ConfigurationReason;
        int CallbackRet = ((int (*)(KernelCallback *))((uintptr_t)fexExtended->Driver.Callback + (uintptr_t)fex))(KCallback);
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

    DriverCode Driver::BindInputAudio(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::DriverLoadBindInput(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        UNUSED(DrvExtHdr);
        UNUSED(IsElf);
        Memory::MemMgr *mem = new Memory::MemMgr(nullptr, TaskManager->GetCurrentProcess()->memDirectory);
        Fex *fex = (Fex *)mem->RequestPages(TO_PAGES(Size + 1));
        memcpy(fex, (void *)DriverAddress, Size);
        FexExtended *fexExtended = (FexExtended *)((uintptr_t)fex + EXTENDED_SECTION_ADDRESS);
        debug("Driver allocated at %#lx-%#lx", fex, (uintptr_t)fex + Size);
#ifdef DEBUG
        uint8_t *result = md5File((uint8_t *)fex, Size);
        debug("MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
              result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],
              result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15]);
        kfree(result);
#endif
        KernelAPI *KAPI = (KernelAPI *)mem->RequestPages(TO_PAGES(sizeof(KernelAPI) + 1));

        if (CallDriverEntryPoint(fex, KAPI) != DriverCode::OK)
        {
            delete mem, mem = nullptr;
            return DriverCode::DRIVER_RETURNED_ERROR;
        }
        debug("Starting driver %s (offset: %#lx)", fexExtended->Driver.Name, fex);

        switch (fexExtended->Driver.Type)
        {
        case FexDriverType::FexDriverType_Input:
            return BindInputInput(mem, fex);
        default:
        {
            warn("Unknown driver type: %d", fexExtended->Driver.Type);
            delete mem, mem = nullptr;
            return DriverCode::UNKNOWN_DRIVER_TYPE;
        }
        }

        return DriverCode::OK;
    }
}
