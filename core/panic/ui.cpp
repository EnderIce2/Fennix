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

#include <display.hpp>
#include <bitmap.hpp>
#include <convert.h>
#include <printf.h>
#include <lock.hpp>
#include <rand.hpp>
#include <uart.hpp>
#include <debug.h>
#include <smp.hpp>
#include <cpu.hpp>
#include <io.h>

#if defined(a64)
#include "../../arch/amd64/cpu/gdt.hpp"
#include "../arch/amd64/cpu/apic.hpp"
#elif defined(a32)
#include "../../arch/i386/cpu/gdt.hpp"
#include "../arch/i386/cpu/apic.hpp"
#elif defined(aa64)
#endif

#include "../../kernel.h"
#include "keyboard.hpp"

extern void ExPrint(const char *Format, ...);
extern void DiagnosticDataCollection();
extern void InitFont();

extern Video::Font *CrashFont;
extern void *FbBeforePanic;

struct StackFrame
{
	struct StackFrame *bp;
	uintptr_t ip;
};

struct x86ExceptionName
{
	const char *Mnemonic;
	const char *Name;
	const char *Cause;
};

/* AMD64 Programmer's Manual Volume 2: 8.2 Vectors */
x86ExceptionName x86Exceptions[] = {
	/* 0*/ {"#DE", "Divide-by-Zero-Error", "DIV, IDIV, AAM instructions"},
	/* 1*/ {"#DB", "Debug", "Instruction accesses and data accesses"},
	/* 2*/ {"NMI", "Non-Maskable-Interrupt", "Non-maskable interrupt"},
	/* 3*/ {"#BP", "Breakpoint", "INT3 instruction"},
	/* 4*/ {"#OF", "Overflow", "INTO instruction"},
	/* 5*/ {"#BR", "Bound-Range", "BOUND instruction"},
	/* 6*/ {"#UD", "Invalid-Opcode", "Invalid instructions"},
	/* 7*/ {"#NM", "Device-Not-Available", "x87 instructions"},
	/* 8*/ {"#DF", "Double-Fault", "Exception during the handling of another exception or interrupt"},
	/* 9*/ {"#--", "Coprocessor-Segment-Overrun", "Unsupported (Reserved)"},
	/*10*/ {"#TS", "Invalid-TSS", "Task-state segment access and task switch"},
	/*11*/ {"#NP", "Segment-Not-Present", "Segment register loads"},
	/*12*/ {"#SS", "Stack", "SS register loads and stack references"},
	/*13*/ {"#GP", "General-Protection", "Memory accesses and protection checks"},
	/*14*/ {"#PF", "Page-Fault", "Memory accesses when paging enabled"},
	/*15*/ {"#r0", "Reserved", "Reserved"},
	/*16*/ {"#MF", "x87 Floating-Point Exception-Pending", "x87 floating-point instructions"},
	/*17*/ {"#AC", "Alignment-Check", "Misaligned memory accesses"},
	/*18*/ {"#MC", "Machine-Check", "Model specific"},
	/*19*/ {"#XF", "SIMD Floating-Point", "SSE floating-point instructions"},
	/*20*/ {"#VE", "Virtualization Exception", "Virtualization event"}, /* AMD says this is reserved */
	/*21*/ {"#CP", "Control-Protection Exception", "RET/IRET or other control transfer"},
	/*22*/ {"#r1", "Reserved", "Reserved"},
	/*23*/ {"#r2", "Reserved", "Reserved"},
	/*24*/ {"#r3", "Reserved", "Reserved"},
	/*25*/ {"#r4", "Reserved", "Reserved"},
	/*26*/ {"#r5", "Reserved", "Reserved"},
	/*27*/ {"#r6", "Reserved", "Reserved"},
	/*28*/ {"#HV", "Hypervisor Injection Exception", "Event injection"},
	/*29*/ {"#VC", "VMM Communication Exception", "Virtualization event"},
	/*30*/ {"#SX", "Security Exception", "Security-sensitive event in host"},
	/*31*/ {"#r7", "Reserved", "Reserved"},
};

static const char *x86PageFaultDescriptions[9] = {
	"Supervisor tried to read a non-present page entry\n",
	"Supervisor tried to read a page and caused a protection fault\n",
	"Supervisor tried to write to a non-present page entry\n",
	"Supervisor tried to write a page and caused a protection fault\n",
	"User tried to read a non-present page entry\n",
	"User tried to read a page and caused a protection fault\n",
	"User tried to write to a non-present page entry\n",
	"User tried to write a page and caused a protection fault\n",
	"One or more page directory entries contain reserved bits which are set to 1.\n"};

