#include <types.h>
#include <printf.h>
#include <uart.hpp>

#include "../kernel.h"

using namespace UniversalAsynchronousReceiverTransmitter;

#if BITS_PER_LONG >= 64
typedef long gcov_type;
#else
typedef long long gcov_type;
#endif

struct gcov_fn_info
{
    unsigned int ident;
    unsigned int checksum;
    unsigned int n_ctrs[0];
};

struct gcov_ctr_info
{
    unsigned int num;
    gcov_type *values;
    void (*merge)(gcov_type *, unsigned int);
};

struct gcov_info
{
    unsigned int version;
    struct gcov_info *next;
    unsigned int stamp;
    const char *filename;
    unsigned int n_functions;
    const struct gcov_fn_info *functions;
    unsigned int ctr_mask;
    struct gcov_ctr_info counts[0];
};

static inline SafeFunction __no_instrument_function void gcov_uart_wrapper(char c, void *unused)
{
    UART(COM2).Write(c);
    (void)unused;
}

// TODO: Implement

EXTERNC SafeFunction __no_instrument_function void __gcov_init(gcov_info *p __unused)
{
}

EXTERNC SafeFunction __no_instrument_function void __gcov_exit(void)
{
}

EXTERNC SafeFunction __no_instrument_function void __gcov_flush(void)
{
}

EXTERNC SafeFunction __no_instrument_function void __gcov_merge_add(gcov_type *counters, unsigned int n_counters)
{
}

EXTERNC SafeFunction __no_instrument_function void __gcov_merge_single(gcov_type *counters, unsigned int n_counters)
{
}
