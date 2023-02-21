#include "../../crashhandler.hpp"
#include "../chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(__amd64__)
#include "../../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(__i386__)
#elif defined(__aarch64__)
#endif

#include "../../../kernel.h"

static const char *PagefaultDescriptions[8] = {
    "Supervisory process tried to read a non-present page entry\n",
    "Supervisory process tried to read a page and caused a protection fault\n",
    "Supervisory process tried to write to a non-present page entry\n",
    "Supervisory process tried to write a page and caused a protection fault\n",
    "User process tried to read a non-present page entry\n",
    "User process tried to read a page and caused a protection fault\n",
    "User process tried to write to a non-present page entry\n",
    "User process tried to write a page and caused a protection fault\n"};

namespace CrashHandler
{
    SafeFunction void DisplayMainScreen(CRData data)
    {
        CHArchTrapFrame *Frame = data.Frame;

        /*
 _______ ___ ___ _______ _______ _______ _______      ______ ______ _______ _______ _______ _______ _____
|     __|   |   |     __|_     _|    ___|   |   |    |      |   __ \   _   |     __|   |   |    ___|     \
|__     |\     /|__     | |   | |    ___|       |    |   ---|      <       |__     |       |    ___|  --  |
|_______| |___| |_______| |___| |_______|__|_|__|    |______|___|__|___|___|_______|___|___|_______|_____/
        */
        EHPrint("\eFF5500 _______ ___ ___ _______ _______ _______ _______      ______ ______ _______ _______ _______ _______ _____  \n");
        EHPrint("|     __|   |   |     __|_     _|    ___|   |   |    |      |   __ \\   _   |     __|   |   |    ___|     \\ \n");
        EHPrint("|__     |\\     /|__     | |   | |    ___|       |    |   ---|      <       |__     |       |    ___|  --  |\n");
        EHPrint("|_______| |___| |_______| |___| |_______|__|_|__|    |______|___|__|___|___|_______|___|___|_______|_____/ \n\eFAFAFA");

        switch (Frame->InterruptNumber)
        {
        case CPU::x86::DivideByZero:
        {
            EHPrint("Exception: Divide By Zero\n");
            EHPrint("The processor attempted to divide a number by zero.\n");
            break;
        }
        case CPU::x86::Debug:
        {
            EHPrint("Exception: Debug\n");
            EHPrint("A debug exception has occurred.\n");
            break;
        }
        case CPU::x86::NonMaskableInterrupt:
        {
            EHPrint("Exception: Non-Maskable Interrupt\n");
            EHPrint("A non-maskable interrupt was received.\n");
            break;
        }
        case CPU::x86::Breakpoint:
        {
            EHPrint("Exception: Breakpoint\n");
            EHPrint("The processor encountered a breakpoint.\n");
            break;
        }
        case CPU::x86::Overflow:
        {
            EHPrint("Exception: Overflow\n");
            EHPrint("The processor attempted to add a number to a number that was too large.\n");
            break;
        }
        case CPU::x86::BoundRange:
        {
            EHPrint("Exception: Bound Range\n");
            EHPrint("The processor attempted to access an array element that is out of bounds.\n");
            break;
        }
        case CPU::x86::InvalidOpcode:
        {
            EHPrint("Exception: Invalid Opcode\n");
            EHPrint("The processor attempted to execute an invalid opcode.\n");
            break;
        }
        case CPU::x86::DeviceNotAvailable:
        {
            EHPrint("Exception: Device Not Available\n");
            EHPrint("The processor attempted to use a device that is not available.\n");
            break;
        }
        case CPU::x86::DoubleFault:
        {
            EHPrint("Exception: Double Fault\n");
            EHPrint("The processor encountered a double fault.\n");
            break;
        }
        case CPU::x86::CoprocessorSegmentOverrun:
        {
            EHPrint("Exception: Coprocessor Segment Overrun\n");
            EHPrint("The processor attempted to access a segment that is not available.\n");
            break;
        }
        case CPU::x86::InvalidTSS:
        {
            EHPrint("Exception: Invalid TSS\n");
            EHPrint("The processor attempted to access a task state segment that is not available or valid.\n");
            CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
            EHPrint("External? %s\n", SelCode.External ? "Yes" : "No");
            EHPrint("GDT  IDT  LDT  IDT\n");
            switch (SelCode.Table)
            {
            case 0b00:
            {
                EHPrint(" ^                \n");
                EHPrint(" |                \n");
                EHPrint(" %ld\n", SelCode.Idx);
                break;
            }
            case 0b01:
            {
                EHPrint("      ^           \n");
                EHPrint("      |           \n");
                EHPrint("      %ld\n", SelCode.Idx);
                break;
            }
            case 0b10:
            {
                EHPrint("           ^      \n");
                EHPrint("           |      \n");
                EHPrint("           %ld\n", SelCode.Idx);
                break;
            }
            case 0b11:
            {
                EHPrint("                ^ \n");
                EHPrint("                | \n");
                EHPrint("                %ld\n", SelCode.Idx);
                break;
            }
            }
            break;
        }
        case CPU::x86::SegmentNotPresent:
        {
            EHPrint("Exception: Segment Not Present\n");
            EHPrint("The processor attempted to access a segment that is not present.\n");
            CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
            EHPrint("External? %s\n", SelCode.External ? "Yes" : "No");
            EHPrint("GDT  IDT  LDT  IDT\n");
            switch (SelCode.Table)
            {
            case 0b00:
            {
                EHPrint(" ^                \n");
                EHPrint(" |                \n");
                EHPrint(" %ld\n", SelCode.Idx);
                break;
            }
            case 0b01:
            {
                EHPrint("      ^           \n");
                EHPrint("      |           \n");
                EHPrint("      %ld\n", SelCode.Idx);
                break;
            }
            case 0b10:
            {
                EHPrint("           ^      \n");
                EHPrint("           |      \n");
                EHPrint("           %ld\n", SelCode.Idx);
                break;
            }
            case 0b11:
            {
                EHPrint("                ^ \n");
                EHPrint("                | \n");
                EHPrint("                %ld\n", SelCode.Idx);
                break;
            }
            }
            break;
        }
        case CPU::x86::StackSegmentFault:
        {
            EHPrint("Exception: Stack Segment Fault\n");
            CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
            EHPrint("External? %s\n", SelCode.External ? "Yes" : "No");
            EHPrint("GDT  IDT  LDT  IDT\n");
            switch (SelCode.Table)
            {
            case 0b00:
            {
                EHPrint(" ^                \n");
                EHPrint(" |                \n");
                EHPrint(" %ld\n", SelCode.Idx);
                break;
            }
            case 0b01:
            {
                EHPrint("      ^           \n");
                EHPrint("      |           \n");
                EHPrint("      %ld\n", SelCode.Idx);
                break;
            }
            case 0b10:
            {
                EHPrint("           ^      \n");
                EHPrint("           |      \n");
                EHPrint("           %ld\n", SelCode.Idx);
                break;
            }
            case 0b11:
            {
                EHPrint("                ^ \n");
                EHPrint("                | \n");
                EHPrint("                %ld\n", SelCode.Idx);
                break;
            }
            }
            break;
        }
        case CPU::x86::GeneralProtectionFault:
        {
            EHPrint("Exception: General Protection Fault\n");
            EHPrint("Kernel performed an illegal operation.\n");
            CPU::x64::SelectorErrorCode SelCode = {.raw = Frame->ErrorCode};
            EHPrint("External? %s\n", SelCode.External ? "Yes" : "No");
            EHPrint("GDT  IDT  LDT  IDT\n");
            switch (SelCode.Table)
            {
            case 0b00:
            {
                EHPrint(" ^                \n");
                EHPrint(" |                \n");
                EHPrint(" %ld\n", SelCode.Idx);
                break;
            }
            case 0b01:
            {
                EHPrint("      ^           \n");
                EHPrint("      |           \n");
                EHPrint("      %ld\n", SelCode.Idx);
                break;
            }
            case 0b10:
            {
                EHPrint("           ^      \n");
                EHPrint("           |      \n");
                EHPrint("           %ld\n", SelCode.Idx);
                break;
            }
            case 0b11:
            {
                EHPrint("                ^ \n");
                EHPrint("                | \n");
                EHPrint("                %ld\n", SelCode.Idx);
                break;
            }
            }
            break;
        }
        case CPU::x86::PageFault:
        {
            EHPrint("Exception: Page Fault\n");
            EHPrint("The processor attempted to access a page that is not present.\n");

            CPU::x64::PageFaultErrorCode params = {.raw = (uint32_t)Frame->ErrorCode};
#if defined(__amd64__)
            EHPrint("At \e8888FF%#lx \eFAFAFAby \e8888FF%#lx\eFAFAFA\n", PageFaultAddress, Frame->rip);
#elif defined(__i386__)
            EHPrint("At \e8888FF%#lx \eFAFAFAby \e8888FF%#lx\eFAFAFA\n", PageFaultAddress, Frame->eip);
#elif defined(__aarch64__)
#endif
            EHPrint("Page: %s\eFAFAFA\n", params.P ? "\e058C19Present" : "\eE85230Not Present");
            EHPrint("Write Operation: \e8888FF%s\eFAFAFA\n", params.W ? "Read-Only" : "Read-Write");
            EHPrint("Processor Mode: \e8888FF%s\eFAFAFA\n", params.U ? "User-Mode" : "Kernel-Mode");
            EHPrint("CPU Reserved Bits: %s\eFAFAFA\n", params.R ? "\eE85230Reserved" : "\e058C19Unreserved");
            EHPrint("Caused By An Instruction Fetch: %s\eFAFAFA\n", params.I ? "\eE85230Yes" : "\e058C19No");
            EHPrint("Caused By A Protection-Key Violation: %s\eFAFAFA\n", params.PK ? "\eE85230Yes" : "\e058C19No");
            EHPrint("Caused By A Shadow Stack Access: %s\eFAFAFA\n", params.SS ? "\eE85230Yes" : "\e058C19No");
            EHPrint("Caused By An SGX Violation: %s\eFAFAFA\n", params.SGX ? "\eE85230Yes" : "\e058C19No");
            EHPrint("More Info: \e8888FF");
            if (Frame->ErrorCode & 0x00000008)
                EHPrint("One or more page directory entries contain reserved bits which are set to 1.\n");
            else
                EHPrint(PagefaultDescriptions[Frame->ErrorCode & 0b111]);
            EHPrint("\eFAFAFA");
            break;
        }
        case CPU::x86::x87FloatingPoint:
        {
            EHPrint("Exception: x87 Floating Point\n");
            EHPrint("The x87 FPU generated an error.\n");
            break;
        }
        case CPU::x86::AlignmentCheck:
        {
            EHPrint("Exception: Alignment Check\n");
            EHPrint("The CPU detected an unaligned memory access.\n");
            break;
        }
        case CPU::x86::MachineCheck:
        {
            EHPrint("Exception: Machine Check\n");
            EHPrint("The CPU detected a hardware error.\n");
            break;
        }
        case CPU::x86::SIMDFloatingPoint:
        {
            EHPrint("Exception: SIMD Floating Point\n");
            EHPrint("The CPU detected an error in the SIMD unit.\n");
            break;
        }
        case CPU::x86::Virtualization:
        {
            EHPrint("Exception: Virtualization\n");
            EHPrint("The CPU detected a virtualization error.\n");
            break;
        }
        case CPU::x86::Security:
        {
            EHPrint("Exception: Security\n");
            EHPrint("The CPU detected a security violation.\n");
            break;
        }
        default:
        {
            EHPrint("Exception: Unknown\n");
            EHPrint("The CPU generated an unknown exception.\n");
            break;
        }
        }

#if defined(__amd64__)
        EHPrint("The exception happened at \e8888FF%#lx\eFAFAFA\n", Frame->rip);
#elif defined(__i386__)
        EHPrint("The exception happened at \e8888FF%#lx\eFAFAFA\n", Frame->eip);
#elif defined(__aarch64__)
#endif
    }
}