int ActiveScreen = 0;

nsa const char *ExGetKSymbol(CPU::ExceptionFrame *Frame)
{
	if (Frame->rip < (uintptr_t)&_kernel_start &&
		Frame->rip > (uintptr_t)&_kernel_end)
		return "<OUTSIDE KERNEL>";

	if (KernelSymbolTable)
#if defined(a64)
		return KernelSymbolTable->GetSymbol(Frame->rip);
#elif defined(a32)
		return KernelSymbolTable->GetSymbol(Frame->eip);
#elif defined(aa64)
		return KernelSymbolTable->GetSymbol(Frame->pc);
#endif
	return "<UNKNOWN>";
}

nsa char *TrimWhiteSpace(char *str)
{
	char *end;
	while (*str == ' ')
		str++;
	if (*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while (end > str && *end == ' ')
		end--;
	*(end + 1) = 0;
	return str;
}

nsa void ExDumpData(void *Address, unsigned long Length)
{
	ExPrint("\eAAAAAA-------------------------------------------------------------------------\n");
	Display->UpdateBuffer();
	unsigned char *AddressChar = (unsigned char *)Address;
	unsigned char Buffer[17];
	unsigned long Iterate;
	for (Iterate = 0; Iterate < Length; Iterate++)
	{
		if ((Iterate % 16) == 0)
		{
			if (Iterate != 0)
				ExPrint("  \e8A78FF%s\eAABBCC\n", Buffer);
			ExPrint("  \e9E9E9E%04x\eAABBCC ", Iterate);
			Display->UpdateBuffer();
		}
		ExPrint(" \e4287f5%02x\eAABBCC", AddressChar[Iterate]);
		if ((AddressChar[Iterate] < 0x20) || (AddressChar[Iterate] > 0x7E))
			Buffer[Iterate % 16] = '.';
		else
			Buffer[Iterate % 16] = AddressChar[Iterate];
		Buffer[(Iterate % 16) + 1] = '\0';
	}

	while ((Iterate % 16) != 0)
	{
		ExPrint("   ");
		Display->UpdateBuffer();
		Iterate++;
	}

	ExPrint("  \e8A78FF%s\eAAAAAA\n", Buffer);
	ExPrint("-------------------------------------------------------------------------\n\n.");
	Display->UpdateBuffer();
}

nsa void DisplayTopOverlay()
{
	Video::Font *f = Display->GetCurrentFont();
	Video::FontInfo fi = f->GetInfo();

	for (uint32_t i = 0; i < Display->GetWidth; i++)
	{
		for (uint32_t j = 0; j < fi.Height + 8; j++)
		{
			// uint32_t grayValue = 0x505050 - (j * 0x020202);
			// Display->SetPixel(i, j, grayValue);
			Display->SetPixel(i, j, 0x202020);
		}
	}

	for (uint32_t i = 0; i < Display->GetWidth; i++)
		Display->SetPixel(i, fi.Height + 8, 0x404040);

	Display->SetBufferCursor(8, (fi.Height + 8) / 6);

	/* This wouldn't have enough space for a 640x480 screen */
	// ExPrint("%sMAIN %sDETAILS %sSTACK %sPROCESS \eAAAAAA| ",
	// 		ActiveScreen == 0 ? "\e00AAFF" : "\eAAAAAA",
	// 		ActiveScreen == 1 ? "\e00AAFF" : "\eAAAAAA",
	// 		ActiveScreen == 2 ? "\e00AAFF" : "\eAAAAAA",
	// 		ActiveScreen == 3 ? "\e00AAFF" : "\eAAAAAA");

	ExPrint("%s %s %s %s \eAAAAAA| ",
			ActiveScreen == 0 ? "\eAAAAAA"
								"MAIN   "
							  : "\e5A5A5A"
								"M",
			ActiveScreen == 1 ? "\eAAAAAA"
								"DETAIL "
							  : "\e5A5A5A"
								"D",
			ActiveScreen == 2 ? "\eAAAAAA"
								"STACK  "
							  : "\e5A5A5A"
								"S",
			ActiveScreen == 3 ? "\eAAAAAA"
								"PROCESS"
							  : "\e5A5A5A"
								"P");

	ExPrint("%s %s %s | ", KERNEL_NAME, KERNEL_ARCH, KERNEL_VERSION);
	ExPrint("%ld | ", TO_MiB(KernelAllocator.GetFreeMemory()));
	ExPrint("%s %s", CPU::Hypervisor(), CPU::Vendor());

	Display->SetBufferCursor(0, fi.Height + 10);

	/* https://imgflip.com/i/77slbl */
	if ((Random::rand32() % 100) >= 98)
	{
		debug("Easter egg activated!");
		int BaseXOffset = Display->GetWidth - 14;
		int BaseYOffset = 8;
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 0, 0x21852E);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 0, 0x21852E);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 0, 0x21852E);
		Display->SetPixel(BaseXOffset + 7, BaseYOffset + 0, 0x21852E);

		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 7, BaseYOffset + 1, 0x21852E);
		Display->SetPixel(BaseXOffset + 8, BaseYOffset + 1, 0x21852E);

		Display->SetPixel(BaseXOffset + 1, BaseYOffset + 2, 0x21852E);
		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 2, 0x21852E);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 2, 0xFFFFFF);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 2, 0x000000);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 2, 0x21852E);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 2, 0xFFFFFF);
		Display->SetPixel(BaseXOffset + 7, BaseYOffset + 2, 0x000000);
		Display->SetPixel(BaseXOffset + 8, BaseYOffset + 2, 0x21852E);

		Display->SetPixel(BaseXOffset + 1, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 3, 0x21852E);
		Display->SetPixel(BaseXOffset + 7, BaseYOffset + 3, 0x21852E);

		Display->SetPixel(BaseXOffset + 0, BaseYOffset + 4, 0x21852E);
		Display->SetPixel(BaseXOffset + 1, BaseYOffset + 4, 0x21852E);
		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 4, 0x21852E);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 4, 0x21852E);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 4, 0xA84832);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 4, 0xA84832);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 4, 0xA84832);
		Display->SetPixel(BaseXOffset + 7, BaseYOffset + 4, 0xA84832);

		Display->SetPixel(BaseXOffset + 0, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 1, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 5, 0x21852E);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 5, 0x21852E);

		Display->SetPixel(BaseXOffset + 0, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 1, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 2, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 3, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 4, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 5, BaseYOffset + 6, 0x1216FF);
		Display->SetPixel(BaseXOffset + 6, BaseYOffset + 6, 0x1216FF);

		Display->UpdateBuffer();
	}
}

