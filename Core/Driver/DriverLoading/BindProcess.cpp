#include "../api.hpp"

#include <interrupts.hpp>
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
    DriverCode Driver::DriverLoadBindProcess(void *DrvExtHdr, uintptr_t DriverAddress, size_t Size, bool IsElf)
    {
        fixme("Process driver: %s", ((FexExtended *)DrvExtHdr)->Driver.Name);
        UNUSED(Size);
        UNUSED(DriverAddress);
        UNUSED(IsElf);
        return DriverCode::NOT_IMPLEMENTED;
    }
}
