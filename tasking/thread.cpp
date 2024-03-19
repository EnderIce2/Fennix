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

#include <task.hpp>

#include <filesystem/ioctl.hpp>
#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>
#include <io.h>

#include "../kernel.h"

#if defined(a64)
#include "../arch/amd64/cpu/apic.hpp"
#include "../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../arch/i386/cpu/apic.hpp"
#include "../arch/i386/cpu/gdt.hpp"
#elif defined(aa64)
#endif

// #define DEBUG_TASKING 1

#ifdef DEBUG_TASKING
#define tskdbg(m, ...)       \
	debug(m, ##__VA_ARGS__); \
	__sync
#else
#define tskdbg(m, ...)
#endif

/* For kernel threads only */
void ThreadDoExit()
{
	CPUData *CPUData = GetCurrentCPU();
	Tasking::TCB *CurrentThread = CPUData->CurrentThread.load();
	CurrentThread->SetState(Tasking::Terminated);

	debug("\"%s\"(%d) exited with code: %#lx",
		  CurrentThread->Name,
		  CurrentThread->ID,
		  CurrentThread->ExitCode);
	CPU::Halt(true);
}

namespace Tasking
{
	int TCB::SendSignal(int sig)
	{
		return this->Parent->Signals->SendSignal(sig);
	}

	void TCB::SetState(TaskState state)
	{
		this->State.store(state);
		if (this->Parent->Threads.size() == 1)
			this->Parent->State.store(state);
	}

	void TCB::SetExitCode(int code)
	{
		this->ExitCode.store(code);
		if (this->Parent->Threads.size() == 1)
			this->Parent->ExitCode.store(code);
	}

	void TCB::Rename(const char *name)
	{
		assert(name != nullptr);
		assert(strlen(name) > 0);

		trace("Renaming thread %s to %s",
			  this->Name, name);

		if (this->Name)
		{
			this->AllocatedMemory -= strlen(this->Name) + 1;
			delete[] this->Name;
		}

		this->Name = new char[strlen(name) + 1];
		this->AllocatedMemory += strlen(name) + 1;
		strcpy((char *)this->Name, name);
	}

	void TCB::SetPriority(TaskPriority priority)
	{
		assert(priority >= _PriorityMin);
		// assert(priority <= _PriorityMax);

		trace("Setting priority of thread %s to %d",
			  this->Name, priority);

		Info.Priority = priority;
	}

	void TCB::SetCritical(bool Critical)
	{
		trace("Setting criticality of thread %s to %s",
			  this->Name, Critical ? "true" : "false");

		Security.IsCritical = Critical;

		if (this->Parent->Threads.size() == 1)
			this->Parent->Security.IsCritical = Critical;
	}

	void TCB::SetDebugMode(bool Enable)
	{
		trace("Setting debug mode of thread %s to %s",
			  this->Name, Enable ? "true" : "false");

		Security.IsDebugEnabled = Enable;

		if (this->Parent->Threads.size() == 1)
			this->Parent->Security.IsDebugEnabled = Enable;
	}

	void TCB::SetKernelDebugMode(bool Enable)
	{
		trace("Setting kernel debug mode of thread %s to %s",
			  this->Name, Enable ? "true" : "false");

		Security.IsKernelDebugEnabled = Enable;

		if (this->Parent->Threads.size() == 1)
			this->Parent->Security.IsKernelDebugEnabled = Enable;
	}

	size_t TCB::GetSize()
	{
		size_t ret = this->AllocatedMemory;
		ret += this->vma->GetAllocatedMemorySize();
		ret += this->Stack->GetSize();
		return ret;
	}

	void TCB::SYSV_ABI_Call(uintptr_t Arg1, uintptr_t Arg2,
							uintptr_t Arg3, uintptr_t Arg4,
							uintptr_t Arg5, uintptr_t Arg6,
							void *Function)
	{
#if defined(a64)
		this->Registers.rdi = Arg1;
		this->Registers.rsi = Arg2;
		this->Registers.rdx = Arg3;
		this->Registers.rcx = Arg4;
		this->Registers.r8 = Arg5;
		this->Registers.r9 = Arg6;
		if (Function != nullptr)
			this->Registers.rip = (uint64_t)Function;
#elif defined(a32)
		this->Registers.eax = Arg1;
		this->Registers.ebx = Arg2;
		this->Registers.ecx = Arg3;
		this->Registers.edx = Arg4;
		this->Registers.esi = Arg5;
		this->Registers.edi = Arg6;
		if (Function != nullptr)
			this->Registers.eip = (uint32_t)Function;
#else
#warning "SYSV ABI not implemented for this architecture"
#endif
	}

	__no_sanitize("undefined") void TCB::SetupUserStack_x86_64(const char **argv,
															   const char **envp,
															   const std::vector<AuxiliaryVector> &auxv,
															   TaskCompatibility Compatibility)
	{
		size_t argvLen = 0;
		if (argv)
			while (argv[argvLen] != nullptr)
				argvLen++;

		size_t envpLen = 0;
		if (envp)
			while (envp[envpLen] != nullptr)
				envpLen++;

		debug("argvLen: %d", argvLen);
		debug("envpLen: %d", envpLen);

		/* https://articles.manugarg.com/aboutelfauxiliaryvectors.html */
		/* https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf#figure.3.9 */
		/* rsp is the top of the stack */
		char *pStack = (char *)this->Stack->GetStackPhysicalTop();
		/* Temporary stack pointer for strings */
		char *stackStr = (char *)pStack;
		char *stackStrVirtual = (char *)this->Stack->GetStackTop();

		/* Store string pointers for later */
		uintptr_t *argvStrings = nullptr;
		uintptr_t *envpStrings = nullptr;
		if (argvLen > 0)
			argvStrings = new uintptr_t[argvLen];
		if (envpLen > 0)
			envpStrings = new uintptr_t[envpLen];

		for (size_t i = 0; i < argvLen; i++)
		{
			/* Subtract the length of the string and the null terminator */
			stackStr -= strlen(argv[i]) + 1;
			stackStrVirtual -= strlen(argv[i]) + 1;
			/* Store the pointer to the string */
			argvStrings[i] = (uintptr_t)stackStrVirtual;
			/* Copy the string to the stack */
			strcpy(stackStr, argv[i]);
			debug("argv[%d]: %s", i, argv[i]);
		}

		for (size_t i = 0; i < envpLen; i++)
		{
			/* Subtract the length of the string and the null terminator */
			stackStr -= strlen(envp[i]) + 1;
			stackStrVirtual -= strlen(envp[i]) + 1;
			/* Store the pointer to the string */
			envpStrings[i] = (uintptr_t)stackStrVirtual;
			/* Copy the string to the stack */
			strcpy(stackStr, envp[i]);
			debug("envp[%d]: %s", i, envp[i]);
		}

		/* Align the stack to 16 bytes */
		stackStr -= (uintptr_t)stackStr & 0xF;
		/* Set "pStack" to the new stack pointer */
		pStack = (char *)stackStr;
		/* If argv and envp sizes are odd then we need to align the stack */
		pStack -= (argvLen + envpLen) % 2;
		debug("odd align: %#lx | %#lx -> %#lx",
			  (argvLen + envpLen) % 2,
			  stackStr, pStack);

		/* Ensure StackPointerReg is aligned to the closest lower 16 bytes boundary */
		uintptr_t lower16Align = (uintptr_t)pStack;
		lower16Align &= ~0xF;
		debug("before: %#lx ; after: %#lx", pStack, lower16Align);
		pStack = (char *)lower16Align;

		/* We need 8 bit pointers for the stack from here */
		uintptr_t *Stack64 = (uintptr_t *)pStack;
		assert(Stack64 != nullptr);

		/* Store the null terminator */
		StackPush(Stack64, uintptr_t, AT_NULL);

		/* auxv_array is initialized with auxv elements.
			If the array is empty then we add a null terminator */
		std::vector<AuxiliaryVector> auxv_array = auxv;
		if (auxv_array.size() == 0)
			auxv_array.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});

		/* Store auxillary vector */
		foreach (AuxiliaryVector av in auxv_array)
		{
			/* Subtract the size of the auxillary vector */
			Stack64 -= sizeof(Elf_auxv_t) / sizeof(uintptr_t);
			/* Store the auxillary vector */
			POKE(Elf_auxv_t, Stack64) = av.archaux;
			/* TODO: Store strings to the stack */
		}

		/* Store the null terminator */
		StackPush(Stack64, uintptr_t, AT_NULL);

		/* Store envpStrings[] to the stack */
		Stack64 -= envpLen; /* (1 Stack64 = 8 bits; Stack64 = 8 * envpLen) */
		for (size_t i = 0; i < envpLen; i++)
		{
			*(Stack64 + i) = (uintptr_t)envpStrings[i];
			debug("envpStrings[%d]: %#lx",
				  i, envpStrings[i]);
		}

		/* Store the null terminator */
		StackPush(Stack64, uintptr_t, AT_NULL);

		/* Store argvStrings[] to the stack */
		Stack64 -= argvLen; /* (1 Stack64 = 8 bits; Stack64 = 8 * argvLen) */
		for (size_t i = 0; i < argvLen; i++)
		{
			*(Stack64 + i) = (uintptr_t)argvStrings[i];
			debug("argvStrings[%d]: %#lx",
				  i, argvStrings[i]);
		}

		/* Store the argc */
		StackPush(Stack64, uintptr_t, argvLen);

		/* Set "pStack" to the new stack pointer */
		pStack = (char *)Stack64;

		/* We need the virtual address but because we are in the kernel we can't use the process page table.
			So we modify the physical address and store how much we need to subtract to get the virtual address for RSP. */
		uintptr_t SubtractStack = (uintptr_t)this->Stack->GetStackPhysicalTop() - (uintptr_t)pStack;
		debug("SubtractStack: %#lx", SubtractStack);

		/* Set the stack pointer to the new stack */
		uintptr_t StackPointerReg = ((uintptr_t)this->Stack->GetStackTop() - SubtractStack);
		// assert(!(StackPointerReg & 0xF));

#if defined(a64)
		this->Registers.rsp = StackPointerReg;
#elif defined(a32)
		this->Registers.esp = StackPointerReg;
#elif defined(aa64)
		this->Registers.sp = StackPointerReg;
#endif

		if (argvLen > 0)
			delete[] argvStrings;
		if (envpLen > 0)
			delete[] envpStrings;

#ifdef DEBUG
		DumpData("Stack Data",
				 (void *)((uintptr_t)this->Stack->GetStackPhysicalTop() - (uintptr_t)SubtractStack),
				 SubtractStack);
#endif

		if (Compatibility != Native)
			return;

#if defined(a64)
		this->Registers.rdi = (uintptr_t)argvLen;										// argc
		this->Registers.rsi = (uintptr_t)(this->Registers.rsp + 8);						// argv
		this->Registers.rcx = (uintptr_t)envpLen;										// envc
		this->Registers.rdx = (uintptr_t)(this->Registers.rsp + 8 + (8 * argvLen) + 8); // envp
#elif defined(a32)
		this->Registers.eax = (uintptr_t)argvLen;										// argc
		this->Registers.ebx = (uintptr_t)(this->Registers.esp + 4);						// argv
		this->Registers.ecx = (uintptr_t)envpLen;										// envc
		this->Registers.edx = (uintptr_t)(this->Registers.esp + 4 + (4 * argvLen) + 4); // envp
#elif defined(aa64)
		this->Registers.x0 = (uintptr_t)argvLen;									  // argc
		this->Registers.x1 = (uintptr_t)(this->Registers.sp + 8);					  // argv
		this->Registers.x2 = (uintptr_t)envpLen;									  // envc
		this->Registers.x3 = (uintptr_t)(this->Registers.sp + 8 + (8 * argvLen) + 8); // envp
#endif
	}

	void TCB::SetupUserStack_x86_32(const char **argv,
									const char **envp,
									const std::vector<AuxiliaryVector> &auxv,
									TaskCompatibility Compatibility)
	{
		fixme("Not implemented");
	}

	void TCB::SetupUserStack_aarch64(const char **argv,
									 const char **envp,
									 const std::vector<AuxiliaryVector> &auxv,
									 TaskCompatibility Compatibility)
	{
		fixme("Not implemented");
	}

	void TCB::SetupThreadLocalStorage()
	{
		if (Parent->TLS.pBase == 0x0)
			return;
		this->TLS = Parent->TLS;

		debug("msz: %ld fsz: %ld",
			  this->TLS.Size, this->TLS.fSize);

		// size_t pOffset = this->TLS.vBase - std::tolower(this->TLS.vBase);
		size_t tlsFullSize = sizeof(uintptr_t) + this->TLS.Size;
		/* 2 guard pages */
		size_t tlsPages = 1 + TO_PAGES(tlsFullSize) + 1;

		void *opTLS = this->vma->RequestPages(tlsPages);
		void *pTLS = (void *)(PAGE_SIZE + uintptr_t(opTLS));
		this->TLS.pBase = uintptr_t(pTLS);

		memcpy(pTLS, (void *)this->TLS.pBase,
			   this->TLS.Size);

		size_t restBytes = this->TLS.Size - this->TLS.fSize;
		if (restBytes)
		{
			memset((void *)(uintptr_t(pTLS) + this->TLS.Size),
				   0, restBytes);
		}

		Memory::Virtual vmm(this->Parent->PageTable);
		/* Map guard pages */
		vmm.Remap((void *)(uintptr_t(pTLS) - PAGE_SIZE), opTLS, Memory::P);
		vmm.Remap((void *)(uintptr_t(pTLS) + this->TLS.Size), opTLS, Memory::P);
		/* Map TLS */
		vmm.Map(pTLS, pTLS, this->TLS.Size, Memory::RW | Memory::US);

		uintptr_t *pTLSPointer = (uintptr_t *)this->TLS.pBase + this->TLS.Size;
		*pTLSPointer = this->TLS.pBase + this->TLS.Size;

		this->GSBase = r_cst(uintptr_t, pTLSPointer);
		this->FSBase = r_cst(uintptr_t, pTLSPointer);
	}

	TCB::TCB(Task *ctx, PCB *Parent, IP EntryPoint,
			 const char **argv, const char **envp,
			 const std::vector<AuxiliaryVector> &auxv,
			 TaskArchitecture Architecture,
			 TaskCompatibility Compatibility,
			 bool ThreadNotReady)
	{
		debug("+ %#lx", this);

		assert(ctx != nullptr);
		assert(Architecture >= _ArchitectureMin);
		assert(Architecture <= _ArchitectureMax);
		assert(Compatibility >= _CompatibilityMin);
		assert(Compatibility <= _CompatibilityMax);

		if (Parent == nullptr)
		{
			this->Parent = ctx->GetCurrentProcess();
			assert(this->Parent != nullptr);
		}
		else
			this->Parent = Parent;

		this->ctx = ctx;
		this->ID = (TID)this->Parent->ID + (TID)this->Parent->Threads.size();

		if (this->Name)
			delete[] this->Name;

		this->Name = new char[strlen(this->Parent->Name) + 1];
		strcpy((char *)this->Name, this->Parent->Name);

		this->EntryPoint = EntryPoint;
		this->ExitCode = KILL_CRASH;

		if (ThreadNotReady)
			this->SetState(Waiting);
		else
			this->SetState(Ready);

		this->vma = this->Parent->vma;

#if defined(a64)
		this->Registers.rip = EntryPoint;
#elif defined(a32)
		this->Registers.eip = EntryPoint;
#elif defined(aa64)
		this->Registers.pc = EntryPoint;
#endif

		switch (this->Parent->Security.ExecutionMode)
		{
		case TaskExecutionMode::System:
			fixme("System mode not supported.");
			[[fallthrough]];
		case TaskExecutionMode::Kernel:
		{
			this->Security.IsCritical = true;
			this->Stack = new Memory::StackGuard(false, this->vma);

#if defined(a64)
			this->ShadowGSBase =
				CPU::x64::rdmsr(CPU::x64::MSRID::MSR_SHADOW_GS_BASE);
			this->GSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_GS_BASE);
			this->FSBase = CPU::x64::rdmsr(CPU::x64::MSRID::MSR_FS_BASE);
			this->Registers.cs = GDT_KERNEL_CODE;
			this->Registers.ss = GDT_KERNEL_DATA;
			this->Registers.rflags.AlwaysOne = 1;
			this->Registers.rflags.IF = 1;
			this->Registers.rflags.ID = 1;
			this->Registers.rflags.AC = 0;
			this->Registers.rsp = ((uintptr_t)this->Stack->GetStackTop());
			POKE(uintptr_t, this->Registers.rsp) = (uintptr_t)ThreadDoExit;
#elif defined(a32)
			this->Registers.cs = GDT_KERNEL_CODE;
			this->Registers.r3_ss = GDT_KERNEL_DATA;
			this->Registers.eflags.AlwaysOne = 1;
			this->Registers.eflags.IF = 1;
			this->Registers.eflags.ID = 1;
			this->Registers.eflags.AC = 0;
			this->Registers.esp = ((uintptr_t)this->Stack->GetStackTop());
			POKE(uintptr_t, this->Registers.esp) = (uintptr_t)ThreadDoExit;
#elif defined(aa64)
			this->Registers.pc = EntryPoint;
			this->Registers.sp = ((uintptr_t)this->Stack->GetStackTop());
			POKE(uintptr_t, this->Registers.sp) = (uintptr_t)ThreadDoExit;
#endif
			break;
		}
		case TaskExecutionMode::User:
		{
			this->Stack = new Memory::StackGuard(true, this->vma);

			gsTCB *gsT = (gsTCB *)this->vma->RequestPages(TO_PAGES(sizeof(gsTCB)), false, true);
#ifdef DEBUG
			gsT->__stub = 0xFFFFFFFFFFFFFFFF;
#endif

			gsT->ScPages = TO_PAGES(STACK_SIZE);
			gsT->SyscallStackBase = this->vma->RequestPages(gsT->ScPages, false, true);
			gsT->SyscallStack = (void *)((uintptr_t)gsT->SyscallStackBase + STACK_SIZE - 0x10);
			debug("New syscall stack created: %#lx (base: %#lx) with gs base at %#lx",
				  gsT->SyscallStack, gsT->SyscallStackBase, gsT);

			gsT->TempStack = 0x0;
			gsT->t = this;
#if defined(a64)
			this->ShadowGSBase = (uintptr_t)gsT;
			this->GSBase = 0;
			this->FSBase = 0;
			this->Registers.cs = GDT_USER_CODE;
			this->Registers.ss = GDT_USER_DATA;
			this->Registers.rflags.AlwaysOne = 1;
			this->Registers.rflags.IF = 1;
			this->Registers.rflags.ID = 1;
			this->Registers.rflags.AC = 0;
			/* We need to leave the libc's crt
			   to make a syscall when the Thread
			   is exited or we are going to get
			   an exception. */

			this->SetupUserStack_x86_64(argv, envp, auxv, Compatibility);
#elif defined(a32)
			this->Registers.cs = GDT_USER_CODE;
			this->Registers.r3_ss = GDT_USER_DATA;
			this->Registers.eflags.AlwaysOne = 1;
			this->Registers.eflags.IF = 1;
			this->Registers.eflags.ID = 1;
			this->Registers.eflags.AC = 0;
			/* We need to leave the libc's crt
			   to make a syscall when the Thread
			   is exited or we are going to get
			   an exception. */

			this->SetupUserStack_x86_32(argv, envp, auxv);
#elif defined(aa64)
			this->SetupUserStack_aarch64(argv, envp, auxv);
#endif
#ifdef DEBUG_TASKING
			DumpData(this->Name, this->Stack, STACK_SIZE);
#endif
			break;
		}
		default:
			assert(false);
		}

		SetupThreadLocalStorage();

		this->Info.Architecture = Architecture;
		this->Info.Compatibility = Compatibility;
		this->Security.ExecutionMode =
			this->Parent->Security.ExecutionMode;

		if (this->Parent->Threads.size() == 0)
		{
			this->Parent->Info.Architecture = this->Info.Architecture;
			this->Parent->Info.Compatibility = this->Info.Compatibility;
		}

		// TODO: Is really a good idea to use the FPU in kernel mode?
		this->FPU.mxcsr = 0b0001111110000000;
		this->FPU.mxcsrmask = 0b1111111110111111;
		this->FPU.fcw = 0b0000001100111111;

