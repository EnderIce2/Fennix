#ifndef __FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__
#define __FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__

#include <types.h>
#include <cpu.hpp>

#if defined(__amd64__)
typedef struct CPU::x64::TrapFrame CHArchTrapFrame;
#elif defined(__i386__)
typedef struct CPU::x86::TrapFrame CHArchTrapFrame;
#elif defined(__aarch64__)
typedef struct CPU::aarch64::TrapFrame CHArchTrapFrame;
#endif

namespace CrashHandler
{
    void TraceFrames(CHArchTrapFrame *Frame, int Count);
}

void DivideByZeroExceptionHandler(CHArchTrapFrame *Frame);
void DebugExceptionHandler(CHArchTrapFrame *Frame);
void NonMaskableInterruptExceptionHandler(CHArchTrapFrame *Frame);
void BreakpointExceptionHandler(CHArchTrapFrame *Frame);
void OverflowExceptionHandler(CHArchTrapFrame *Frame);
void BoundRangeExceptionHandler(CHArchTrapFrame *Frame);
void InvalidOpcodeExceptionHandler(CHArchTrapFrame *Frame);
void DeviceNotAvailableExceptionHandler(CHArchTrapFrame *Frame);
void DoubleFaultExceptionHandler(CHArchTrapFrame *Frame);
void CoprocessorSegmentOverrunExceptionHandler(CHArchTrapFrame *Frame);
void InvalidTSSExceptionHandler(CHArchTrapFrame *Frame);
void SegmentNotPresentExceptionHandler(CHArchTrapFrame *Frame);
void StackFaultExceptionHandler(CHArchTrapFrame *Frame);
void GeneralProtectionExceptionHandler(CHArchTrapFrame *Frame);
void PageFaultExceptionHandler(CHArchTrapFrame *Frame);
void x87FloatingPointExceptionHandler(CHArchTrapFrame *Frame);
void AlignmentCheckExceptionHandler(CHArchTrapFrame *Frame);
void MachineCheckExceptionHandler(CHArchTrapFrame *Frame);
void SIMDFloatingPointExceptionHandler(CHArchTrapFrame *Frame);
void VirtualizationExceptionHandler(CHArchTrapFrame *Frame);
void SecurityExceptionHandler(CHArchTrapFrame *Frame);
void UnknownExceptionHandler(CHArchTrapFrame *Frame);
void UserModeExceptionHandler(CHArchTrapFrame *Frame);

#endif // !__FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__