nsa void DisplayBottomOverlay()
{
	Video::Font *f = Display->GetCurrentFont();
	Video::FontInfo fi = f->GetInfo();

	for (uint32_t i = 0; i < Display->GetWidth; i++)
		for (uint32_t j = Display->GetHeight - fi.Height - 8; j < Display->GetHeight; j++)
			Display->SetPixel(i, j, 0x202020);

	for (uint32_t i = 0; i < Display->GetWidth; i++)
		Display->SetPixel(i, Display->GetHeight - fi.Height - 8, 0x404040);

	Display->SetBufferCursor(8, Display->GetHeight - fi.Height - 4);
	ExPrint("\eAAAAAA> \eFAFAFA");
}

nsa void DisplayMainScreen(CPU::ExceptionFrame *Frame)
{
	ExPrint("\n\eFAFAFAWe're sorry, but the system has encountered a critical error and needs to restart.\n");

	ExPrint("\nError: %s (%s 0x%x)\n",
#if defined(a86)
			x86Exceptions[Frame->InterruptNumber].Name,
			x86Exceptions[Frame->InterruptNumber].Mnemonic,
#elif defined(aa64)
#error "AA64 not implemented"
#endif
			Frame->InterruptNumber);
#if defined(a86)
	ExPrint("Cause: %s\n", x86Exceptions[Frame->InterruptNumber].Cause);
#endif
	ExPrint("Exception occurred in function %s (%#lx)\n",
			ExGetKSymbol(Frame),
#if defined(a64)
			Frame->rip);
#elif defined(a32)
			Frame->eip);
#elif defined(aa64)
			Frame->pc);
#endif

	CPUData *core = GetCurrentCPU();
	if (TaskManager)
	{
		ExPrint("Core: %d / pid: %d / tid: %d\n", core->ID,
				core->CurrentProcess->ID, core->CurrentThread->ID);
	}
	else if (core->IsActive)
		ExPrint("Core: %d\n", core->ID);

	ExPrint("\nWhat to do:\n");
	ExPrint("  1. Restart your device and see if the problem persists.\n");
	ExPrint("  2. Remove any newly installed hardware.\n");
	ExPrint("  3. Check for any available updates for your operating system.\n");
	ExPrint("  4. Uninstall any recently installed drivers or software.\n");
	ExPrint("  If none of the above steps resolve the issue, create a new issue on \e00AAFFhttps://github.com/Fennix-Project/Fennix\eFAFAFA for further assistance.\n");

	ExPrint("\nUse command 'diag' to create a diagnostic report.\n");
}

