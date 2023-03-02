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
    DriverCode Driver::BindProcessGeneric(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessDisplay(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessNetwork(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessStorage(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessFileSystem(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessInput(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::BindProcessAudio(Memory::MemMgr *mem, void *fex)
    {
        return DriverCode::NOT_IMPLEMENTED;
    }

    DriverCode Driver::DriverLoadBindProcess(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        fixme("Process driver: %s", ((FexExtended *)DrvExtHdr)->Driver.Name);
        UNUSED(Size);
        UNUSED(DriverAddress);
        UNUSED(IsElf);
        return DriverCode::NOT_IMPLEMENTED;
    }
}
