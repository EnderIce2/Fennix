#ifndef __FENNIX_KERNEL_SYSCALLS_LIST_H__
#define __FENNIX_KERNEL_SYSCALLS_LIST_H__

#include <types.h>

enum NativeSyscalls
{
    _Exit = 0,
    _Print,

    _RequestPages,
    _FreePages,
};

static inline long syscall0(long syscall)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall1(long syscall, long arg1)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall2(long syscall, long arg1, long arg2)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall3(long syscall, long arg1, long arg2, long arg3)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall4(long syscall, long arg1, long arg2, long arg3, long arg4)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall5(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall6(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    register long r9 __asm__("r9") = arg6;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
                         : "rcx", "r11", "memory");
    return ret;
}

#endif // !__FENNIX_KERNEL_SYSCALLS_LIST_H__
