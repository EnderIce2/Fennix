#include <syscalls.hpp>

#include <cpu.hpp>

#include "cpu/gdt.hpp"

using namespace CPU::x32;

extern "C" uint32_t SystemCallsHandler(SyscallsFrame *regs);

void InitializeSystemCalls()
{
}