nsa void DisplayDetailsScreen(CPU::ExceptionFrame *Frame)
{
	ExPrint("\n\eFAFAFAException Frame:\n");
	ExPrint(" General Purpose Registers:\n\eAAAAAA");
	ExPrint("  RAX: %#lx  RBX: %#lx  RCX: %#lx  RDX: %#lx\n",
			Frame->rax, Frame->rbx, Frame->rcx, Frame->rdx);
	ExPrint("  RSI: %#lx  RDI: %#lx  R8: %#lx  R9: %#lx\n",
			Frame->rsi, Frame->rdi, Frame->r8, Frame->r9);
	ExPrint("  R10: %#lx  R11: %#lx  R12: %#lx  R13: %#lx\n",
			Frame->r10, Frame->r11, Frame->r12, Frame->r13);
	ExPrint("  R14: %#lx  R15: %#lx\n", Frame->r14, Frame->r15);

	ExPrint("\eFAFAFA Control Registers:\n\eAAAAAA");
	ExPrint("  CR0: %#lx  CR2: %#lx  CR3: %#lx  CR4: %#lx\n",
			Frame->cr0, Frame->cr2, Frame->cr3, Frame->cr4);
	ExPrint("  CR8: %#lx\n", Frame->cr8);

	ExPrint("\eFAFAFA Segment Registers:\n\eAAAAAA");
	ExPrint("  CS: %#lx  SS: %#lx  DS: %#lx  ES: %#lx  FS: %#lx  GS: %#lx\n",
			Frame->cs, Frame->ss, Frame->ds, Frame->es, Frame->fs, Frame->gs);

	ExPrint("\eFAFAFA Debug Registers:\n\eAAAAAA");
	ExPrint("  DR0: %#lx  DR1: %#lx  DR2: %#lx  DR3: %#lx\n",
			Frame->dr0, Frame->dr1, Frame->dr2, Frame->dr3);
	ExPrint("  DR6: %#lx  DR7: %#lx\n", Frame->dr6, Frame->dr7);

	ExPrint("\eFAFAFA Other:\n\eAAAAAA");
	ExPrint("  INT: %#lx  ERR: %#lx  RIP: %#lx  RFLAGS: %#lx\n",
			Frame->InterruptNumber, Frame->ErrorCode,
			Frame->rip, Frame->rflags.raw);
	ExPrint("  RSP: %#lx  RBP: %#lx\n",
			Frame->rsp, Frame->rbp);

	ExPrint("\eFAFAFAException Details:\n");
	switch (Frame->InterruptNumber)
	{
	case CPU::x86::PageFault:
	{
		CPU::x64::PageFaultErrorCode pfCode = {.raw = (uint32_t)Frame->ErrorCode};
		ExPrint("P:%d W:%d U:%d R:%d I:%d PK:%d SS:%d SGX:%d\n",
				pfCode.P, pfCode.W, pfCode.U, pfCode.R,
				pfCode.I, pfCode.PK, pfCode.SS, pfCode.SGX);

		ExPrint("%s", x86PageFaultDescriptions[Frame->ErrorCode & 0b111]);
		if (Frame->ErrorCode & 0x8)
			ExPrint("%s", x86PageFaultDescriptions[8]);
		break;
	}
	case CPU::x86::StackSegmentFault:
	case CPU::x86::GeneralProtectionFault:
	{
		CPU::x64::SelectorErrorCode sCode = {.raw = Frame->ErrorCode};
		ExPrint("Kernel performed an illegal operation.\n");
		ExPrint("External: %d\n", sCode.External);
		ExPrint("Table: %d\n", sCode.Table);
		ExPrint("Index: %#x\n", sCode.Idx);
		break;
	}
	default:
		ExPrint("No additional information available for this exception.\n");
		break;
	}

	if (!TaskManager)
		return;

	CPUData *core = GetCurrentCPU();
	Tasking::PCB *proc = core->CurrentProcess.load();
	Tasking::TCB *thread = core->CurrentThread.load();
	ExPrint("Exception in %s process %s(%d) thread %s(%d)\n",
			proc->Security.ExecutionMode == Tasking::User ? "user" : "kernel",
			proc->Name, proc->ID,
			thread->Name, thread->ID);
}

