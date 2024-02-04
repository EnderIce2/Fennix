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

#include "../crashhandler.hpp"
#include "chfcts.hpp"

#include <display.hpp>
#include <printf.h>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>

#if defined(a64)
#include "../../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#elif defined(aa64)
#endif

#include "../../kernel.h"

nsa bool UserModeExceptionHandler(CPU::TrapFrame *Frame)
{
	CPUData *CurCPU = GetCurrentCPU();
	Tasking::PCB *CurProc = CurCPU->CurrentProcess;
	Tasking::TCB *CurThread = CurCPU->CurrentThread;
	debug("Current process %s(%d) and thread %s(%d)",
		  CurProc->Name, CurProc->ID, CurThread->Name, CurThread->ID);
	CurThread->SetState(Tasking::Waiting);

#ifdef DEBUG
	{
#if defined(a64)
		CPU::x64::CR0 cr0 = CPU::x64::readcr0();
		CPU::x64::CR2 cr2 = CPU::x64::CR2{.PFLA = CrashHandler::PageFaultAddress};
		CPU::x64::CR3 cr3 = CPU::x64::readcr3();
		CPU::x64::CR4 cr4 = CPU::x64::readcr4();
		CPU::x64::CR8 cr8 = CPU::x64::readcr8();
		CPU::x64::EFER efer;
		efer.raw = CPU::x64::rdmsr(CPU::x64::MSR_EFER);
		uintptr_t ds;
		asmv("mov %%ds, %0"
			 : "=r"(ds));
#elif defined(a32)
		CPU::x32::CR0 cr0 = CPU::x32::readcr0();
		CPU::x32::CR2 cr2 = CPU::x32::CR2{.PFLA = CrashHandler::PageFaultAddress};
		CPU::x32::CR3 cr3 = CPU::x32::readcr3();
		CPU::x32::CR4 cr4 = CPU::x32::readcr4();
		CPU::x32::CR8 cr8 = CPU::x32::readcr8();
		uintptr_t ds;
		asmv("mov %%ds, %0"
			 : "=r"(ds));
#elif defined(aa64)
#endif

#if defined(a64)
		debug("FS=%#lx GS=%#lx SS=%#lx CS=%#lx DS=%#lx",
			  CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE), CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE),
			  Frame->ss, Frame->cs, ds);
		debug("R8=%#lx R9=%#lx R10=%#lx R11=%#lx", Frame->r8, Frame->r9, Frame->r10, Frame->r11);
		debug("R12=%#lx R13=%#lx R14=%#lx R15=%#lx", Frame->r12, Frame->r13, Frame->r14, Frame->r15);
		debug("RAX=%#lx RBX=%#lx RCX=%#lx RDX=%#lx", Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
		debug("RSI=%#lx RDI=%#lx RBP=%#lx RSP=%#lx", Frame->rsi, Frame->rdi, Frame->rbp, Frame->rsp);
		debug("RIP=%#lx RFL=%#lx INT=%#lx ERR=%#lx EFER=%#lx", Frame->rip, Frame->rflags.raw, Frame->InterruptNumber, Frame->ErrorCode, efer.raw);
#elif defined(a32)
		debug("FS=%#x GS=%#x CS=%#x DS=%#x",
			  CPU::x32::rdmsr(CPU::x32::MSR_FS_BASE), CPU::x32::rdmsr(CPU::x32::MSR_GS_BASE),
			  Frame->cs, ds);
		debug("EAX=%#x EBX=%#x ECX=%#x EDX=%#x", Frame->eax, Frame->ebx, Frame->ecx, Frame->edx);
		debug("ESI=%#x EDI=%#x EBP=%#x ESP=%#x", Frame->esi, Frame->edi, Frame->ebp, Frame->esp);
		debug("EIP=%#x EFL=%#x INT=%#x ERR=%#x", Frame->eip, Frame->eflags.raw, Frame->InterruptNumber, Frame->ErrorCode);
#elif defined(aa64)
#endif

#if defined(a86)
		debug("CR0=%#lx CR2=%#lx CR3=%#lx CR4=%#lx CR8=%#lx", cr0.raw, cr2.raw, cr3.raw, cr4.raw, cr8.raw);

		debug("CR0: PE:%s MP:%s EM:%s TS:%s ET:%s NE:%s WP:%s AM:%s NW:%s CD:%s PG:%s R0:%#x R1:%#x R2:%#x",
			  cr0.PE ? "True " : "False", cr0.MP ? "True " : "False", cr0.EM ? "True " : "False", cr0.TS ? "True " : "False",
			  cr0.ET ? "True " : "False", cr0.NE ? "True " : "False", cr0.WP ? "True " : "False", cr0.AM ? "True " : "False",
			  cr0.NW ? "True " : "False", cr0.CD ? "True " : "False", cr0.PG ? "True " : "False",
			  cr0.Reserved0, cr0.Reserved1, cr0.Reserved2);

		debug("CR2: PFLA: %#lx",
			  cr2.PFLA);

		debug("CR3: PWT:%s PCD:%s PDBR:%#llx",
			  cr3.PWT ? "True " : "False", cr3.PCD ? "True " : "False", cr3.PDBR);
#endif // defined(a86)

#if defined(a64)
		debug("CR4: VME:%s PVI:%s TSD:%s DE:%s PSE:%s PAE:%s MCE:%s PGE:%s PCE:%s UMIP:%s OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s PKE:%s R0:%#x R1:%#x R2:%#x",
			  cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
			  cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
			  cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
			  cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
			  cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
			  cr4.Reserved0, cr4.Reserved1, cr4.Reserved2);
#elif defined(a32)
		debug("CR4: VME:%s PVI:%s TSD:%s DE:%s PSE:%s PAE:%s MCE:%s PGE:%s PCE:%s UMIP:%s OSFXSR:%s OSXMMEXCPT:%s    LA57:%s    VMXE:%s    SMXE:%s   PCIDE:%s OSXSAVE:%s    SMEP:%s    SMAP:%s PKE:%s R0:%#x R1:%#x",
			  cr4.VME ? "True " : "False", cr4.PVI ? "True " : "False", cr4.TSD ? "True " : "False", cr4.DE ? "True " : "False",
			  cr4.PSE ? "True " : "False", cr4.PAE ? "True " : "False", cr4.MCE ? "True " : "False", cr4.PGE ? "True " : "False",
			  cr4.PCE ? "True " : "False", cr4.UMIP ? "True " : "False", cr4.OSFXSR ? "True " : "False", cr4.OSXMMEXCPT ? "True " : "False",
			  cr4.LA57 ? "True " : "False", cr4.VMXE ? "True " : "False", cr4.SMXE ? "True " : "False", cr4.PCIDE ? "True " : "False",
			  cr4.OSXSAVE ? "True " : "False", cr4.SMEP ? "True " : "False", cr4.SMAP ? "True " : "False", cr4.PKE ? "True " : "False",
			  cr4.Reserved0, cr4.Reserved1);
#endif

#if defined(a86)
		debug("CR8: TPL:%d", cr8.TPL);
#endif // defined(a86)

#if defined(a64)
		debug("RFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x R3:%#x",
			  Frame->rflags.CF ? "True " : "False", Frame->rflags.PF ? "True " : "False", Frame->rflags.AF ? "True " : "False", Frame->rflags.ZF ? "True " : "False",
			  Frame->rflags.SF ? "True " : "False", Frame->rflags.TF ? "True " : "False", Frame->rflags.IF ? "True " : "False", Frame->rflags.DF ? "True " : "False",
			  Frame->rflags.OF ? "True " : "False", Frame->rflags.IOPL ? "True " : "False", Frame->rflags.NT ? "True " : "False", Frame->rflags.RF ? "True " : "False",
			  Frame->rflags.VM ? "True " : "False", Frame->rflags.AC ? "True " : "False", Frame->rflags.VIF ? "True " : "False", Frame->rflags.VIP ? "True " : "False",
			  Frame->rflags.ID ? "True " : "False", Frame->rflags.AlwaysOne,
			  Frame->rflags.Reserved0, Frame->rflags.Reserved1, Frame->rflags.Reserved2, Frame->rflags.Reserved3);
#elif defined(a32)
		debug("EFL: CF:%s PF:%s AF:%s ZF:%s SF:%s TF:%s IF:%s DF:%s OF:%s IOPL:%s NT:%s RF:%s VM:%s AC:%s VIF:%s VIP:%s ID:%s AlwaysOne:%d R0:%#x R1:%#x R2:%#x",
			  Frame->eflags.CF ? "True " : "False", Frame->eflags.PF ? "True " : "False", Frame->eflags.AF ? "True " : "False", Frame->eflags.ZF ? "True " : "False",
			  Frame->eflags.SF ? "True " : "False", Frame->eflags.TF ? "True " : "False", Frame->eflags.IF ? "True " : "False", Frame->eflags.DF ? "True " : "False",
			  Frame->eflags.OF ? "True " : "False", Frame->eflags.IOPL ? "True " : "False", Frame->eflags.NT ? "True " : "False", Frame->eflags.RF ? "True " : "False",
			  Frame->eflags.VM ? "True " : "False", Frame->eflags.AC ? "True " : "False", Frame->eflags.VIF ? "True " : "False", Frame->eflags.VIP ? "True " : "False",
			  Frame->eflags.ID ? "True " : "False", Frame->eflags.AlwaysOne,
			  Frame->eflags.Reserved0, Frame->eflags.Reserved1, Frame->eflags.Reserved2);
#elif defined(aa64)
#endif

#if defined(a64)
		debug("EFER: SCE:%s LME:%s LMA:%s NXE:%s SVME:%s LMSLE:%s FFXSR:%s TCE:%s R0:%#x R1:%#x R2:%#x",
			  efer.SCE ? "True " : "False", efer.LME ? "True " : "False", efer.LMA ? "True " : "False", efer.NXE ? "True " : "False",
			  efer.SVME ? "True " : "False", efer.LMSLE ? "True " : "False", efer.FFXSR ? "True " : "False", efer.TCE ? "True " : "False",
			  efer.Reserved0, efer.Reserved1, efer.Reserved2);
#endif
	}
