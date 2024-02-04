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

#include <ints.hpp>
#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../../kernel.h"

namespace CrashHandler
{
	nsa void DisplayStackFrameScreen(CRData data)
	{
		EHPrint("\eFAFAFATracing 10 frames...");
		TraceFrames(data, 10, KernelSymbolTable, true);
		if (data.Process)
		{
			EHPrint("\n\eFAFAFATracing 10 process frames...");
			SymbolResolver::Symbols *pSt = data.Process->ELFSymbolTable;
			debug("pSt = %#lx", pSt);
			if (!pSt || !pSt->SymTableExists)
				EHPrint("\n\eFF0000< No symbol table available. >\n");
			else
				TraceFrames(data, 10, pSt, false);
		}
		EHPrint("\n\eFAFAFATracing interrupt frames...");
		for (short i = 0; i < 8; i++)
		{
			if (EHIntFrames[i])
			{
				if (!Memory::Virtual().Check(EHIntFrames[i]))
					continue;
				EHPrint("\n\e2565CC%p", EHIntFrames[i]);
				EHPrint("\e7925CC-");
#if defined(a64)
				if ((uintptr_t)EHIntFrames[i] >= 0xFFFFFFFF80000000 &&
					(uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#elif defined(a32)
				if ((uintptr_t)EHIntFrames[i] >= 0xC0000000 &&
					(uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#elif defined(aa64)
				if ((uintptr_t)EHIntFrames[i] >= 0xFFFFFFFF80000000 &&
					(uintptr_t)EHIntFrames[i] <= (uintptr_t)&_kernel_end)
#endif
					EHPrint("\e25CCC9%s",
							KernelSymbolTable->GetSymbol((uintptr_t)EHIntFrames[i]));
				else
					EHPrint("\eFF4CA9Outside Kernel");
			}
		}
	}
}