nsa void DisplayStackScreen(CPU::ExceptionFrame *Frame)
{
	Memory::Virtual vmm;
	struct StackFrame *sf;
#if defined(a64)
	sf = (struct StackFrame *)Frame->rbp;
#elif defined(a32)
	sf = (struct StackFrame *)Frame->ebp;
#endif

	ExPrint("\n\eFAFAFAStack trace (%#lx):\n", sf);

	if (!vmm.Check(sf))
	{
		void *ptr = ((Memory::PageTable *)Frame->cr3)->Get(sf);
		debug("Virtual pointer %#lx -> %#lx", sf, ptr);
		if (vmm.Check(ptr))
			sf = (struct StackFrame *)ptr;
		else
		{
			ExPrint("\eFF0000< No stack trace available. >\n");
			return;
		}
	}

	/* FIXME: Get symbol offset more efficiently */

	uintptr_t fIP;
#if defined(a64)
	fIP = Frame->rip;
#elif defined(a32)
	fIP = Frame->eip;
#elif defined(aa64)
	fIP = Frame->pc;
#endif

	ExPrint("\eAAAAAA%p", (void *)fIP);
	ExPrint("\e5A5A5A in ");
	if ((fIP >= (uintptr_t)&_kernel_start &&
		 fIP <= (uintptr_t)&_kernel_end))
	{
		const char *sym = KernelSymbolTable->GetSymbol(fIP);
		ssize_t offset = fIP - KernelSymbolTable->GetSymbol(sym);
		if (offset < 0)
			offset = -offset;

		ExPrint("\eFAFAFA%s\e5A5A5A+%#lx \eFFAAAA<- Exception\n",
				sym, offset);
	}
	else
		ExPrint("\eFF5555???\n");

	if (!sf || !sf->ip || !sf->bp)
	{
		ExPrint("\n\eFF0000< No stack trace available. >\n");
		return;
	}

	for (int i = 0; i < 16; ++i)
	{
		if (!sf->ip)
			break;

		ExPrint("\eAAAAAA%p", (void *)sf->ip);
		ExPrint("\e5A5A5A in ");
		if ((sf->ip >= (uintptr_t)&_kernel_start &&
			 sf->ip <= (uintptr_t)&_kernel_end))
		{
			const char *sym = KernelSymbolTable->GetSymbol(sf->ip);
			ssize_t offset = sf->ip - KernelSymbolTable->GetSymbol(sym);
			if (offset < 0)
				offset = -offset;

			ExPrint("\eFAFAFA%s\e5A5A5A+%#lx\n", sym, offset);
		}
		else
			ExPrint("\eFF5555???\n");

		if (!vmm.Check(sf->bp))
			return;
		sf = sf->bp;
	}
}

nsa void DisplayProcessScreen(CPU::ExceptionFrame *Frame, bool IgnoreReady = true)
{
	const char *StatusColor[] = {
		"FF0000", // Unknown
		"AAFF00", // Ready
		"00AA00", // Running
		"FFAA00", // Sleeping
		"FFAA00", // Blocked
		"FFAA00", // Stopped
		"FFAA00", // Waiting

		"FF00FF", // Core dump
		"FF0088", // Zombie
		"FF0000", // Terminated
	};

	const char *StatusString[] = {
		"UNK", // Unknown
		"RDY", // Ready
		"RUN", // Running
		"SLP", // Sleeping
		"BLK", // Blocked
		"STP", // Stopped
		"WTG", // Waiting

		"CRD", // Core dump
		"ZMB", // Zombie
		"TRM", // Terminated
	};

	if (!TaskManager)
	{
		ExPrint("\eFF5555Tasking is not initialized\n");
		return;
	}

	size_t textLimit = 32;
	if (Display->GetWidth > 800 && Display->GetHeight > 600)
		textLimit = 128;

	std::list<Tasking::PCB *> Plist = TaskManager->GetProcessList();

	ExPrint("\n\eFAFAFAProcess list (%ld):\n", Plist.size());
	bool pRdy = false;
	bool showNote = false;
	/* FIXME: This is slow */
	foreach (auto Process in Plist)
	{
		bool ignore = true;
		if (Process->State == Tasking::Ready && IgnoreReady)
		{
			foreach (auto Thread in Process->Threads)
			{
				if (Thread->State == Tasking::Ready)
					continue;
				ignore = false;
				break;
			}

			if (ignore)
			{
				pRdy = true;
				debug("Ignoring ready process %s(%d)",
					  Process->Name, Process->ID);
				showNote = true;
				continue;
			}
		}

		ExPrint("\eAAAAAA-> \eFAFAFA%.*s%s\e8A8A8A(%ld) \e%s%s\n",
				textLimit, Process->Name,
				strlen(Process->Name) > textLimit ? "\e8A8A8A..." : "",
				Process->ID, StatusColor[Process->State.load()],
				StatusString[Process->State.load()]);

		bool tRdy = false;
		foreach (auto Thread in Process->Threads)
		{
			if (Thread->State == Tasking::Ready && IgnoreReady)
			{
				tRdy = true;
				debug("Ignoring ready thread %s(%d)",
					  Thread->Name, Thread->ID);
				showNote = true;
				continue;
			}

			ExPrint("\eAAAAAA  -> \eFAFAFA%.*s%s\e8A8A8A(%ld) \e%s%s\n",
					textLimit, Thread->Name,
					strlen(Thread->Name) > textLimit ? "\e8A8A8A..." : "",
					Thread->ID, StatusColor[Thread->State.load()],
					StatusString[Thread->State.load()]);
		}
		if (tRdy)
			ExPrint("\eAAAAAA%s  -> \e8A8A8A...\n");
	}
	if (pRdy)
		ExPrint("\eAAAAAA-> \e8A8A8A...\n");

	if (showNote)
		ExPrint("\e5A5A5ANote: Some processes may not be displayed.\n");
}

