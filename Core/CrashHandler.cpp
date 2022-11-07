#include "crashhandler.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../kernel.h"

#if defined(__amd64__)
void DivideByZeroExceptionHandler(CPU::x64::TrapFrame *Frame);
void DebugExceptionHandler(CPU::x64::TrapFrame *Frame);
void NonMaskableInterruptExceptionHandler(CPU::x64::TrapFrame *Frame);
void BreakpointExceptionHandler(CPU::x64::TrapFrame *Frame);
void OverflowExceptionHandler(CPU::x64::TrapFrame *Frame);
void BoundRangeExceptionHandler(CPU::x64::TrapFrame *Frame);
void InvalidOpcodeExceptionHandler(CPU::x64::TrapFrame *Frame);
void DeviceNotAvailableExceptionHandler(CPU::x64::TrapFrame *Frame);
void DoubleFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void CoprocessorSegmentOverrunExceptionHandler(CPU::x64::TrapFrame *Frame);
void InvalidTSSExceptionHandler(CPU::x64::TrapFrame *Frame);
void SegmentNotPresentExceptionHandler(CPU::x64::TrapFrame *Frame);
void StackFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void GeneralProtectionExceptionHandler(CPU::x64::TrapFrame *Frame);
void PageFaultExceptionHandler(CPU::x64::TrapFrame *Frame);
void x87FloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame);
void AlignmentCheckExceptionHandler(CPU::x64::TrapFrame *Frame);
void MachineCheckExceptionHandler(CPU::x64::TrapFrame *Frame);
void SIMDFloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame);
void VirtualizationExceptionHandler(CPU::x64::TrapFrame *Frame);
void SecurityExceptionHandler(CPU::x64::TrapFrame *Frame);
void UnknownExceptionHandler(CPU::x64::TrapFrame *Frame);
void UserModeExceptionHandler(CPU::x64::TrapFrame *Frame);
#endif

namespace CrashHandler
{
    struct StackFrame
    {
        struct StackFrame *rbp;
        uint64_t rip;
    };

    __attribute__((no_stack_protector)) void TraceFrames(CPU::x64::TrapFrame *Frame, int Count)
    {
        struct StackFrame *frames = (struct StackFrame *)Frame->rbp; // (struct StackFrame *)__builtin_frame_address(0);
        debug("Stack tracing...");
        EHPrint("\e7981FC\nStack Trace:\n");
        if (!frames || !frames->rip || !frames->rbp)
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            EHPrint("\eAA25CC%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            EHPrint("\e7981FC <- Exception");
            EHPrint("\eFF0000\n< No stack trace available. >\n");
        }
        else
        {
            EHPrint("\e2565CC%p", (void *)Frame->rip);
            EHPrint("\e7925CC-");
            if (Frame->rip >= 0xFFFFFFFF80000000 && Frame->rip <= (uint64_t)&_kernel_end)
                EHPrint("\eAA25CC%s", KernelSymbolTable->GetSymbolFromAddress(Frame->rip));
            else
                EHPrint("Outside Kernel");
            EHPrint("\e7981FC <- Exception");
            for (int frame = 0; frame < Count; ++frame)
            {
                if (!frames->rip)
                    break;
                EHPrint("\n\e2565CC%p", (void *)frames->rip);
                EHPrint("\e7925CC-");
                if (frames->rip >= 0xFFFFFFFF80000000 && frames->rip <= (uint64_t)&_kernel_end)
                    EHPrint("\e25CCC9%s", KernelSymbolTable->GetSymbolFromAddress(frames->rip));
                else
                    EHPrint("\eFF4CA9Outside Kernel");

                if (!Memory::Virtual().Check(frames->rbp))
                    return;
                frames = frames->rbp;
            }
        }
    }

    __attribute__((no_stack_protector)) void printfWrapper(char c, void *unused)
    {
        Display->Print(c, 255, true);
        UNUSED(unused);
    }

    __attribute__((no_stack_protector)) void EHPrint(const char *Format, ...)
    {
        va_list args;
        va_start(args, Format);
        vfctprintf(printfWrapper, NULL, Format, args);
        va_end(args);
    }

