#include <syscalls.hpp>

#include <cpu.hpp>

extern "C" __attribute__((naked, used, no_stack_protector)) void SystemCallHandlerStub()
{

}

extern "C" uint64_t SystemCallsHandler(SyscallsRegs *regs);

void InitializeSystemCalls()
{
}
