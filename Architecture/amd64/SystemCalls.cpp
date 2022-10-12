#include <syscalls.hpp>

#include <cpu.hpp>

#include "cpu/gdt.hpp"

using namespace CPU::x64;

extern "C" __attribute__((naked, used, no_stack_protector)) void SystemCallHandlerStub()
{
}

extern "C" uint64_t SystemCallsHandler(SyscallsRegs *regs);

void InitializeSystemCalls()
{
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
    wrmsr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE) << 32) | ((uint64_t)(GDT_KERNEL_DATA | 3) << 48));
    wrmsr(MSR_LSTAR, (uint64_t)SystemCallsHandler);
    wrmsr(MSR_SYSCALL_MASK, 0);
}
