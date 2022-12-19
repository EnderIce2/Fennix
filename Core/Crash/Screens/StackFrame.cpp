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
        EHPrint("\eFAFAFATracing 40 frames...");
        TraceFrames(data.Frame, 40, KernelSymbolTable, true);
        if (data.Process)
        {
            EHPrint("\n\eFAFAFATracing 40 process frames...");
            SymbolResolver::Symbols *sh = data.Process->ELFSymbolTable;
            if (!sh)
                EHPrint("\n\eFF0000< No symbol table available. >\n");
            else
                TraceFrames(data.Frame, 40, sh, false);
        }
        EHPrint("\n\eFAFAFATracing interrupt frames...");
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
        if (data.Process && data.Thread)
        {
            EHPrint("\n\n\eFAFAFATracing thread RIP history...");
            SymbolResolver::Symbols *sh = data.Process->ELFSymbolTable;
            if (!sh)
                EHPrint("\n\eFFA500Warning: No symbol table available.");
            int SameItr = 0;
            uint64_t LastRIP = 0;
            for (int i = 0; i < 128; i++)
            {
                if (data.Thread->RIPHistory[i] == 0)
                    break;
                if (data.Thread->RIPHistory[i] == LastRIP)
                {
                    SameItr++;
                    if (SameItr > 3)
                        continue;
                }
                else
                    SameItr = 0;
                LastRIP = data.Thread->RIPHistory[i];
                if (!sh)
                    EHPrint("\n\e2565CC%p", data.Thread->RIPHistory[i]);
                else
                    EHPrint("\n\e2565CC%p\e7925CC-\e25CCC9%s", data.Thread->RIPHistory[i], sh->GetSymbolFromAddress((uint64_t)data.Thread->RIPHistory[i]));
            }
            EHPrint("\n\e7925CCNote: \e2565CCSame RIPs are not shown more than 3 times.\n");
        }
    }
}
