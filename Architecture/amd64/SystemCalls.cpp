#include <syscalls.hpp>

#include <cpu.hpp>

#include "cpu/gdt.hpp"

// https://supercip971.github.io/02-wingos-syscalls.html
using namespace CPU::x64;

// "Core/SystemCalls.cpp"
extern "C" uint64_t SystemCallsHandler(SyscallsRegs *regs);

extern "C" void SystemCallHandlerStub();

extern "C" __attribute__((naked, used, no_stack_protector)) void SystemCallHandlerStub_broken()
{
    // asmv(
    //     // "cmp $0x08, 0x8(%rsp)\n"
    //     //  "je 1f\n"
    //     "swapgs\n"
    //     //  "1:\n"

    //     "mov %rsp, 0x8(%gs)\n" // CPUData->TempStack
    //     "mov 0x0(%gs), %rsp\n" // CPUData->SystemCallStack
    //     "push $0x1b\n"         // user data segment
    //     "push 0x8(%gs)\n"      // saved stack
    //     "push %r11\n"          // saved rflags
    //     "push $0x23\n"         // user code segment
    //     "push %rcx\n"          // Current RIP

    //     "push %rax\n"
    //     "push %rbx\n"
    //     "push %rcx\n"
    //     "push %rdx\n"
    //     "push %rsi\n"
    //     "push %rdi\n"
    //     "push %rbp\n"
    //     "push %r8\n"
    //     "push %r9\n"
    //     "push %r10\n"
    //     "push %r11\n"
    //     "push %r12\n"
    //     "push %r13\n"
    //     "push %r14\n"
    //     "push %r15\n"

    //     "mov %rsp, %rdi\n"
    //     "mov $0, %rbp\n"
    //     "call SystemCallsHandler\n"

    //     "pop %r15\n"
    //     "pop %r14\n"
    //     "pop %r13\n"
    //     "pop %r12\n"
    //     "pop %r11\n"
    //     "pop %r10\n"
    //     "pop %r9\n"
    //     "pop %r8\n"
    //     "pop %rbp\n"
    //     "pop %rdi\n"
    //     "pop %rsi\n"
    //     "pop %rdx\n"
    //     "pop %rcx\n"
    //     "pop %rbx\n"
    //     /* "pop %rax\n" */

    //     "mov 0x8(%gs), %rsp\n" // CPUData->TempStack

    //     //  "cmp $0x08, 0x8(%rsp)\n"
    //     //  "je 1f\n"
    //     "swapgs\n"
    //     //  "1:\n"

    //     "sti\n"

    //     "sysretq\n");
}

void InitializeSystemCalls()
{
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
    wrmsr(MSR_STAR, ((uint64_t)(GDT_KERNEL_CODE) << 32) | ((uint64_t)(GDT_KERNEL_DATA | 3) << 48));
    wrmsr(MSR_LSTAR, (uint64_t)SystemCallHandlerStub);
    wrmsr(MSR_SYSCALL_MASK, (uint64_t)(1 << 9));
}
