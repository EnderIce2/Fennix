/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <syscalls.hpp>

#include <cpu.hpp>

#include "cpu/gdt.hpp"

// https://supercip971.github.io/02-wingos-syscalls.html
using namespace CPU::x64;

// "Core/SystemCalls.cpp"
extern "C" uint64_t SystemCallsHandler(SyscallsFrame *regs);

extern "C" void SystemCallHandlerStub();

extern "C" __naked __used __no_stack_protector __aligned(16) void SystemCallHandlerStub()
{
    asmv("swapgs\n");

    asmv("mov %rsp, %gs:0x8\n"); // CPUData->TempStack
    asmv("mov %gs:0x0, %rsp\n"); // CPUData->SystemCallStack
    asmv("push $0x1b\n");        // user data segment
    asmv("push %gs:0x8\n");      // saved stack
    asmv("push %r11\n");         // saved rflags
    asmv("push $0x23\n");        // user code segment
    asmv("push %rcx\n");         // Current RIP

    asmv("push %rax\n"
         "push %rbx\n"
         "push %rcx\n"
         "push %rdx\n"
         "push %rsi\n"
         "push %rdi\n"
         "push %rbp\n"
         "push %r8\n"
         "push %r9\n"
         "push %r10\n"
         "push %r11\n"
         "push %r12\n"
         "push %r13\n"
         "push %r14\n"
         "push %r15\n");

    asmv("mov %rsp, %rdi\n");
    asmv("mov $0, %rbp\n");
    asmv("call SystemCallsHandler\n");

    asmv("pop %r15\n"
         "pop %r14\n"
         "pop %r13\n"
         "pop %r12\n"
         "pop %r11\n"
         "pop %r10\n"
         "pop %r9\n"
         "pop %r8\n"
         "pop %rbp\n"
         "pop %rdi\n"
         "pop %rsi\n"
         "pop %rdx\n"
         "pop %rcx\n"
         "pop %rbx\n");

    asmv("mov %gs:0x8, %rsp\n"); // CPUData->TempStack
#ifdef DEBUG
    asmv("movq $0, %gs:0x8\n"); // Easier to debug stacks // FIXME: Can't use xor
#endif

    asmv("swapgs\n");
    asmv("sti\n");
    asmv("sysretq\n");
}

void InitializeSystemCalls()
{
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
    wrmsr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE) << 32) | ((uint64_t)(GDT_KERNEL_DATA | 3) << 48));
    wrmsr(MSR_LSTAR, (uint64_t)SystemCallHandlerStub);
    wrmsr(MSR_SYSCALL_MASK, (uint64_t)(1 << 9));
}
