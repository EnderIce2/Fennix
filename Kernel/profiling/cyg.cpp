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

#include <types.h>
#include <printf.h>
#include <uart.hpp>

#include "../kernel.h"

bool EnableProfiler = false;
bool Wait = false;
unsigned long long LogDepth = 0;
unsigned int Level = 0;
using namespace UniversalAsynchronousReceiverTransmitter;
UART com2(COM2);

static inline nsa NIF void profiler_uart_wrapper(char c, void *unused)
{
	bool renable = EnableProfiler;
	EnableProfiler = false;
	com2.Write(c);
	UNUSED(unused);
	if (renable)
		EnableProfiler = true;
}

EXTERNC nsa NIF void __cyg_profile_func_enter(void *Function, void *CallSite)
{
	if (!EnableProfiler)
		return;

	while (Wait)
#if defined(__amd64__) || defined(__i386__)
		asmv("pause");
#elif defined(__aarch64__)
		asmv("yield");
#endif
	Wait = true;

	if (Level > 40)
		Level--;

	Level++;

	if (!KernelSymbolTable)
		fctprintf(profiler_uart_wrapper, nullptr, "%lld [%02d]: \033[42m->\033[0m%*c \033[33m%p\033[0m  -  \033[33m%p\033[0m\n",
				  LogDepth++,
				  Level - 1,
				  Level,
				  ' ',
				  Function,
				  CallSite);
	else
		fctprintf(profiler_uart_wrapper, nullptr, "%lld [%02d]: \033[42m->\033[0m%*c \033[33m%s\033[0m  -  \033[33m%s\033[0m\n",
				  LogDepth++,
				  Level - 1,
				  Level,
				  ' ',
				  KernelSymbolTable->GetSymbol((uintptr_t)Function),
				  KernelSymbolTable->GetSymbol((uintptr_t)CallSite));
	Wait = false;
}

EXTERNC nsa NIF void __cyg_profile_func_exit(void *Function, void *CallSite)
{
	if (!EnableProfiler)
		return;

	while (Wait)
#if defined(__amd64__) || defined(__i386__)
		asmv("pause");
#elif defined(__aarch64__)
		asmv("yield");
#endif
	Wait = true;

	if (Level > 40)
		Level--;

	Level--;

	if (!KernelSymbolTable)
		fctprintf(profiler_uart_wrapper, nullptr, "%lld [%02d]: \033[41m<-\033[0m%*c \033[33m%p\033[0m  -  \033[33m%p\033[0m\n",
				  LogDepth++,
				  Level - 1,
				  Level,
				  ' ',
				  Function,
				  CallSite);
	else
		fctprintf(profiler_uart_wrapper, nullptr, "%lld [%02d]: \033[41m<-\033[0m%*c \033[33m%s\033[0m  -  \033[33m%s\033[0m\n",
				  LogDepth++,
				  Level - 1,
				  Level,
				  ' ',
				  KernelSymbolTable->GetSymbol((uintptr_t)Function),
				  KernelSymbolTable->GetSymbol((uintptr_t)CallSite));
	Wait = false;
}