CPU::ExceptionFrame *ExFrame;
nsa void DisplayCrashScreen(CPU::ExceptionFrame *Frame)
{
	ExFrame = Frame;

	/* TODO */
	// void *BufferBeforeUpdate = KernelAllocator.RequestPages(TO_PAGES(Display->GetSize));
	// if (BufferBeforeUpdate)
	// 	memcpy(BufferBeforeUpdate, Display->GetBuffer, Display->GetSize);

	Display->ClearBuffer();
	if (Config.InterruptsOnCrash == false)
	{
		Display->SetBufferCursor(0, 0);
		DisplayMainScreen(Frame);
		Display->UpdateBuffer();
		CPU::Stop();
	}

	DisplayTopOverlay();

	DisplayMainScreen(Frame);

	Display->UpdateBuffer();
	new CrashKeyboardDriver;

	DisplayBottomOverlay();
	Display->UpdateBuffer();
	CPU::Halt(true);
}

nsa void DisplayStackSmashing()
{
	InitFont();
	Display->ClearBuffer();
	DisplayTopOverlay();

	ExPrint("\n\eFAFAFAWe're sorry, but the system has encountered a critical error and needs to restart.\n");
	ExPrint("\nError: Stack Smashing In Kernel Mode\n");

	CPUData *core = GetCurrentCPU();
	if (TaskManager)
	{
		ExPrint("Core: %d / pid: %d / tid: %d\n", core->ID,
				core->CurrentProcess->ID, core->CurrentThread->ID);
	}
	else if (core->IsActive)
		ExPrint("Core: %d\n", core->ID);

	ExPrint("\nWhat to do:\n");
	ExPrint(" This is a kernel bug.\n");
	ExPrint(" Please create a new issue on \e00AAFFhttps://github.com/Fennix-Project/Fennix\eFAFAFA for further assistance.\n");

	Display->UpdateBuffer();

	/* TODO: Add additional info */
}

nsa void DisplayBufferOverflow()
{
	InitFont();
	Display->ClearBuffer();
	DisplayTopOverlay();

	ExPrint("\n\eFAFAFAWe're sorry, but the system has encountered a critical error and needs to restart.\n");
	ExPrint("\nError: Buffer Overflow In Kernel Mode\n");

	CPUData *core = GetCurrentCPU();
	if (TaskManager)
	{
		ExPrint("Core: %d / pid: %d / tid: %d\n", core->ID,
				core->CurrentProcess->ID, core->CurrentThread->ID);
	}
	else if (core->IsActive)
		ExPrint("Core: %d\n", core->ID);

	ExPrint("\nWhat to do:\n");
	ExPrint(" This is a kernel bug.\n");
	ExPrint(" Please create a new issue on \e00AAFFhttps://github.com/Fennix-Project/Fennix\eFAFAFA for further assistance.\n");

	Display->UpdateBuffer();

	/* TODO: Add additional info */
}