#endif

	switch (Frame->InterruptNumber)
	{
	case CPU::x86::PageFault:
	{
		bool Handled = false;

		Handled = CurProc->vma->HandleCoW(CrashHandler::PageFaultAddress);
		if (!Handled)
			Handled = CurThread->Stack->Expand(CrashHandler::PageFaultAddress);

		if (Handled)
		{
			debug("Page fault handled");
			CurThread->SetState(Tasking::Ready);
			return true;
		}

		CurProc->Signals->SendSignal(SIGSEGV,
									 {Tasking::KILL_CRASH});
		break;
	}
	case CPU::x86::Debug:
	case CPU::x86::Breakpoint:
	{
		CurProc->Signals->SendSignal(SIGTRAP,
									 {Tasking::KILL_CRASH});
		break;
	}
	case CPU::x86::DivideByZero:
	case CPU::x86::Overflow:
	case CPU::x86::BoundRange:
	case CPU::x86::x87FloatingPoint:
	case CPU::x86::SIMDFloatingPoint:
	{
		CurProc->Signals->SendSignal(SIGFPE,
									 {Tasking::KILL_CRASH});
		break;
	}
	case CPU::x86::InvalidOpcode:
	case CPU::x86::GeneralProtectionFault:
	{
		CurProc->Signals->SendSignal(SIGILL,
									 {Tasking::KILL_CRASH});
		break;
	}
	case CPU::x86::DeviceNotAvailable:
	{
		CurProc->Signals->SendSignal(SIGBUS,
									 {Tasking::KILL_CRASH});
		break;
	}
	case CPU::x86::NonMaskableInterrupt:
	case CPU::x86::DoubleFault:
	case CPU::x86::CoprocessorSegmentOverrun:
	case CPU::x86::InvalidTSS:
	case CPU::x86::SegmentNotPresent:
	case CPU::x86::StackSegmentFault:
	case CPU::x86::AlignmentCheck:
	case CPU::x86::MachineCheck:
	case CPU::x86::Virtualization:
	case CPU::x86::Security:
	default:
	{
		error("Unhandled exception %d on CPU %d",
			  Frame->InterruptNumber, CurCPU->ID);
		break;
	}
	}

	error("User mode exception handler failed");
	return false;
}
