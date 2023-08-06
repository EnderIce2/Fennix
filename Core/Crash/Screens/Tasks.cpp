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
    SafeFunction void DisplayTasksScreen(CRData data)
    {
        const char *StatusColor[7] = {
            "FF0000", // Unknown
            "AAFF00", // Ready
            "00AA00", // Running
            "FFAA00", // Sleeping
            "FFAA00", // Blocked
            "FF0088", // Zombie
            "FF0000", // Terminated
        };

        const char *StatusString[7] = {
            "Unknown",    // Unknown
            "Ready",      // Ready
            "Running",    // Running
            "Sleeping",   // Sleeping
            "Blocked",    // Blocked
            "Zombie",     // Zombie
            "Terminated", // Terminated
        };

        if (TaskManager)
        {
            std::vector<Tasking::PCB *> Plist = TaskManager->GetProcessList();

            if (data.Thread)
#if defined(a64)
                EHPrint("\eFAFAFACrash occurred in thread \eAA0F0F%s\eFAFAFA(%ld) at \e00AAAA%#lx\n",
                        data.Thread->Name, data.Thread->ID, data.Frame->rip);
#elif defined(a32)
                EHPrint("\eFAFAFACrash occurred in thread \eAA0F0F%s\eFAFAFA(%ld) at \e00AAAA%#lx\n",
                        data.Thread->Name, data.Thread->ID, data.Frame->eip);
#elif defined(aa64)
#endif

            EHPrint("\eFAFAFAProcess list (%ld):\n", Plist.size());
            foreach (auto Process in Plist)
            {
                EHPrint("\e%s-> \eFAFAFA%s\eCCCCCC(%ld) \e00AAAA%s\eFAFAFA PT:\e00AAAA%#lx\n",
                        StatusColor[Process->Status.load()], Process->Name,
                        Process->ID, StatusString[Process->Status.load()],
                        Process->PageTable);

                foreach (auto Thread in Process->Threads)
                    EHPrint("\e%s  -> \eFAFAFA%s\eCCCCCC(%ld) \e00AAAA%s\eFAFAFA Stack:\e00AAAA%#lx\n",
                            StatusColor[Thread->Status.load()], Thread->Name,
                            Thread->ID, StatusString[Thread->Status.load()],
                            Thread->Stack);
            }
        }
        else
            EHPrint("\eFAFAFATaskManager is not initialized!\n");
    }
}
