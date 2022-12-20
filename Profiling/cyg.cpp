#include <types.h>
#include <printf.h>
#include <uart.hpp>

#include "../kernel.h"

bool EnableProfiler = false;
bool Wait = false;
unsigned long long LogDepth = 0;
unsigned int Level = 0;
using namespace UniversalAsynchronousReceiverTransmitter;

static inline SafeFunction __no_instrument_function void profiler_uart_wrapper(char c, void *unused)
{
    bool renable = EnableProfiler;
    EnableProfiler = false;
    UART(COM2).Write(c);
    (void)unused;
    if (renable)
        EnableProfiler = true;
}

EXTERNC SafeFunction __no_instrument_function void __cyg_profile_func_enter(void *Function, void *CallSite)
{
    if (!EnableProfiler)
        return;

    while (Wait)
        asmv("pause");
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
                  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)Function),
                  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)CallSite));
    Wait = false;
}

EXTERNC SafeFunction __no_instrument_function void __cyg_profile_func_exit(void *Function, void *CallSite)
{
    if (!EnableProfiler)
        return;

    while (Wait)
        asmv("pause");
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
                  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)Function),
                  KernelSymbolTable->GetSymbolFromAddress((uintptr_t)CallSite));
    Wait = false;
}
