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

#include "../../crashhandler.hpp"
#include "../chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../../kernel.h"

namespace CrashHandler
{
    SafeFunction void DisplayDetailsScreen(CRData data)
    {
        if (data.Process)
            EHPrint("\e7981FCCurrent Process: %s(%ld)\n",
                    data.Process->Name,
                    data.Process->ID);
        if (data.Thread)
            EHPrint("\e7981FCCurrent Thread: %s(%ld)\n",
                    data.Thread->Name,
                    data.Thread->ID);
        EHPrint("\e7981FCTechnical Informations on CPU %lld:\n", data.ID);
        uintptr_t ds;
#if defined(a64)

        CPUData *cpu = (CPUData *)data.CPUData;
        if (cpu)
        {
            EHPrint("\eE46CEBCPU Data Address: %#lx\n", cpu);
            EHPrint("Syscalls Stack: %#lx, TempStack: %#lx\n", cpu->SystemCallStack, cpu->TempStack);
            EHPrint("Core Stack: %#lx, Core ID: %ld, Error Code: %ld\n", cpu->Stack, cpu->ID, cpu->ErrorCode);
            EHPrint("Is Active: %s\n", cpu->IsActive ? "true" : "false");
            EHPrint("Current Process: %#lx, Current Thread: %#lx\n", cpu->CurrentProcess, cpu->CurrentThread);
            EHPrint("Arch Specific Data: %#lx\n", cpu->Data);
            EHPrint("Checksum: 0x%X\n", cpu->Checksum);
        }

        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(a32)
        asmv("mov %%ds, %0"
             : "=r"(ds));
#elif defined(aa64)
#endif

#if defined(a64)
        EHPrint("\e7981FCFS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx\n",
                CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
                data.Frame->ss, data.Frame->cs, ds);
        EHPrint("R8=%#llx  R9=%#llx  R10=%#llx  R11=%#llx\n", data.Frame->r8, data.Frame->r9, data.Frame->r10, data.Frame->r11);
        EHPrint("R12=%#llx  R13=%#llx  R14=%#llx  R15=%#llx\n", data.Frame->r12, data.Frame->r13, data.Frame->r14, data.Frame->r15);
        EHPrint("RAX=%#llx  RBX=%#llx  RCX=%#llx  RDX=%#llx\n", data.Frame->rax, data.Frame->rbx, data.Frame->rcx, data.Frame->rdx);
        EHPrint("RSI=%#llx  RDI=%#llx  RBP=%#llx  RSP=%#llx\n", data.Frame->rsi, data.Frame->rdi, data.Frame->rbp, data.Frame->rsp);
        EHPrint("RIP=%#llx  RFL=%#llx  INT=%#llx  ERR=%#llx  EFER=%#llx\n", data.Frame->rip, data.Frame->rflags.raw, data.Frame->InterruptNumber, data.Frame->ErrorCode, data.efer.raw);
#elif defined(a32)
        EHPrint("\e7981FCFS=%#llx  GS=%#llx  SS=%#llx  CS=%#llx  DS=%#llx\n",
                CPU::x32::rdmsr(CPU::x32::MSR_FS_BASE), CPU::x32::rdmsr(CPU::x32::MSR_GS_BASE),
                data.Frame->ss, data.Frame->cs, ds);
        EHPrint("EAX=%#llx  EBX=%#llx  ECX=%#llx  EDX=%#llx\n", data.Frame->eax, data.Frame->ebx, data.Frame->ecx, data.Frame->edx);
        EHPrint("ESI=%#llx  EDI=%#llx  EBP=%#llx  ESP=%#llx\n", data.Frame->esi, data.Frame->edi, data.Frame->ebp, data.Frame->esp);
        EHPrint("EIP=%#llx  EFL=%#llx  INT=%#llx  ERR=%#llx  EFER=%#llx\n", data.Frame->eip, data.Frame->eflags.raw, data.Frame->InterruptNumber, data.Frame->ErrorCode, data.efer.raw);
#elif defined(aa64)
#endif

#if defined(a86)
        EHPrint("CR0=%#llx  CR2=%#llx  CR3=%#llx  CR4=%#llx  CR8=%#llx\n", data.cr0.raw, data.cr2.raw, data.cr3.raw, data.cr4.raw, data.cr8.raw);
        EHPrint("DR0=%#llx  DR1=%#llx  DR2=%#llx  DR3=%#llx  DR6=%#llx  DR7=%#llx\n", data.dr0, data.dr1, data.dr2, data.dr3, data.dr6, data.dr7.raw);

        EHPrint("\eFC797BCR0: PE:%s     MP:%s     EM:%s     TS:%s\n     ET:%s     NE:%s     WP:%s     AM:%s\n     NW:%s     CD:%s     PG:%s\n     R0:%#x R1:%#x R2:%#x\n",
                data.cr0.PE ? "True " : "False", data.cr0.MP ? "True " : "False", data.cr0.EM ? "True " : "False", data.cr0.TS ? "True " : "False",
                data.cr0.ET ? "True " : "False", data.cr0.NE ? "True " : "False", data.cr0.WP ? "True " : "False", data.cr0.AM ? "True " : "False",
                data.cr0.NW ? "True " : "False", data.cr0.CD ? "True " : "False", data.cr0.PG ? "True " : "False",
                data.cr0.Reserved0, data.cr0.Reserved1, data.cr0.Reserved2);

        EHPrint("\eFCBD79CR2: PFLA: %#llx\n",
                data.cr2.PFLA);

        EHPrint("\e79FC84CR3: PWT:%s     PCD:%s    PDBR:%#llx\n",
                data.cr3.PWT ? "True " : "False", data.cr3.PCD ? "True " : "False", data.cr3.PDBR);

        EHPrint("\eBD79FCCR4: VME:%s     PVI:%s     TSD:%s      DE:%s\n     PSE:%s     PAE:%s     MCE:%s     PGE:%s\n     PCE:%s    UMIP:%s  OSFXSR:%s OSXMMEXCPT:%s\n    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s\n OSXSAVE:%s    SMEP:%s    SMAP:%s     PKE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                data.cr4.VME ? "True " : "False", data.cr4.PVI ? "True " : "False", data.cr4.TSD ? "True " : "False", data.cr4.DE ? "True " : "False",
                data.cr4.PSE ? "True " : "False", data.cr4.PAE ? "True " : "False", data.cr4.MCE ? "True " : "False", data.cr4.PGE ? "True " : "False",
                data.cr4.PCE ? "True " : "False", data.cr4.UMIP ? "True " : "False", data.cr4.OSFXSR ? "True " : "False", data.cr4.OSXMMEXCPT ? "True " : "False",
                data.cr4.LA57 ? "True " : "False", data.cr4.VMXE ? "True " : "False", data.cr4.SMXE ? "True " : "False", data.cr4.PCIDE ? "True " : "False",
                data.cr4.OSXSAVE ? "True " : "False", data.cr4.SMEP ? "True " : "False", data.cr4.SMAP ? "True " : "False", data.cr4.PKE ? "True " : "False",
#if defined(a64)
                data.cr4.Reserved0, data.cr4.Reserved1, data.cr4.Reserved2);
#elif defined(a32)
                data.cr4.Reserved0, data.cr4.Reserved1, 0);
#endif
        EHPrint("\e79FCF5CR8: TPL:%d\n", data.cr8.TPL);
#endif // a64 || a32

#if defined(a64)
        EHPrint("\eFCFC02RFL: CF:%s     PF:%s     AF:%s     ZF:%s\n     SF:%s     TF:%s     IF:%s     DF:%s\n     OF:%s   IOPL:%s     NT:%s     RF:%s\n     VM:%s     AC:%s    VIF:%s    VIP:%s\n     ID:%s     AlwaysOne:%d\n     R0:%#x R1:%#x R2:%#x R3:%#x\n",
                data.Frame->rflags.CF ? "True " : "False", data.Frame->rflags.PF ? "True " : "False", data.Frame->rflags.AF ? "True " : "False", data.Frame->rflags.ZF ? "True " : "False",
                data.Frame->rflags.SF ? "True " : "False", data.Frame->rflags.TF ? "True " : "False", data.Frame->rflags.IF ? "True " : "False", data.Frame->rflags.DF ? "True " : "False",
                data.Frame->rflags.OF ? "True " : "False", data.Frame->rflags.IOPL ? "True " : "False", data.Frame->rflags.NT ? "True " : "False", data.Frame->rflags.RF ? "True " : "False",
                data.Frame->rflags.VM ? "True " : "False", data.Frame->rflags.AC ? "True " : "False", data.Frame->rflags.VIF ? "True " : "False", data.Frame->rflags.VIP ? "True " : "False",
                data.Frame->rflags.ID ? "True " : "False", data.Frame->rflags.AlwaysOne,
                data.Frame->rflags.Reserved0, data.Frame->rflags.Reserved1, data.Frame->rflags.Reserved2, data.Frame->rflags.Reserved3);
#elif defined(a32)
        EHPrint("\eFCFC02EFL: CF:%s     PF:%s     AF:%s     ZF:%s\n     SF:%s     TF:%s     IF:%s     DF:%s\n     OF:%s   IOPL:%s     NT:%s     RF:%s\n     VM:%s     AC:%s    VIF:%s    VIP:%s\n     ID:%s     AlwaysOne:%d\n     R0:%#x R1:%#x R2:%#x\n",
                data.Frame->eflags.CF ? "True " : "False", data.Frame->eflags.PF ? "True " : "False", data.Frame->eflags.AF ? "True " : "False", data.Frame->eflags.ZF ? "True " : "False",
                data.Frame->eflags.SF ? "True " : "False", data.Frame->eflags.TF ? "True " : "False", data.Frame->eflags.IF ? "True " : "False", data.Frame->eflags.DF ? "True " : "False",
                data.Frame->eflags.OF ? "True " : "False", data.Frame->eflags.IOPL ? "True " : "False", data.Frame->eflags.NT ? "True " : "False", data.Frame->eflags.RF ? "True " : "False",
                data.Frame->eflags.VM ? "True " : "False", data.Frame->eflags.AC ? "True " : "False", data.Frame->eflags.VIF ? "True " : "False", data.Frame->eflags.VIP ? "True " : "False",
                data.Frame->eflags.ID ? "True " : "False", data.Frame->eflags.AlwaysOne,
                data.Frame->eflags.Reserved0, data.Frame->eflags.Reserved1, data.Frame->eflags.Reserved2);
#elif defined(aa64)
#endif

#if defined(a86)
        EHPrint("\eA0F0F0DR7: LDR0:%s     GDR0:%s     LDR1:%s     GDR1:%s\n     LDR2:%s     GDR2:%s     LDR3:%s     GDR3:%s\n     CDR0:%s     SDR0:%s     CDR1:%s     SDR1:%s\n     CDR2:%s     SDR2:%s     CDR3:%s     SDR3:%s\n     R:%#x\n",
                data.dr7.LocalDR0 ? "True " : "False", data.dr7.GlobalDR0 ? "True " : "False", data.dr7.LocalDR1 ? "True " : "False", data.dr7.GlobalDR1 ? "True " : "False",
                data.dr7.LocalDR2 ? "True " : "False", data.dr7.GlobalDR2 ? "True " : "False", data.dr7.LocalDR3 ? "True " : "False", data.dr7.GlobalDR3 ? "True " : "False",
                data.dr7.ConditionsDR0 ? "True " : "False", data.dr7.SizeDR0 ? "True " : "False", data.dr7.ConditionsDR1 ? "True " : "False", data.dr7.SizeDR1 ? "True " : "False",
                data.dr7.ConditionsDR2 ? "True " : "False", data.dr7.SizeDR2 ? "True " : "False", data.dr7.ConditionsDR3 ? "True " : "False", data.dr7.SizeDR3 ? "True " : "False",
                data.dr7.Reserved);

