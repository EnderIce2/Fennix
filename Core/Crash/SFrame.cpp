#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../../kernel.h"

namespace CrashHandler
{
    struct StackFrame
    {
        struct StackFrame *rbp;
        uint64_t rip;
    };

    __no_stack_protector void TraceFrames(CHArchTrapFrame *Frame, int Count)
    {
        struct StackFrame *frames = (struct StackFrame *)Frame->rbp; // (struct StackFrame *)__builtin_frame_address(0);
        debug("Stack tracing...");
        EHPrint("\e7981FC\nStack Trace:\n");
        if (!frames || !frames->rip || !frames->rbp)
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            EHPrint("\eAA25CC%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            EHPrint("\e7981FC <- Exception");
            EHPrint("\eFF0000\n< No stack trace available. >\n");
        }
        else
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            if (Frame->rip >= 0xFFFFFFFF80000000 && Frame->rip <= (uint64_t)&_kernel_end)
                EHPrint("\eAA25CC%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            else
                EHPrint("Outside Kernel");
            EHPrint("\e7981FC <- Exception");
            for (int frame = 0; frame < Count; ++frame)
            {
                if (!frames->rip)
                    break;
                EHPrint("\n\e2565CC%p", (void *)frames->rip);
                EHPrint("\e7925CC-");
                if (frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uint64_t)&_kernel_end)
                    EHPrint("\e25CCC9%s", KernelSymbolTable->GetSymbolFromAddress(frames->rip));
                else
                    EHPrint("\eFF4CA9Outside Kernel");

                if (!Memory::Virtual().Check(frames->rbp))
                    return;
                frames = frames->rbp;
            }
        }
    }
}
