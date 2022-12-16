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
    DriverCode Driver::DriverLoadBindProcess(void *DrvExtHdr, uint64_t DriverAddress, uint64_t Size, bool IsElf)
    {
        fixme("Process driver: %s", ((FexExtended *)DrvExtHdr)->Driver.Name);
        return DriverCode::NOT_IMPLEMENTED;
    }
}