EXTERNC nsa __noreturn void DisplayAssertionFailed(const char *File, int Line,
												   const char *Expression)
{
	InitFont();
	Display->ClearBuffer();
	DisplayTopOverlay();

	ExPrint("\n\eFAFAFAWe're sorry, but the system has encountered a critical error and needs to restart.\n");
	ExPrint("\nError: Assertion Failed\n");
	ExPrint("In file \"%s\" at line %d, \"%s\" failed\n",
			File, Line, Expression);

	CPUData *core = GetCurrentCPU();
	if (TaskManager)
	{
		ExPrint("Core: %d / pid: %d / tid: %d\n", core->ID,
				core->CurrentProcess->ID, core->CurrentThread->ID);
	}
	else if (core->IsActive)
		ExPrint("Core: %d\n", core->ID);

	ExPrint("\nWhat to do:\n");
	ExPrint(" This is a kernel bug.\n");
	ExPrint(" Please create a new issue on \e00AAFFhttps://github.com/Fennix-Project/Fennix\eFAFAFA for further assistance.\n");

	Display->UpdateBuffer();
	CPU::Stop();
}

nsa void ArrowInput(uint8_t key)
{
	switch (key)
	{
	case KEY_D_RIGHT:
		if (ActiveScreen < 3)
			ActiveScreen++;
		else
			return;
		break;
	case KEY_D_LEFT:
		if (ActiveScreen > 0)
			ActiveScreen--;
		else
			return;
		break;
	case KEY_D_DOWN:
	case KEY_D_UP:
	default:
		return;
	}

	Display->ClearBuffer();
	DisplayTopOverlay();

	switch (ActiveScreen)
	{
	case 0:
		DisplayMainScreen(ExFrame);
		break;
	case 1:
		DisplayDetailsScreen(ExFrame);
		break;
	case 2:
		DisplayStackScreen(ExFrame);
		break;
	case 3:
		DisplayProcessScreen(ExFrame);
		break;
	default:
		break;
	}

	DisplayBottomOverlay();
	Display->UpdateBuffer();
}

