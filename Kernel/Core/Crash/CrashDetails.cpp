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

SafeFunction void DivideByZeroExceptionHandler(CHArchTrapFrame *Frame)
{
    fixme("Divide by zero exception\n");
}
SafeFunction void DebugExceptionHandler(CHArchTrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel triggered debug exception.\n");
}
SafeFunction void NonMaskableInterruptExceptionHandler(CHArchTrapFrame *Frame) { fixme("NMI exception"); }
SafeFunction void BreakpointExceptionHandler(CHArchTrapFrame *Frame) { fixme("Breakpoint exception"); }
SafeFunction void OverflowExceptionHandler(CHArchTrapFrame *Frame) { fixme("Overflow exception"); }
SafeFunction void BoundRangeExceptionHandler(CHArchTrapFrame *Frame) { fixme("Bound range exception"); }
SafeFunction void InvalidOpcodeExceptionHandler(CHArchTrapFrame *Frame)
{
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("Kernel tried to execute an invalid opcode.\n");
}
SafeFunction void DeviceNotAvailableExceptionHandler(CHArchTrapFrame *Frame) { fixme("Device not available exception"); }
SafeFunction void DoubleFaultExceptionHandler(CHArchTrapFrame *Frame) { fixme("Double fault exception"); }
SafeFunction void CoprocessorSegmentOverrunExceptionHandler(CHArchTrapFrame *Frame) { fixme("Coprocessor segment overrun exception"); }
SafeFunction void InvalidTSSExceptionHandler(CHArchTrapFrame *Frame) { fixme("Invalid TSS exception"); }
SafeFunction void SegmentNotPresentExceptionHandler(CHArchTrapFrame *Frame) { fixme("Segment not present exception"); }
SafeFunction void StackFaultExceptionHandler(CHArchTrapFrame *Frame)
{
    CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
    CrashHandler::EHPrint("\eDD2920System crashed!\n");
    CrashHandler::EHPrint("More info about the exception:\n");
#if defined(__amd64__)
    CrashHandler::EHPrint("Stack segment fault at address %#lx\n", Frame->rip);
#elif defined(__i386__)
    CrashHandler::EHPrint("Stack segment fault at address %#lx\n", Frame->eip);
#elif defined(__aarch64__)
#endif
    CrashHandler::EHPrint("External: %d\n", SelCode.External);
    CrashHandler::EHPrint("Table: %d\n", SelCode.Table);
    CrashHandler::EHPrint("Index: %#x\n", SelCode.Idx);
    CrashHandler::EHPrint("Error code: %#lx\n", Frame->ErrorCode);
}
SafeFunction void GeneralProtectionExceptionHandler(CHArchTrapFrame *Frame)
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
SafeFunction void PageFaultExceptionHandler(CHArchTrapFrame *Frame)
{
    CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
    CrashHandler::EHPrint("\eDD2920System crashed!\n\eFFFFFF");
#if defined(__amd64__)
    CrashHandler::EHPrint("An exception occurred at %#lx by %#lx\n", CPU::x64::readcr2().PFLA, Frame->rip);
#elif defined(__i386__)
    CrashHandler::EHPrint("An exception occurred at %#lx by %#lx\n", CPU::x64::readcr2().PFLA, Frame->eip);
#elif defined(__aarch64__)
#endif
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
SafeFunction void x87FloatingPointExceptionHandler(CHArchTrapFrame *Frame) { fixme("x87 floating point exception"); }
SafeFunction void AlignmentCheckExceptionHandler(CHArchTrapFrame *Frame) { fixme("Alignment check exception"); }
SafeFunction void MachineCheckExceptionHandler(CHArchTrapFrame *Frame) { fixme("Machine check exception"); }
SafeFunction void SIMDFloatingPointExceptionHandler(CHArchTrapFrame *Frame) { fixme("SIMD floating point exception"); }
SafeFunction void VirtualizationExceptionHandler(CHArchTrapFrame *Frame) { fixme("Virtualization exception"); }
SafeFunction void SecurityExceptionHandler(CHArchTrapFrame *Frame) { fixme("Security exception"); }
SafeFunction void UnknownExceptionHandler(CHArchTrapFrame *Frame) { fixme("Unknown exception"); }
