#include "../../crashhandler.hpp"
#include "../chfcts.hpp"

#include <interrupts.hpp>
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
    SafeFunction void DisplayStackFrameScreen(CRData data)
    {
        EHPrint("\eFAFAFATracing 40 frames...\n");
        TraceFrames(data.Frame, 40);
        EHPrint("\n\n\eFAFAFATracing interrupt frames...\n");
        for (uint64_t i = 0; i < 8; i++)
        {
            if (EHIntFrames[i])
            {
                if (!Memory::Virtual().Check(EHIntFrames[i]))
                    continue;
                EHPrint("\n\e2565CC%p", EHIntFrames[i]);
                EHPrint("\e7925CC-");
#if defined(__amd64__)
                if ((uint64_t)EHIntFrames[i] >= 0xFFFFFFFF80000000 && (uint64_t)EHIntFrames[i] <= (uint64_t)&_kernel_end)
#elif defined(__i386__)
                if ((uint64_t)EHIntFrames[i] >= 0xC0000000 && (uint64_t)EHIntFrames[i] <= (uint64_t)&_kernel_end)
#elif defined(__aarch64__)
#endif
                    EHPrint("\e25CCC9%s", KernelSymbolTable->GetSymbolFromAddress((uint64_t)EHIntFrames[i]));
                else
                    EHPrint("\eFF4CA9Outside Kernel");
            }
        }
    }
}
