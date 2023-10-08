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

#ifndef __FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__
#define __FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__

#include <types.h>

#include <ints.hpp>
#include <task.hpp>
#include <cpu.hpp>

#if defined(a64)
typedef struct CPU::x64::TrapFrame CHArchTrapFrame;

struct CRData
{
    CHArchTrapFrame *Frame;

    CPU::x64::CR0 cr0;
    CPU::x64::CR2 cr2;
    CPU::x64::CR3 cr3;
    CPU::x64::CR4 cr4;
    CPU::x64::CR8 cr8;
    CPU::x64::EFER efer;
    uintptr_t dr0, dr1, dr2, dr3;
    CPU::x64::DR6 dr6;
    CPU::x64::DR7 dr7;

    long ID;
    void *CPUData;
    Tasking::PCB *Process;
    Tasking::TCB *Thread;
};

#elif defined(a32)
typedef struct CPU::x32::TrapFrame CHArchTrapFrame;

struct CRData
{
    CHArchTrapFrame *Frame;

    CPU::x32::CR0 cr0;
    CPU::x32::CR2 cr2;
    CPU::x32::CR3 cr3;
    CPU::x32::CR4 cr4;
    CPU::x32::CR8 cr8;
    uintptr_t dr0, dr1, dr2, dr3;
    CPU::x32::DR6 dr6;
    CPU::x32::DR7 dr7;

    long ID;
    void *CPUData;
    Tasking::PCB *Process;
    Tasking::TCB *Thread;
};
#elif defined(aa64)
typedef struct CPU::aarch64::TrapFrame CHArchTrapFrame;

struct CRData
{
    CHArchTrapFrame *Frame;

    long ID;
    void *CPUData;
    Tasking::PCB *Process;
    Tasking::TCB *Thread;
};
#endif

enum Keys
{
    KEY_INVALID = 0x0,
    KEY_D_ESCAPE = 0x1,
    KEY_D_1 = 0x2,
    KEY_D_2 = 0x3,
    KEY_D_3 = 0x4,
    KEY_D_4 = 0x5,
    KEY_D_5 = 0x6,
    KEY_D_6 = 0x7,
    KEY_D_7 = 0x8,
    KEY_D_8 = 0x9,
    KEY_D_9 = 0xa,
    KEY_D_0 = 0xb,
    KEY_D_MINUS = 0xc,
    KEY_D_EQUALS = 0xd,
    KEY_D_BACKSPACE = 0xe,
    KEY_D_TAB = 0xf,
    KEY_D_Q = 0x10,
    KEY_D_W = 0x11,
    KEY_D_E = 0x12,
    KEY_D_R = 0x13,
    KEY_D_T = 0x14,
    KEY_D_Y = 0x15,
    KEY_D_U = 0x16,
    KEY_D_I = 0x17,
    KEY_D_O = 0x18,
    KEY_D_P = 0x19,
    KEY_D_LBRACKET = 0x1a,
    KEY_D_RBRACKET = 0x1b,
    KEY_D_RETURN = 0x1c,
    KEY_D_LCTRL = 0x1d,
    KEY_D_A = 0x1e,
    KEY_D_S = 0x1f,
    KEY_D_D = 0x20,
    KEY_D_F = 0x21,
    KEY_D_G = 0x22,
    KEY_D_H = 0x23,
    KEY_D_J = 0x24,
    KEY_D_K = 0x25,
    KEY_D_L = 0x26,
    KEY_D_SEMICOLON = 0x27,
    KEY_D_APOSTROPHE = 0x28,
    KEY_D_GRAVE = 0x29,
    KEY_D_LSHIFT = 0x2a,
    KEY_D_BACKSLASH = 0x2b,
    KEY_D_Z = 0x2c,
    KEY_D_X = 0x2d,
    KEY_D_C = 0x2e,
    KEY_D_V = 0x2f,
    KEY_D_B = 0x30,
    KEY_D_N = 0x31,
    KEY_D_M = 0x32,
    KEY_D_COMMA = 0x33,
    KEY_D_PERIOD = 0x34,
    KEY_D_SLASH = 0x35,
    KEY_D_RSHIFT = 0x36,
    KEY_D_PRTSC = 0x37,
    KEY_D_LALT = 0x38,
    KEY_D_SPACE = 0x39,
    KEY_D_CAPSLOCK = 0x3a,
    KEY_D_NUMLOCK = 0x45,
    KEY_D_SCROLLLOCK = 0x46,

    KEY_D_KP_MULTIPLY = 0x37,
    KEY_D_KP_7 = 0x47,
    KEY_D_KP_8 = 0x48,
    KEY_D_KP_9 = 0x49,
    KEY_D_KP_MINUS = 0x4a,
    KEY_D_KP_4 = 0x4b,
    KEY_D_KP_5 = 0x4c,
    KEY_D_KP_6 = 0x4d,
    KEY_D_KP_PLUS = 0x4e,
    KEY_D_KP_1 = 0x4f,
    KEY_D_KP_2 = 0x50,
    KEY_D_KP_3 = 0x51,
    KEY_D_KP_0 = 0x52,
    KEY_D_KP_PERIOD = 0x53,

    KEY_D_F1 = 0x3b,
    KEY_D_F2 = 0x3c,
    KEY_D_F3 = 0x3d,
    KEY_D_F4 = 0x3e,
    KEY_D_F5 = 0x3f,
    KEY_D_F6 = 0x40,
    KEY_D_F7 = 0x41,
    KEY_D_F8 = 0x42,
    KEY_D_F9 = 0x43,
    KEY_D_F10 = 0x44,
    KEY_D_F11 = 0x57,
    KEY_D_F12 = 0x58,

