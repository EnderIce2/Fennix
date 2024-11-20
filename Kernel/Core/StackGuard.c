#include <types.h>
#include <debug.h>

#include "../kernel.h"

#ifndef STACK_CHK_GUARD_VALUE
#if UINTPTR_MAX == UINT32_MAX
#define STACK_CHK_GUARD_VALUE 0xDEAD57AC
#else
#define STACK_CHK_GUARD_VALUE 0xDEAD57AC00000000
#endif
#endif

__attribute__((weak)) uintptr_t __stack_chk_guard = 0;

__attribute__((weak, no_stack_protector)) uintptr_t __stack_chk_guard_init(void)
{
    return STACK_CHK_GUARD_VALUE;
}

extern __attribute__((constructor, no_stack_protector)) void __guard_setup(void)
{
    debug("StackGuard: __guard_setup");
    if (__stack_chk_guard == 0)
        __stack_chk_guard = __stack_chk_guard_init();
}

__attribute__((weak, noreturn, no_stack_protector)) void __stack_chk_fail(void)
{
    TaskingPanic();
    for (short i = 0; i < 10; i++)
        error("Stack smashing detected!");
    debug("%#lx", __stack_chk_guard);
    KPrint("\eFF0000Stack smashing detected!");
#if defined(__amd64__) || defined(__i386__)
    while (1)
        asmv("cli; hlt");
#elif defined(__aarch64__)
    asmv("wfe");
#endif
}

// https://github.com/gcc-mirror/gcc/blob/master/libssp/ssp.c
__attribute__((weak, noreturn, no_stack_protector)) void __chk_fail(void)
{
    TaskingPanic();
    for (short i = 0; i < 10; i++)
        error("Buffer overflow detected!");
    KPrint("\eFF0000Buffer overflow detected!");
#if defined(__amd64__) || defined(__i386__)
    while (1)
        asmv("cli; hlt");
#elif defined(__aarch64__)
    asmv("wfe");
#endif
}
