#include "../../crashhandler.hpp"
#include "../chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../../kernel.h"

namespace CrashHandler
{
    SafeFunction void DisplayConsoleScreen(CRData data)
    {
        EHPrint("TODO");
        UNUSED(data);
    }
}