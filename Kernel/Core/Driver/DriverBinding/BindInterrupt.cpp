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
    DriverCode Driver::DriverLoadBindInterrupt(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
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
        case FexDriverType::FexDriverType_Generic:
            return BindInterruptGeneric(mem, fex);
        case FexDriverType::FexDriverType_Display:
            return BindInterruptDisplay(mem, fex);
        case FexDriverType::FexDriverType_Network:
            return BindInterruptNetwork(mem, fex);
        case FexDriverType::FexDriverType_Storage:
            return BindInterruptStorage(mem, fex);
        case FexDriverType::FexDriverType_FileSystem:
            return BindInterruptFileSystem(mem, fex);
        case FexDriverType::FexDriverType_Input:
            return BindInterruptInput(mem, fex);
        case FexDriverType::FexDriverType_Audio:
            return BindInterruptAudio(mem, fex);
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
