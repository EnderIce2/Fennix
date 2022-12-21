#include <types.h>
#include <printf.h>
#include <uart.hpp>

#include "../kernel.h"

using namespace UniversalAsynchronousReceiverTransmitter;

static inline SafeFunction __no_instrument_function void gprof_uart_wrapper(char c, void *unused)
{
    UART(COM2).Write(c);
    UNUSED(unused);
}

EXTERNC SafeFunction __no_instrument_function void mcount(unsigned long frompc, unsigned long selfpc)
{
    // TODO: Implement
    /* https://docs.kernel.org/trace/ftrace-design.html */
    UNUSED(frompc);
    UNUSED(selfpc);
}
