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
        uintptr_t rip;
    };

    SafeFunction void TraceFrames(CHArchTrapFrame *Frame, int Count, SymbolResolver::Symbols *SymHandle, bool Kernel)
    {
        if (!Memory::Virtual().Check(Frame))
        {
            EHPrint("Invalid frame pointer: %p\n", Frame);
            return;
        }

#if defined(__amd64__)
        struct StackFrame *frames = (struct StackFrame *)Frame->rbp; // (struct StackFrame *)__builtin_frame_address(0);
        if (!Memory::Virtual().Check((void *)Frame->rbp))
#elif defined(__i386__)
        struct StackFrame *frames = (struct StackFrame *)Frame->ebp; // (struct StackFrame *)__builtin_frame_address(0);
        if (!Memory::Virtual().Check((void *)Frame->ebp))
#elif defined(__aarch64__)
#endif
        {
            EHPrint("Invalid rbp pointer: %p\n", Frame->rbp);
            return;
        }

        if (!Memory::Virtual().Check(SymHandle))
        {
            EHPrint("Invalid symbol handle: %p\n", SymHandle);
            return;
        }

        debug("\nStack tracing... %p %d %p %d", Frame, Count, frames, Kernel);
        EHPrint("\e7981FC\nStack Trace:\n");
        if (!frames || !frames->rip || !frames->rbp)
        {
#if defined(__amd64__)
            EHPrint("\e2565CC%p", (void *)Frame->rip);
#elif defined(__i386__)
            EHPrint("\e2565CC%p", (void *)Frame->eip);
#elif defined(__aarch64__)
#endif
            EHPrint("\e7925CC-");
#if defined(__amd64__)
            EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(Frame->rip));
#elif defined(__i386__)
            EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(Frame->eip));
#elif defined(__aarch64__)
#endif
            EHPrint("\e7981FC <- Exception");
            EHPrint("\eFF0000\n< No stack trace available. >\n");
        }
        else
        {
#if defined(__amd64__)
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            if ((Frame->rip >= 0xFFFFFFFF80000000 && Frame->rip <= (uintptr_t)&_kernel_end) || !Kernel)
                EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(Frame->rip));
            else
                EHPrint("Outside Kernel");
#elif defined(__i386__)
            EHPrint("\e2565CC%p", (void *)Frame->eip);
            EHPrint("\e7925CC-");
            if ((Frame->eip >= 0xC0000000 && Frame->eip <= (uintptr_t)&_kernel_end) || !Kernel)
                EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(Frame->eip));
            else
                EHPrint("Outside Kernel");
#elif defined(__aarch64__)
#endif
            EHPrint("\e7981FC <- Exception");
            for (int frame = 0; frame < Count; ++frame)
            {
                if (!frames->rip)
                    break;
                EHPrint("\n\e2565CC%p", (void *)frames->rip);
                EHPrint("\e7925CC-");
#if defined(__amd64__)
                if ((frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uintptr_t)&_kernel_end) || !Kernel)
#elif defined(__i386__)
                if ((frames->rip >= 0xC0000000 && frames->rip <= (uintptr_t)&_kernel_end) || !Kernel)
#elif defined(__aarch64__)
#endif
                    EHPrint("\e25CCC9%s", SymHandle->GetSymbolFromAddress(frames->rip));
                else
                    EHPrint("\eFF4CA9Outside Kernel");

                if (!Memory::Virtual().Check(frames->rbp))
                    return;
                frames = frames->rbp;
            }
        }
        EHPrint("\n");
    }
}