#ifdef DEBUG
#ifdef a64
		debug("Thread EntryPoint: %#lx => RIP: %#lx",
			  this->EntryPoint, this->Registers.rip);
		if (this->Parent->Security.ExecutionMode == TaskExecutionMode::User)
			debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx",
				  this->Stack->GetStackBottom(), this->Stack->GetStackTop(),
				  this->Registers.rsp);
		else
			debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx",
				  this->Stack->GetStackBottom(), this->Stack->GetStackTop(),
				  this->Registers.rsp);
#elif defined(a32)
		debug("Thread EntryPoint: %#lx => RIP: %#lx",
			  this->EntryPoint, this->Registers.eip);
		if (Parent->Security.ExecutionMode == TaskExecutionMode::User)
			debug("Thread stack region is %#lx-%#lx (U) and rsp is %#lx",
				  this->Stack->GetStackBottom(), this->Stack->GetStackTop(),
				  this->Registers.esp);
		else
			debug("Thread stack region is %#lx-%#lx (K) and rsp is %#lx",
				  this->Stack->GetStackBottom(), this->Stack->GetStackTop(),
				  this->Registers.esp);
#elif defined(aa64)
#endif
		debug("Created %s thread \"%s\"(%d) in process \"%s\"(%d)",
			  this->Security.ExecutionMode == TaskExecutionMode::User ? "user" : "kernel",
			  this->Name, this->ID, this->Parent->Name,
			  this->Parent->ID);
#endif

		this->AllocatedMemory += sizeof(TCB);
		this->AllocatedMemory += strlen(this->Parent->Name) + 1;
		this->AllocatedMemory += sizeof(Memory::StackGuard);

		this->Info.SpawnTime = TimeManager->GetCounter();
		this->Parent->Threads.push_back(this);

		if (this->Parent->Threads.size() == 1 &&
			this->Parent->State == Waiting &&
			ThreadNotReady == false)
		{
			this->Parent->SetState(Ready);
			debug("Setting process \"%s\"(%d) to ready",
				  this->Parent->Name, this->Parent->ID);
		}
	}

	TCB::~TCB()
	{
		debug("- %#lx", this);

		/* Remove us from the process list so we
			don't get scheduled anymore */
		std::list<Tasking::TCB *> &Threads = this->Parent->Threads;
		Threads.erase(std::find(Threads.begin(),
								Threads.end(),
								this));

		/* Free CPU Stack */
		delete this->Stack;

		/* Free Name */
		delete[] this->Name;

		debug("Thread \"%s\"(%d) destroyed",
			  this->Name, this->ID);
	}
}