nsa void UserInput(char *Input)
{
	debug("User input: %s", Input);
	Display->ClearBuffer();
	DisplayTopOverlay();

	if (strcmp(Input, "help") == 0)
	{
		ExPrint("Commands:\n");
		ExPrint("\eAAAAAA  help              - Display this help message.\n");
		ExPrint("\eCACACA  clear             - Clear the screen.\n");
		ExPrint("\eAAAAAA  exit              - Shutdown the device.\n");
		ExPrint("\eCACACA  reboot            - Reboot the device.\n");
		ExPrint("\eAAAAAA  bitmap            - Display the kernel's bitmap.\n");
		ExPrint("\eCACACA  mem               - Display memory information.\n");
		ExPrint("\eAAAAAA  dump [addr] [len] - Dump [len] bytes from [addr].\n");
		ExPrint("\eCACACA  diag			     - Collect diagnostic information.\n");
		ExPrint("\eAAAAAA  screen            - Display the final output prior to system panic.\n");
	}
	else if (strcmp(Input, "clear") == 0)
	{
		Display->ClearBuffer();
		DisplayTopOverlay();
	}
	else if (strcmp(Input, "exit") == 0)
	{
		Display->ClearBuffer();

		const char msg[] = "Shutting down...";
		size_t msgLen = strlen(msg);
		size_t msgPixels = msgLen * CrashFont->GetInfo().Width;
		uint32_t x = uint32_t((Display->GetWidth - msgPixels) / 2);
		uint32_t y = uint32_t((Display->GetHeight - CrashFont->GetInfo().Height) / 2);
		Display->SetBufferCursor(x, y);
		Display->PrintString("\eAAAAAA");
		Display->PrintString(msg, CrashFont);

		Display->UpdateBuffer();
		PowerManager->Shutdown();
		CPU::Stop();
	}
	else if (strcmp(Input, "reboot") == 0)
	{
		Display->ClearBuffer();

		const char msg[] = "Rebooting...";
		size_t msgLen = strlen(msg);
		size_t msgPixels = msgLen * CrashFont->GetInfo().Width;
		uint32_t x = uint32_t((Display->GetWidth - msgPixels) / 2);
		uint32_t y = uint32_t((Display->GetHeight - CrashFont->GetInfo().Height) / 2);
		Display->SetBufferCursor(x, y);
		Display->PrintString("\eAAAAAA");
		Display->PrintString(msg, CrashFont);

		Display->UpdateBuffer();
		PowerManager->Reboot();
		CPU::Stop();
	}
	else if (strncmp(Input, "bitmap", 6) == 0)
	{
		Bitmap bm = KernelAllocator.GetPageBitmap();
		Video::Font *oldFont = CrashFont;
		CrashFont = Display->GetDefaultFont();

		ExPrint("\n\eAAAAAA[0%%] %08ld: ", 0);
		for (size_t i = 0; i < bm.Size; i++)
		{
			if (bm.Get(i))
				Display->PrintString("\eFF0000");
			else
				Display->PrintString("\e00FF00");
			Display->Print('0');
			if (i % 128 == 127)
			{
				short Percentage = s_cst(short, (i * 100) / bm.Size);
				ExPrint("\n\eAAAAAA[%03ld%%] %08ld: ", Percentage, i);
				Display->UpdateBuffer();
			}
		}
		ExPrint("\n\eAAAAAA--- END OF BITMAP ---\nBitmap size: %ld\n\n.", bm.Size);
		DisplayTopOverlay();
		Display->UpdateBuffer();
		CrashFont = oldFont;
	}
	else if (strcmp(Input, "mem") == 0)
	{
		uint64_t Total = KernelAllocator.GetTotalMemory();
		uint64_t Used = KernelAllocator.GetUsedMemory();
		uint64_t Free = KernelAllocator.GetFreeMemory();
		uint64_t Reserved = KernelAllocator.GetReservedMemory();

		ExPrint("\e22AA44Total: %ld bytes\n\eFF0000Used: %ld bytes\n\e00FF00Free: %ld bytes\n\eFF00FFReserved: %ld bytes\n", Total, Used, Free, Reserved);
		int Progress = s_cst(int, (Used * 100) / Total);
		int ReservedProgress = s_cst(int, (Reserved * 100) / Total);
		ExPrint("\e22AA44%3d%% \eCCCCCC[", Progress);
		for (int i = 0; i < Progress; i++)
			ExPrint("\eFF0000|");
		for (int i = 0; i < 100 - Progress; i++)
			ExPrint("\e00FF00|");
		for (int i = 0; i < ReservedProgress; i++)
			ExPrint("\eFF00FF|");
		ExPrint("\eCCCCCC]\eAAAAAA\n");
	}
	else if (strncmp(Input, "dump", 4) == 0)
	{
		char *arg = TrimWhiteSpace(Input + 4);
		char *addr = strtok(arg, " ");
		char *len = strtok(NULL, " ");
		if (addr == NULL || len == NULL)
		{
			ExPrint("\eFF0000Invalid arguments\n");
			goto End;
		}

		uintptr_t Address = strtoul(addr, NULL, 16);
		size_t Length = strtoul(len, NULL, 10);

		uintptr_t AlignedAddress = ROUND_DOWN(Address, PAGE_SIZE);
		bool IsRangeValid = true;
		{
			Memory::Virtual vmm;
			for (uintptr_t adr = AlignedAddress;
				 adr < Address + Length;
				 adr += PAGE_SIZE)
			{
				if (!vmm.Check((void *)adr))
				{
					ExPrint("\eFFA500Address %#lx is not mapped\n", adr);
					IsRangeValid = false;
				}
			}
		}

		if (IsRangeValid)
		{
			debug("Dumping %ld bytes from %#lx\n", Length, Address);

			Video::Font *oldFont = CrashFont;
			CrashFont = Display->GetDefaultFont();
			ExDumpData((void *)Address, (unsigned long)Length);
			CrashFont = oldFont;
		}
	}
	else if (strcmp(Input, "diag") == 0)
	{
		DiagnosticDataCollection();
	}
	else if (strcmp(Input, "screen") == 0)
	{
		if (unlikely(FbBeforePanic == nullptr))
		{
			ExPrint("\eFF0000No screen data available\n");
			goto End;
		}
		memcpy(Display->GetBuffer, FbBeforePanic, Display->GetSize);
		Display->UpdateBuffer();
		return;
	}
#ifdef DEBUG
	else if (strcmp(Input, "pt") == 0)
	{
		/* Helpful for qemu "info tlb" command */
		CPU::PageTable((void *)ExFrame->cr3);
		ExPrint("Here be dragons\n");
	}
	else if (strcmp(Input, "ps") == 0)
	{
		DisplayProcessScreen(ExFrame, false);
	}
#endif // DEBUG
	else if (strlen(Input) > 0)
		ExPrint("Unknown command: %s", Input);
	else
		ExPrint("Use the 'help' command to display a list of available commands.");

End:
	DisplayBottomOverlay();
	Display->UpdateBuffer();
}
