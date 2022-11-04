#include <syscalls.hpp>

#include <debug.h>

extern "C" uint64_t SystemCallsHandler(SyscallsRegs *regs)
{
#if defined(__amd64__)
    fixme("System call %ld", regs->rax);
#elif defined(__i386__)
    fixme("System call %lld", regs->eax);
#elif defined(__aarch64__)
    fixme("System call");
#endif
    return 0;
}