    KEY_D_UP = 0x48,
    KEY_D_LEFT = 0x4b,
    KEY_D_RIGHT = 0x4d,
    KEY_D_DOWN = 0x50,

    KEY_U_ESCAPE = 0x81,
    KEY_U_1 = 0x82,
    KEY_U_2 = 0x83,
    KEY_U_3 = 0x84,
    KEY_U_4 = 0x85,
    KEY_U_5 = 0x86,
    KEY_U_6 = 0x87,
    KEY_U_7 = 0x88,
    KEY_U_8 = 0x89,
    KEY_U_9 = 0x8a,
    KEY_U_0 = 0x8b,
    KEY_U_MINUS = 0x8c,
    KEY_U_EQUALS = 0x8d,
    KEY_U_BACKSPACE = 0x8e,
    KEY_U_TAB = 0x8f,
    KEY_U_Q = 0x90,
    KEY_U_W = 0x91,
    KEY_U_E = 0x92,
    KEY_U_R = 0x93,
    KEY_U_T = 0x94,
    KEY_U_Y = 0x95,
    KEY_U_U = 0x96,
    KEY_U_I = 0x97,
    KEY_U_O = 0x98,
    KEY_U_P = 0x99,
    KEY_U_LBRACKET = 0x9a,
    KEY_U_RBRACKET = 0x9b,
    KEY_U_RETURN = 0x9c,
    KEY_U_LCTRL = 0x9d,
    KEY_U_A = 0x9e,
    KEY_U_S = 0x9f,
    KEY_U_D = 0xa0,
    KEY_U_F = 0xa1,
    KEY_U_G = 0xa2,
    KEY_U_H = 0xa3,
    KEY_U_J = 0xa4,
    KEY_U_K = 0xa5,
    KEY_U_L = 0xa6,
    KEY_U_SEMICOLON = 0xa7,
    KEY_U_APOSTROPHE = 0xa8,
    KEY_U_GRAVE = 0xa9,
    KEY_U_LSHIFT = 0xaa,
    KEY_U_BACKSLASH = 0xab,
    KEY_U_Z = 0xac,
    KEY_U_X = 0xad,
    KEY_U_C = 0xae,
    KEY_U_V = 0xaf,
    KEY_U_B = 0xb0,
    KEY_U_N = 0xb1,
    KEY_U_M = 0xb2,
    KEY_U_COMMA = 0xb3,
    KEY_U_PERIOD = 0xb4,
    KEY_U_SLASH = 0xb5,
    KEY_U_RSHIFT = 0xb6,
    KEY_U_KP_MULTIPLY = 0xb7,
    KEY_U_LALT = 0xb8,
    KEY_U_SPACE = 0xb9,
    KEY_U_CAPSLOCK = 0xba,
    KEY_U_F1 = 0xbb,
    KEY_U_F2 = 0xbc,
    KEY_U_F3 = 0xbd,
    KEY_U_F4 = 0xbe,
    KEY_U_F5 = 0xbf,
    KEY_U_F6 = 0xc0,
    KEY_U_F7 = 0xc1,
    KEY_U_F8 = 0xc2,
    KEY_U_F9 = 0xc3,
    KEY_U_F10 = 0xc4,
    KEY_U_NUMLOCK = 0xc5,
    KEY_U_SCROLLLOCK = 0xc6,
    KEY_U_KP_7 = 0xc7,
    KEY_U_KP_8 = 0xc8,
    KEY_U_KP_9 = 0xc9,
    KEY_U_KP_MINUS = 0xca,
    KEY_U_KP_4 = 0xcb,
    KEY_U_KP_5 = 0xcc,
    KEY_U_KP_6 = 0xcd,
    KEY_U_KP_PLUS = 0xce,
    KEY_U_KP_1 = 0xcf,
    KEY_U_KP_2 = 0xd0,
    KEY_U_KP_3 = 0xd1,
    KEY_U_KP_0 = 0xd2,
    KEY_U_KP_PERIOD = 0xd3,
    KEY_U_F11 = 0xd7,
    KEY_U_F12 = 0xd8,
};

namespace CrashHandler
{
    extern int SBIdx;

    class CrashKeyboardDriver : public Interrupts::Handler
    {
    private:
        void PS2Wait(bool Read);

#if defined(a64)
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(a32)
        void OnInterruptReceived(CPU::x32::TrapFrame *Frame);
#elif defined(aa64)
        void OnInterruptReceived(CPU::aarch64::TrapFrame *Frame);
#endif
    public:
        CrashKeyboardDriver();
        ~CrashKeyboardDriver();
    };

    void TraceFrames(CRData data, int Count, SymbolResolver::Symbols *SymHandle, bool Kernel);

    void ArrowInput(uint8_t key);
    void UserInput(char *Input);

    void DisplayMainScreen(CRData data);
    void DisplayDetailsScreen(CRData data);
    void DisplayStackFrameScreen(CRData data);
    void DisplayTasksScreen(CRData data);
    void DisplayConsoleScreen(CRData data);
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

bool UserModeExceptionHandler(CHArchTrapFrame *Frame);

#endif // !__FENNIX_KERNEL_CRASH_HANDLERS_FUNCTIONS_H__
