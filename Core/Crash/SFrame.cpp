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

#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

namespace CrashHandler
{
    struct StackFrame
    {
        struct StackFrame *rbp;
        uintptr_t rip;
    };

    SafeFunction void TraceFrames(CRData data, int Count, SymbolResolver::Symbols *SymHandle, bool Kernel)
    {
        if (!Memory::Virtual().Check(data.Frame))
        {
            EHPrint("Invalid frame pointer: %p\n", data.Frame);
            return;
        }

        if (!Memory::Virtual().Check(SymHandle))
        {
            EHPrint("Invalid symbol handle: %p\n", SymHandle);
            return;
        }

        bool TriedRetryBP = false;
        struct StackFrame *frames = nullptr;
    RetryBP:
#if defined(a64)
        if (TriedRetryBP == false)
            frames = (struct StackFrame *)data.Frame->rbp;
#elif defined(a32)
        if (TriedRetryBP == false)
            frames = (struct StackFrame *)data.Frame->ebp;
#elif defined(aa64)
#endif
        if (!Memory::Virtual().Check((void *)frames))
        {
            if (TriedRetryBP == false)
            {
                frames = (struct StackFrame *)Memory::Virtual(data.Process->PageTable).GetPhysical((void *)frames);
                TriedRetryBP = true;
                goto RetryBP;
            }
#if defined(a64)
            EHPrint("Invalid rbp pointer: %p\n", data.Frame->rbp);
#elif defined(a32)
            EHPrint("Invalid ebp pointer: %p\n", data.Frame->ebp);
#elif defined(aa64)
#endif
            return;
        }

        debug("\nStack tracing... %p %d %p %d", data.Frame, Count, frames, Kernel);
        EHPrint("\e7981FC\nStack Trace:\n");
        if (!frames || !frames->rip || !frames->rbp)
        {
#if defined(a64)
            EHPrint("\e2565CC%p", (void *)data.Frame->rip);
#elif defined(a32)
            EHPrint("\e2565CC%p", (void *)data.Frame->eip);
#elif defined(aa64)
#endif
            EHPrint("\e7925CC-");
#if defined(a64)
            EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(data.Frame->rip));
#elif defined(a32)
            EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(data.Frame->eip));
#elif defined(aa64)
#endif
            EHPrint("\e7981FC <- Exception");
            EHPrint("\eFF0000\n< No stack trace available. >\n");
        }
        else
        {
#if defined(a64)
            EHPrint("\e2565CC%p", (void *)data.Frame->rip);
            EHPrint("\e7925CC-");
            if ((data.Frame->rip >= 0xFFFFFFFF80000000 && data.Frame->rip <= (uintptr_t)&_kernel_end) || !Kernel)
                EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(data.Frame->rip));
            else
                EHPrint("Outside Kernel");
#elif defined(a32)
            EHPrint("\e2565CC%p", (void *)data.Frame->eip);
            EHPrint("\e7925CC-");
            if ((data.Frame->eip >= 0xC0000000 && data.Frame->eip <= (uintptr_t)&_kernel_end) || !Kernel)
                EHPrint("\eAA25CC%s", SymHandle->GetSymbolFromAddress(data.Frame->eip));
            else
                EHPrint("Outside Kernel");
#elif defined(aa64)
#endif
            EHPrint("\e7981FC <- Exception");
            for (int frame = 0; frame < Count; ++frame)
            {
                if (!frames->rip)
                    break;
                EHPrint("\n\e2565CC%p", (void *)frames->rip);
                EHPrint("\e7925CC-");
#if defined(a64)
                if ((frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uintptr_t)&_kernel_end) || !Kernel)
#elif defined(a32)
                if ((frames->rip >= 0xC0000000 && frames->rip <= (uintptr_t)&_kernel_end) || !Kernel)
#elif defined(aa64)
                if ((frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uintptr_t)&_kernel_end) || !Kernel)
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
