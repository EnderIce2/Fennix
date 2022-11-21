#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../../kernel.h"

static const char *PagefaultDescriptions[8] = {
    "Supervisory process tried to read a non-present page entry\n",
    "Supervisory process tried to read a page and caused a protection fault\n",
    "Supervisory process tried to write to a non-present page entry\n",
    "Supervisory process tried to write a page and caused a protection fault\n",
    "User process tried to read a non-present page entry\n",
    "User process tried to read a page and caused a protection fault\n",
    "User process tried to write to a non-present page entry\n",
    "User process tried to write a page and caused a protection fault\n"};

__no_stack_protector void UserModeExceptionHandler(CHArchTrapFrame *Frame)
{
    CriticalSection cs;
    debug("Interrupts? %s.", cs.IsInterruptsEnabled() ? "Yes" : "No");
    fixme("Handling user mode exception");
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Stopped;
    CPUData *CurCPU = GetCurrentCPU();

    {
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR2 cr2 = CPU::x64::readcr2();
        CPU::x64::CR3 cr3 = CPU::x64::readcr3();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        CPU::x64::CR8 cr8 = CPU::x64::readcr8();
        CPU::x64::EFER efer;
        efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        error("Technical Informations on CPU %lld:", CurCPU->ID);
#if defined(__amd64__)
        uint64_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(__i386__)
        uint32_t ds;
        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(__aarch64__)
#endif
        error("FS=%#llx GS=%#llx SS=%#llx CS=%#llx DS=%#llx",
              CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
              Frame->ss, Frame->cs, ds);
#if defined(__amd64__)
        error("R8=%#llx R9=%#llx R10=%#llx R11=%#llx", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        error("R12=%#llx R13=%#llx R14=%#llx R15=%#llx", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        error("RAX=%#llx RBX=%#llx RCX=%#llx RDX=%#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        error("RSI=%#llx RDI=%#llx RBP=%#llx RSP=%#llx", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        error("RIP=%#llx RFL=%#llx INT=%#llx ERR=%#llx EFER=%#llx", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
#elif defined(__i386__)
        error("EAX=%#llx EBX=%#llx ECX=%#llx EDX=%#llx", Frame->eax, Frame->ebx, Frame->ecx, Frame->edx);
        error("ESI=%#llx EDI=%#llx EBP=%#llx ESP=%#llx", Frame->esi, Frame->edi, Frame->ebp, Frame->esp);
        error("EIP=%#llx EFL=%#llx INT=%#llx ERR=%#llx EFER=%#llx", Frame->eip, Frame->eflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
#elif defined(__aarch64__)
#endif
        error("CR0=%#llx CR2=%#llx CR3=%#llx CR4=%#llx CR8=%#llx", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);

        error("CR0: PE:%s MP:%s EM:%s TS:%s ET:%s NE:%s WP:%s AM:%s NW:%s CD:%s PG:%s R0:%#x R1:%#x R2:%#x",
              cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
              cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
              cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
              cr0.Reserved0, cr0.Reserved1, cr0.Reserved2);

        error("CR2: PFLA: %#llx",
              cr2.PFLA);

        error("CR3: PWT:%s PCD:%s PDBR:%#llx",
              cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);

        error("CR4: VME:%s PVI:%s TSD:%s DE:%s PSE:%s PAE:%s MCE:%s PGE:%s PCE:%s UMIP:%s OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s PKE:%s R0:%#x R1:%#x R2:%#x",
              cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
              cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
              cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
              cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
              cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
              cr4.Reserved0, cr4.Reserved1, cr4.Reserved2);

        error("CR8: TPL:%d", cr8.TPL);

#if defined(__amd64__)
        error("RFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x R3:%#x",
              Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
              Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
              Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
              Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
              Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
              Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);
#elif defined(__i386__)
        error("EFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x",
              Frame->eflags.CF ? "True " : "False", Frame->eflags.PF ? "True " : "False", Frame->eflags.AF ? "True " : "False", Frame->eflags.ZF ? "True " : "False",
              Frame->eflags.SF ? "True " : "False", Frame->eflags.TF ? "True " : "False", Frame->eflags.IF ? "True " : "False", Frame->eflags.DF ? "True " : "False",
              Frame->eflags.OF ? "True " : "False", Frame->eflags.IOPL ? "True " : "False", Frame->eflags.NT ? "True " : "False", Frame->eflags.RF ? "True " : "False",
              Frame->eflags.VM ? "True " : "False", Frame->eflags.AC ? "True " : "False", Frame->eflags.VIF ? "True " : "False", Frame->eflags.VIP ? "True " : "False",
              Frame->eflags.ID ? "True " : "False", Frame->eflags.AlwaysOne,
              Frame->eflags.Reserved0, Frame->eflags.Reserved1, Frame->eflags.Reserved2);
#elif defined(__aarch64__)
#endif

        error("EFER: SCE:%s LME:%s LMA:%s NXE:%s SVME:%s LMSLE:%s FFXSR:%s TCE:%s R0:%#x R1:%#x R2:%#x",
              efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
              efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
              efer.Reserved0, efer.Reserved1, efer.Reserved2);
    }

    switch (Frame->InterruptNumber)
    {
    case CPU::x64::DivideByZero:
    {
        break;
    }
    case CPU::x64::Debug:
    {
        break;
    }
    case CPU::x64::NonMaskableInterrupt:
    {
        break;
    }
    case CPU::x64::Breakpoint:
    {
        break;
    }
    case CPU::x64::Overflow:
    {
        break;
    }
    case CPU::x64::BoundRange:
    {
        break;
    }
    case CPU::x64::InvalidOpcode:
    {
        break;
    }
    case CPU::x64::DeviceNotAvailable:
    {
        break;
    }
    case CPU::x64::DoubleFault:
    {
        break;
    }
    case CPU::x64::CoprocessorSegmentOverrun:
    {
        break;
    }
    case CPU::x64::InvalidTSS:
    {
        break;
    }
    case CPU::x64::SegmentNotPresent:
    {
        break;
    }
    case CPU::x64::StackSegmentFault:
    {
        break;
    }
    case CPU::x64::GeneralProtectionFault:
    {
        break;
    }
    case CPU::x64::PageFault:
    {
        if (CurCPU)
            if (CurCPU->CurrentThread->Stack->Expand(CPU::x64::readcr2().raw))
            {
                debug("Stack expanded");
                TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Ready;
                return;
            }

        CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
#if defined(__amd64__)
        error("An exception occurred at %#lx by %#lx", CPU::x64::readcr2().PFLA, Frame->rip);
#elif defined(__i386__)
        error("An exception occurred at %#lx by %#lx", CPU::x64::readcr2().PFLA, Frame->eip);
#elif defined(__aarch64__)
#endif
        error("Page: %s", params.P ? "Present" : "Not Present");
        error("Write Operation: %s", params.W ? "Read-Only" : "Read-Write");
        error("Processor Mode: %s", params.U ? "User-Mode" : "Kernel-Mode");
        error("CPU Reserved Bits: %s", params.R ? "Reserved" : "Unreserved");
        error("Caused By An Instruction Fetch: %s", params.I ? "Yes" : "No");
        error("Caused By A Protection-Key Violation: %s", params.PK ? "Yes" : "No");
        error("Caused By A Shadow Stack Access: %s", params.SS ? "Yes" : "No");
        error("Caused By An SGX Violation: %s", params.SGX ? "Yes" : "No");
        if (Frame->ErrorCode & 0x00000008)
            error("One or more page directory entries contain reserved bits which are set to 1.");
        else
            error(PagefaultDescriptions[Frame->ErrorCode & 0b111]);
        break;
    }
    case CPU::x64::x87FloatingPoint:
    {
        break;
    }
    case CPU::x64::AlignmentCheck:
    {
        break;
    }
    case CPU::x64::MachineCheck:
    {
        break;
    }
    case CPU::x64::SIMDFloatingPoint:
    {
        break;
    }
    case CPU::x64::Virtualization:
    {
        break;
    }
    case CPU::x64::Security:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
    __sync_synchronize();
    error("End of report.");
    CPU::Interrupts(CPU::Enable);
    debug("Interrupts enabled back.");
    return;
}
