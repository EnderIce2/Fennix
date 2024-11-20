#include "../../crashhandler.hpp"
#include "../chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../../../kernel.h"

namespace CrashHandler
{
    SafeFunction void DisplayConsoleScreen(CRData data)
    {
        EHPrint("TODO");
    }
}