    __attribute__((no_stack_protector)) void Handle(void *Data)
    {
#if defined(__amd64__)
        CPU::x64::TrapFrame *Frame = (CPU::x64::TrapFrame *)Data;
        error("Exception: %#llx", Frame->InterruptNumber);

        if (Frame->cs != GDT_USER_CODE && Frame->cs != GDT_USER_DATA)
        {
            debug("Exception in kernel mode");
            CPU::Interrupts(CPU::Disable);
            Display->CreateBuffer(0, 0, 255);
        }
        else
        {
            debug("Exception in user mode");
            if (!GetCurrentCPU()->CurrentThread->Security.IsCritical)
            {
                UserModeExceptionHandler(Frame);
                return;
            }
            else
                EHPrint("\eFF0000Init process crashed!");
        }

        debug("Reading control registers...");
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR2 cr2 = CPU::x64::readcr2();
        CPU::x64::CR3 cr3 = CPU::x64::readcr3();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        CPU::x64::CR8 cr8 = CPU::x64::readcr8();
        CPU::x64::EFER efer;
        efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        uint64_t dr0, dr1, dr2, dr3, dr6;
        CPU::x64::DR7 dr7;

        // store debug registers
        debug("Reading debug registers...");
        asmv("movq %%dr0, %0"
             : "=r"(dr0));
        asmv("movq %%dr1, %0"
             : "=r"(dr1));
        asmv("movq %%dr2, %0"
             : "=r"(dr2));
        asmv("movq %%dr3, %0"
             : "=r"(dr3));
        asmv("movq %%dr6, %0"
             : "=r"(dr6));
        asmv("movq %%dr7, %0"
             : "=r"(dr7));

        switch (Frame->InterruptNumber)
        {
        case CPU::x64::DivideByZero:
        {
            DivideByZeroExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Debug:
        {
            DebugExceptionHandler(Frame);
            break;
        }
        case CPU::x64::NonMaskableInterrupt:
        {
            NonMaskableInterruptExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Breakpoint:
        {
            BreakpointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Overflow:
        {
            OverflowExceptionHandler(Frame);
            break;
        }
        case CPU::x64::BoundRange:
        {
            BoundRangeExceptionHandler(Frame);
            break;
        }
        case CPU::x64::InvalidOpcode:
        {
            InvalidOpcodeExceptionHandler(Frame);
            break;
        }
        case CPU::x64::DeviceNotAvailable:
        {
            DeviceNotAvailableExceptionHandler(Frame);
            break;
        }
        case CPU::x64::DoubleFault:
        {
            DoubleFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::CoprocessorSegmentOverrun:
        {
            CoprocessorSegmentOverrunExceptionHandler(Frame);
            break;
        }
        case CPU::x64::InvalidTSS:
        {
            InvalidTSSExceptionHandler(Frame);
            break;
        }
        case CPU::x64::SegmentNotPresent:
        {
            SegmentNotPresentExceptionHandler(Frame);
            break;
        }
        case CPU::x64::StackSegmentFault:
        {
            StackFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::GeneralProtectionFault:
        {
            GeneralProtectionExceptionHandler(Frame);
            break;
        }
        case CPU::x64::PageFault:
        {
            PageFaultExceptionHandler(Frame);
            break;
        }
        case CPU::x64::x87FloatingPoint:
        {
            x87FloatingPointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::AlignmentCheck:
        {
            AlignmentCheckExceptionHandler(Frame);
            break;
        }
        case CPU::x64::MachineCheck:
        {
            MachineCheckExceptionHandler(Frame);
            break;
        }
        case CPU::x64::SIMDFloatingPoint:
        {
            SIMDFloatingPointExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Virtualization:
        {
            VirtualizationExceptionHandler(Frame);
            break;
        }
        case CPU::x64::Security:
        {
            SecurityExceptionHandler(Frame);
            break;
        }
        default:
        {
            UnknownExceptionHandler(Frame);
            break;
        }
        }

        EHPrint("\e7981FCTechnical Informations on CPU %lld:\n", GetCurrentCPU()->ID);
        EHPrint("FS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx\n",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                Frame->ss, Frame->cs, Frame->ds);
        EHPrint("R8=%#llx  R9=%#llx  R10=%#llx  R11=%#llx\n", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        EHPrint("R12=%#llx  R13=%#llx  R14=%#llx  R15=%#llx\n", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        EHPrint("RAX=%#llx  RBX=%#llx  RCX=%#llx  RDX=%#llx\n", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        EHPrint("RSI=%#llx  RDI=%#llx  RBP=%#llx  RSP=%#llx\n", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        EHPrint("RIP=%#llx  RFL=%#llx  INT=%#llx  ERR=%#llx  EFER=%#llx\n", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
        EHPrint("CR0=%#llx  CR2=%#llx  CR3=%#llx  CR4=%#llx  CR8=%#llx\n", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);
        EHPrint("DR0=%#llx  DR1=%#llx  DR2=%#llx  DR3=%#llx  DR6=%#llx  DR7=%#llx\n", dr0, dr1, dr2, dr3, dr6, dr7.raw);

        EHPrint("\eFC797BCR0: PE:%s     MP:%s     EM:%s     TS:%s\n     ET:%s     NE:%s     WP:%s     AM:%s\n     NW:%s     CD:%s     PG:%s\n     R0:%#x R1:%#x R2:%#x\n",
                cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
                cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
                cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
                cr0.Reserved0, cr0.Reserved1, cr0.Reserved2);

        EHPrint("\eFCBD79CR2: PFLA: %#llx\n",
                cr2.PFLA);

        EHPrint("\e79FC84CR3: PWT:%s     PCD:%s    PDBR:%#llx\n",
                cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);

        EHPrint("\eBD79FCCR4: VME:%s     PVI:%s     TSD:%s      DE:%s\n     PSE:%s     PAE:%s     MCE:%s     PGE:%s\n     PCE:%s    UMIP:%s  OSFXSR:%s OSXMMEXCPT:%s\n    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s\n OSXSAVE:%s    SMEP:%s    SMAP:%s     PKE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
                cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
                cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
                cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
                cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
                cr4.Reserved0, cr4.Reserved1, cr4.Reserved2);

        EHPrint("\e79FCF5CR8: TPL:%d\n", cr8.TPL);

        EHPrint("\eFCFC02RFL: CF:%s     PF:%s     AF:%s     ZF:%s\n     SF:%s     TF:%s     IF:%s     DF:%s\n     OF:%s   IOPL:%s     NT:%s     RF:%s\n     VM:%s     AC:%s    VIF:%s    VIP:%s\n     ID:%s     AlwaysOne:%d\n     R0:%#x R1:%#x R2:%#x R3:%#x\n",
                Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
                Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
                Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
                Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
                Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
                Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);

        EHPrint("\eA0F0F0DR7: LDR0:%s     GDR0:%s     LDR1:%s     GDR1:%s\n     LDR2:%s     GDR2:%s     LDR3:%s     GDR3:%s\n     CDR0:%s     SDR0:%s     CDR1:%s     SDR1:%s\n     CDR2:%s     SDR2:%s     CDR3:%s     SDR3:%s\n     R:%#x\n",
                dr7.LocalDR0 ? "True " : "False", dr7.GlobalDR0 ? "True " : "False", dr7.LocalDR1 ? "True " : "False", dr7.GlobalDR1 ? "True " : "False",
                dr7.LocalDR2 ? "True " : "False", dr7.GlobalDR2 ? "True " : "False", dr7.LocalDR3 ? "True " : "False", dr7.GlobalDR3 ? "True " : "False",
                dr7.ConditionsDR0 ? "True " : "False", dr7.SizeDR0 ? "True " : "False", dr7.ConditionsDR1 ? "True " : "False", dr7.SizeDR1 ? "True " : "False",
                dr7.ConditionsDR2 ? "True " : "False", dr7.SizeDR2 ? "True " : "False", dr7.ConditionsDR3 ? "True " : "False", dr7.SizeDR3 ? "True " : "False",
                dr7.Reserved);

        EHPrint("\e009FF0EFER: SCE:%s      LME:%s      LMA:%s      NXE:%s\n     SVME:%s    LMSLE:%s    FFXSR:%s      TCE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
                efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
                efer.Reserved0, efer.Reserved1, efer.Reserved2);

        // restore debug registers
        debug("Restoring debug registers...");
        asmv("movq %0, %%dr0"
             :
             : "r"(dr0));
        asmv("movq %0, %%dr1"
             :
             : "r"(dr1));
        asmv("movq %0, %%dr2"
             :
             : "r"(dr2));
        asmv("movq %0, %%dr3"
             :
             : "r"(dr3));
        asmv("movq %0, %%dr6"
             :
             : "r"(dr6));
        asmv("movq %0, %%dr7"
             :
             : "r"(dr7));

        TraceFrames(Frame, 20);
        goto CrashEnd;

#elif defined(__i386__)
        void *Frame = Data;
#elif defined(__aarch64__)
        void *Frame = Data;
#endif

    CrashEnd:
        Display->SetBuffer(255);
        CPU::Stop();
    }
}

#if defined(__amd64__) || defined(__i386__)
static const char *PagefaultDescriptions[] = {
    "Supervisory process tried to read a non-present page entry\n",
    "Supervisory process tried to read a page and caused a protection fault\n",
    "Supervisory process tried to write to a non-present page entry\n",
    "Supervisory process tried to write a page and caused a protection fault\n",
    "User process tried to read a non-present page entry\n",
    "User process tried to read a page and caused a protection fault\n",
    "User process tried to write to a non-present page entry\n",
    "User process tried to write a page and caused a protection fault\n"};
#endif

#if defined(__amd64__)
#define staticbuffer(name) char name[] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

__attribute__((no_stack_protector)) void DivideByZeroExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    fixme("Divide by zero exception\n");
}
__attribute__((no_stack_protector)) void DebugExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel triggered debug exception.\n");
}
__attribute__((no_stack_protector)) void NonMaskableInterruptExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("NMI exception"); }
__attribute__((no_stack_protector)) void BreakpointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Breakpoint exception"); }
__attribute__((no_stack_protector)) void OverflowExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Overflow exception"); }
__attribute__((no_stack_protector)) void BoundRangeExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Bound range exception"); }
__attribute__((no_stack_protector)) void InvalidOpcodeExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel tried to execute an invalid opcode.\n");
}
__attribute__((no_stack_protector)) void DeviceNotAvailableExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Device not available exception"); }
__attribute__((no_stack_protector)) void DoubleFaultExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Double fault exception"); }
__attribute__((no_stack_protector)) void CoprocessorSegmentOverrunExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Coprocessor segment overrun exception"); }
__attribute__((no_stack_protector)) void InvalidTSSExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Invalid TSS exception"); }
__attribute__((no_stack_protector)) void SegmentNotPresentExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Segment not present exception"); }
__attribute__((no_stack_protector)) void StackFaultExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    staticbuffer(descbuf);
    staticbuffer(desc_ext);
    staticbuffer(desc_table);
    staticbuffer(desc_idx);
    staticbuffer(desc_tmp);
    CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
    switch (SelCode.Table)
    {
    case 0b00:
        memcpy(desc_tmp, "GDT", 3);
        break;
    case 0b01:
        memcpy(desc_tmp, "IDT", 3);
        break;
    case 0b10:
        memcpy(desc_tmp, "LDT", 3);
        break;
    case 0b11:
        memcpy(desc_tmp, "IDT", 3);
        break;
    default:
        memcpy(desc_tmp, "Unknown", 7);
        break;
    }
    debug("external:%d table:%d idx:%#x", SelCode.External, SelCode.Table, SelCode.Idx);
    sprintf_(descbuf, "Stack segment fault at address %#lx", Frame->rip);
    CrashHandler::EHPrint(descbuf);
    sprintf_(desc_ext, "External: %d", SelCode.External);
    CrashHandler::EHPrint(desc_ext);
    sprintf_(desc_table, "Table: %d (%s)", SelCode.Table, desc_tmp);
    CrashHandler::EHPrint(desc_table);
    sprintf_(desc_idx, "%s Index: %#x", desc_tmp, SelCode.Idx);
    CrashHandler::EHPrint(desc_idx);
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("More info about the exception:\n");
}
__attribute__((no_stack_protector)) void GeneralProtectionExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    // staticbuffer(descbuf);
    // staticbuffer(desc_ext);
    // staticbuffer(desc_table);
    // staticbuffer(desc_idx);
    // staticbuffer(desc_tmp);
    CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
    // switch (SelCode.Table)
    // {
    // case CPU::x64::0b00:
    //     memcpy(desc_tmp, "GDT", 3);
    //     break;
    // case CPU::x64::0b01:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // case CPU::x64::0b10:
    //     memcpy(desc_tmp, "LDT", 3);
    //     break;
    // case CPU::x64::0b11:
    //     memcpy(desc_tmp, "IDT", 3);
    //     break;
    // default:
    //     memcpy(desc_tmp, "Unknown", 7);
    //     break;
    // }
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel performed an illegal operation.\n");
    CrashHandler::EHPrint("More info about the exception:\n");
    CrashHandler::EHPrint("External: %d\n", SelCode.External);
    CrashHandler::EHPrint("Table: %d\n", SelCode.Table);
    CrashHandler::EHPrint("Index: %#x\n", SelCode.Idx);
}
__attribute__((no_stack_protector)) void PageFaultExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
    CrashHandler::EHPrint("\eDD2920System crashed!\n\eFFFFFF");
    CrashHandler::EHPrint("An exception occurred at %#lx by %#lx\n", CPU::x64::readcr2().PFLA, Frame->rip);
    CrashHandler::EHPrint("Page: %s\n", params.P ? "Present" : "Not Present");
    CrashHandler::EHPrint("Write Operation: %s\n", params.W ? "Read-Only" : "Read-Write");
    CrashHandler::EHPrint("Processor Mode: %s\n", params.U ? "User-Mode" : "Kernel-Mode");
    CrashHandler::EHPrint("CPU Reserved Bits: %s\n", params.R ? "Reserved" : "Unreserved");
    CrashHandler::EHPrint("Caused By An Instruction Fetch: %s\n", params.I ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By A Protection-Key Violation: %s\n", params.PK ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By A Shadow Stack Access: %s\n", params.SS ? "Yes" : "No");
    CrashHandler::EHPrint("Caused By An SGX Violation: %s\n", params.SGX ? "Yes" : "No");
    if (Frame->ErrorCode & 0x00000008)
        CrashHandler::EHPrint("One or more page directory entries contain reserved bits which are set to 1.\n");
    else
        CrashHandler::EHPrint(PagefaultDescriptions[Frame->ErrorCode & 0b111]);
}
__attribute__((no_stack_protector)) void x87FloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("x87 floating point exception"); }
__attribute__((no_stack_protector)) void AlignmentCheckExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Alignment check exception"); }
__attribute__((no_stack_protector)) void MachineCheckExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Machine check exception"); }
__attribute__((no_stack_protector)) void SIMDFloatingPointExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("SIMD floating point exception"); }
__attribute__((no_stack_protector)) void VirtualizationExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Virtualization exception"); }
__attribute__((no_stack_protector)) void SecurityExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Security exception"); }
__attribute__((no_stack_protector)) void UnknownExceptionHandler(CPU::x64::TrapFrame *Frame) { fixme("Unknown exception"); }

__attribute__((no_stack_protector)) void UserModeExceptionHandler(CPU::x64::TrapFrame *Frame)
{
    CriticalSection cs;
    debug("Interrupts? %s.", cs.IsInterruptsEnabled() ? "Yes" : "No");
    fixme("Handling user mode exception");
    TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;

    {
        CPU::x64::CR0 cr0 = CPU::x64::readcr0();
        CPU::x64::CR2 cr2 = CPU::x64::readcr2();
        CPU::x64::CR3 cr3 = CPU::x64::readcr3();
        CPU::x64::CR4 cr4 = CPU::x64::readcr4();
        CPU::x64::CR8 cr8 = CPU::x64::readcr8();
        CPU::x64::EFER efer;
        efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);

        error("Technical Informations on CPU %lld:", GetCurrentCPU()->ID);
        error("FS=%#llx GS=%#llx SS=%#llx CS=%#llx DS=%#llx",
              CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
              Frame->ss, Frame->cs, Frame->ds);
        error("R8=%#llx R9=%#llx R10=%#llx R11=%#llx", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
        error("R12=%#llx R13=%#llx R14=%#llx R15=%#llx", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
        error("RAX=%#llx RBX=%#llx RCX=%#llx RDX=%#llx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
        error("RSI=%#llx RDI=%#llx RBP=%#llx RSP=%#llx", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
        error("RIP=%#llx RFL=%#llx INT=%#llx ERR=%#llx EFER=%#llx", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
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

        error("RFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x R3:%#x",
              Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
              Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
              Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
              Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
              Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
              Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);

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
        CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
        error("An exception occurred at %#lx by %#lx", CPU::x64::readcr2().PFLA, Frame->rip);
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
}
#endif
