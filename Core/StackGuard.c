#include <types.h>
#include <debug.h>

#include "../kernel.h"

#ifndef STACK_CHK_GUARD_VALUE
#if UINTPTR_MAX == UINT32_MAX
#define STACK_CHK_GUARD_VALUE 0x25F6CC8D
#else
#define STACK_CHK_GUARD_VALUE 0xBADFE2EC255A8572
#endif
#endif

__attribute__((weak)) uintptr_t __stack_chk_guard = 0;

__attribute__((weak)) uintptr_t __stack_chk_guard_init(void)
{
    return STACK_CHK_GUARD_VALUE;
}

static void __attribute__((constructor, no_stack_protector)) __construct_stk_chk_guard()
{
    if (__stack_chk_guard == 0)
        __stack_chk_guard = __stack_chk_guard_init();
}

// https://opensource.apple.com/source/xnu/xnu-1504.7.4/libkern/stack_protector.c.auto.html
// static void __guard_setup(void) __attribute__((constructor));

// static void __guard_setup(void)
// {
// read_random(__stack_chk_guard, sizeof(__stack_chk_guard));
// }

__attribute__((weak, noreturn, no_stack_protector)) void __stack_chk_fail(void)
{
    TaskingPanic();
    error("Stack smashing detected!");
    KPrint("\eFF0000Stack smashing detected!");
#if defined(__amd64__) || defined(__i386__)
    asmv("loop__stack_chk_fail:\n"
         "cli\n"
         "hlt\n"
         "jmp loop__stack_chk_fail\n");
#elif defined(__aarch64__)
    asmv("wfe");
#endif
}

// https://github.com/gcc-mirror/gcc/blob/master/libssp/ssp.c
__attribute__((weak, noreturn, no_stack_protector)) void __chk_fail(void)
{
    TaskingPanic();
    error("Buffer overflow detected!");
    KPrint("\eFF0000Buffer overflow detected!");
    for (;;)
    {
#if defined(__amd64__) || defined(__i386__)
        asmv("loop__chk_fail:\n"
             "cli\n"
             "hlt\n"
             "jmp loop__chk_fail\n");
#elif defined(__aarch64__)
        asmv("wfe");
#endif
    }
}
