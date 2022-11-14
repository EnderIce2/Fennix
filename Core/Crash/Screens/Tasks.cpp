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
    __no_stack_protector void DisplayTasksScreen(CRData data)
    {
        const char *StatusColor[7] = {
            "FF0000", // Unknown
            "AAFF00", // Ready
            "00AA00", // Running
            "FFAA00", // Sleeping
            "FFAA00", // Waiting
            "FF0088", // Stopped
            "FF0000", // Terminated
        };

        const char *StatusString[7] = {
            "Unknown",    // Unknown
            "Ready",      // Ready
            "Running",    // Running
            "Sleeping",   // Sleeping
            "Waiting",    // Waiting
            "Stopped",    // Stopped
            "Terminated", // Terminated
        };

        Vector<Tasking::PCB *> Plist = TaskManager->GetProcessList();

        if (TaskManager)
        {
            if (data.Thread)
#if defined(__amd64__)
                EHPrint("\eFAFAFACrash occured in thread \eAA0F0F%s\eFAFAFA(%ld) at \e00AAAA%#lx\n", data.Thread->Name, data.Thread->ID, data.Frame->rip);
#elif defined(__i386__)
                EHPrint("\eFAFAFACrash occured in thread \eAA0F0F%s\eFAFAFA(%ld) at \e00AAAA%#lx\n", data.Thread->Name, data.Thread->ID, data.Frame->eip);
#elif defined(__aarch64__)
#endif

            EHPrint("\eFAFAFAProcess list (%ld):\n", Plist.size());
            foreach (auto Process in Plist)
            {
                EHPrint("\e%s-> \eFAFAFA%s\eCCCCCC(%ld) \e00AAAA%s\eFAFAFA PT:\e00AAAA%#lx\n",
                        StatusColor[Process->Status], Process->Name, Process->ID, StatusString[Process->Status],
                        Process->PageTable);

                foreach (auto Thread in Process->Threads)
                    EHPrint("\e%s  -> \eFAFAFA%s\eCCCCCC(%ld) \e00AAAA%s\eFAFAFA Stack:\e00AAAA%#lx\n",
                            StatusColor[Thread->Status], Thread->Name, Thread->ID, StatusString[Thread->Status],
                            Thread->Stack);
            }
        }
        else
            EHPrint("\eFAFAFATaskManager is not initialized!\n");
    }
}