        EHPrint("\e009FF0EFER: SCE:%s      LME:%s      LMA:%s      NXE:%s\n     SVME:%s    LMSLE:%s    FFXSR:%s      TCE:%s\n     R0:%#x R1:%#x R2:%#x\n",
                data.efer.SCE ? "True " : "False", data.efer.LME ? "True " : "False", data.efer.LMA ? "True " : "False", data.efer.NXE ? "True " : "False",
                data.efer.SVME ? "True " : "False", data.efer.LMSLE ? "True " : "False", data.efer.FFXSR ? "True " : "False", data.efer.TCE ? "True " : "False",
                data.efer.Reserved0, data.efer.Reserved1, data.efer.Reserved2);
#endif

        switch (data.Frame->InterruptNumber)
        {
        case CPU::x86::DivideByZero:
        {
            DivideByZeroExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::Debug:
        {
            DebugExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::NonMaskableInterrupt:
        {
            NonMaskableInterruptExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::Breakpoint:
        {
            BreakpointExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::Overflow:
        {
            OverflowExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::BoundRange:
        {
            BoundRangeExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::InvalidOpcode:
        {
            InvalidOpcodeExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::DeviceNotAvailable:
        {
            DeviceNotAvailableExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::DoubleFault:
        {
            DoubleFaultExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::CoprocessorSegmentOverrun:
        {
            CoprocessorSegmentOverrunExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::InvalidTSS:
        {
            InvalidTSSExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::SegmentNotPresent:
        {
            SegmentNotPresentExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::StackSegmentFault:
        {
            StackFaultExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::GeneralProtectionFault:
        {
            GeneralProtectionExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::PageFault:
        {
            PageFaultExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::x87FloatingPoint:
        {
            x87FloatingPointExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::AlignmentCheck:
        {
            AlignmentCheckExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::MachineCheck:
        {
            MachineCheckExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::SIMDFloatingPoint:
        {
            SIMDFloatingPointExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::Virtualization:
        {
            VirtualizationExceptionHandler(data.Frame);
            break;
        }
        case CPU::x86::Security:
        {
            SecurityExceptionHandler(data.Frame);
            break;
        }
        default:
        {
            UnknownExceptionHandler(data.Frame);
            break;
        }
        }
    }
}
