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

#include <dumper.hpp>
#include <convert.h>
#include <lock.hpp>
#include <printf.h>
#include <smp.hpp>
#include <io.h>

#include "../kernel.h"

#if defined(a64)
#include "../Architecture/amd64/cpu/apic.hpp"
#include "../Architecture/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../Architecture/i386/cpu/apic.hpp"
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

void ThreadDoExit()
{
	CPUData *CPUData = GetCurrentCPU();
	Tasking::TCB *CurrentThread = CPUData->CurrentThread.load();
	CurrentThread->Status = Tasking::TaskStatus::Terminated;

	debug("\"%s\"(%d) exited with code: %#lx",
		  CurrentThread->Name,
		  CurrentThread->ID,
		  CurrentThread->ExitCode);
	CPU::Halt(true);
}

namespace Tasking
{
	void TCB::Rename(const char *name)
	{
		assert(name != nullptr);
		assert(strlen(name) > 0);

		trace("Renaming thread %s to %s", this->Name, name);
		if (this->Name)
			delete[] this->Name;
		this->Name = new char[strlen(name) + 1];
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
	}

	void TCB::SetDebugMode(bool Enable)
	{
		trace("Setting debug mode of thread %s to %s",
			  this->Name, Enable ? "true" : "false");

		Security.IsDebugEnabled = Enable;
	}

	void TCB::SetKernelDebugMode(bool Enable)
	{
		trace("Setting kernel debug mode of thread %s to %s",
			  this->Name, Enable ? "true" : "false");

		Security.IsKernelDebugEnabled = Enable;
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
#else
#warning "SYSV ABI not implemented for this architecture"
#endif
	}

	__no_sanitize("undefined")
		TCB::TCB(Task *ctx, PCB *Parent, IP EntryPoint,
				 const char **argv, const char **envp,
				 const std::vector<AuxiliaryVector> &auxv,
				 TaskArchitecture Architecture,
				 TaskCompatibility Compatibility,
				 bool ThreadNotReady)
	{
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
		this->ID = ctx->NextTID++;

		if (this->Name)
			delete[] this->Name;
		this->Name = new char[strlen(this->Parent->Name) + 1];
		strcpy((char *)this->Name, this->Parent->Name);

		this->EntryPoint = EntryPoint;
		this->ExitCode = KILL_CRASH;
		this->Info.Architecture = Architecture;
		this->Info.Compatibility = Compatibility;
		this->Security.ExecutionMode =
			this->Parent->Security.ExecutionMode;
		if (ThreadNotReady)
			this->Status = TaskStatus::Zombie;
		else
			this->Status = TaskStatus::Ready;
		this->Memory = new Memory::MemMgr(this->Parent->PageTable,
										  this->Parent->memDirectory);
		std::size_t FXPgs = TO_PAGES(sizeof(CPU::x64::FXState) + 1);
		this->FPU = (CPU::x64::FXState *)this->Memory->RequestPages(FXPgs);
		memset(this->FPU, 0, sizeof(CPU::x64::FXState));

		// TODO: Is really a good idea to use the FPU in kernel mode?
		this->FPU->mxcsr = 0b0001111110000000;
		this->FPU->mxcsrmask = 0b1111111110111111;
		this->FPU->fcw = 0b0000001100111111;

		// CPU::x64::fxrstor(this->FPU);
		// uint16_t FCW = 0b1100111111;
		// asmv("fldcw %0"
		//      :
		//      : "m"(FCW)
		//      : "memory");
		// uint32_t MXCSR = 0b1111110000000;
		// asmv("ldmxcsr %0"
		//      :
		//      : "m"(MXCSR)
		//      : "memory");
		// CPU::x64::fxsave(this->FPU);

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
			this->Stack = new Memory::StackGuard(false,
												 this->Parent->PageTable);

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
			this->Registers.rsp = ((uintptr_t)this->Stack->GetStackTop());
			POKE(uintptr_t, this->Registers.rsp) = (uintptr_t)ThreadDoExit;
#elif defined(a32)
#elif defined(aa64)
#endif
			break;
		}
		case TaskExecutionMode::User:
		{
			this->Stack = new Memory::StackGuard(true,
												 this->Parent->PageTable);

			gsTCB *gsT = (gsTCB *)this->Memory->RequestPages(TO_PAGES(sizeof(gsTCB)));

			gsT->SyscallStack =
				(uintptr_t)this->Memory->RequestPages(TO_PAGES(STACK_SIZE)) +
				STACK_SIZE - 0x10;

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
			/* We need to leave the libc's crt
			   to make a syscall when the Thread
			   is exited or we are going to get
			   GPF or PF exception. */

#pragma region
			size_t ArgvSize = 0;
			if (argv)
				while (argv[ArgvSize] != nullptr)
					ArgvSize++;

			size_t EnvpSize = 0;
			if (envp)
				while (envp[EnvpSize] != nullptr)
					EnvpSize++;

			debug("ArgvSize: %d", ArgvSize);
			debug("EnvpSize: %d", EnvpSize);

			/* https://articles.manugarg.com/aboutelfauxiliaryvectors.html */
			/* https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf#figure.3.9 */
			// rsp is the top of the stack
			char *Stack = (char *)this->Stack->GetStackPhysicalTop();
			// Temporary stack pointer for strings
			char *StackStrings = (char *)Stack;
			char *StackStringsVirtual = (char *)this->Stack->GetStackTop();

			// Store string pointers for later
			uintptr_t *ArgvStrings = nullptr;
			uintptr_t *EnvpStrings = nullptr;
			if (ArgvSize > 0)
				ArgvStrings = new uintptr_t[ArgvSize];
			if (EnvpSize > 0)
				EnvpStrings = new uintptr_t[EnvpSize];

			for (size_t i = 0; i < ArgvSize; i++)
			{
				// Subtract the length of the string and the null terminator
				StackStrings -= strlen(argv[i]) + 1;
				StackStringsVirtual -= strlen(argv[i]) + 1;
				// Store the pointer to the string
				ArgvStrings[i] = (uintptr_t)StackStringsVirtual;
				// Copy the string to the stack
				strcpy(StackStrings, argv[i]);
				debug("argv[%d]: %s", i, argv[i]);
			}

			for (size_t i = 0; i < EnvpSize; i++)
			{
				// Subtract the length of the string and the null terminator
				StackStrings -= strlen(envp[i]) + 1;
				StackStringsVirtual -= strlen(envp[i]) + 1;
				// Store the pointer to the string
				EnvpStrings[i] = (uintptr_t)StackStringsVirtual;
				// Copy the string to the stack
				strcpy(StackStrings, envp[i]);
				debug("envp[%d]: %s", i, envp[i]);
			}

			// Align the stack to 16 bytes
			StackStrings -= (uintptr_t)StackStrings & 0xF;
			// Set "Stack" to the new stack pointer
			Stack = (char *)StackStrings;
			// If argv and envp sizes are odd then we need to align the stack
			Stack -= (ArgvSize + EnvpSize) % 2;

			// We need 8 bit pointers for the stack from here
			uintptr_t *Stack64 = (uintptr_t *)Stack;

			// Store the null terminator
			Stack64--;
			*Stack64 = AT_NULL;

			// auxv_array is initialized with auxv elements. If the array is empty then we add a null terminator
			std::vector<AuxiliaryVector> auxv_array = auxv;
			if (auxv_array.size() == 0)
				auxv_array.push_back({.archaux = {.a_type = AT_NULL, .a_un = {.a_val = 0}}});

			// Store auxillary vector
			foreach (AuxiliaryVector var in auxv_array)
			{
				// Subtract the size of the auxillary vector
				Stack64 -= sizeof(Elf64_auxv_t) / sizeof(uintptr_t);
				// Store the auxillary vector
				POKE(Elf64_auxv_t, Stack64) = var.archaux;
				// TODO: Store strings to the stack
			}

			// Store the null terminator
			Stack64--;
			*Stack64 = AT_NULL;

			// Store EnvpStrings[] to the stack
			Stack64 -= EnvpSize; // (1 Stack64 = 8 bits; Stack64 = 8 * EnvpSize)
			for (size_t i = 0; i < EnvpSize; i++)
			{
				*(Stack64 + i) = (uintptr_t)EnvpStrings[i];
				debug("EnvpStrings[%d]: %#lx",
					  i, EnvpStrings[i]);
			}

			// Store the null terminator
			Stack64--;
			*Stack64 = AT_NULL;

			// Store ArgvStrings[] to the stack
			Stack64 -= ArgvSize; // (1 Stack64 = 8 bits; Stack64 = 8 * ArgvSize)
			for (size_t i = 0; i < ArgvSize; i++)
			{
				*(Stack64 + i) = (uintptr_t)ArgvStrings[i];
				debug("ArgvStrings[%d]: %#lx",
					  i, ArgvStrings[i]);
			}

			// Store the argc
			Stack64--;
			*Stack64 = ArgvSize;

			// Set "Stack" to the new stack pointer
			Stack = (char *)Stack64;

			/* We need the virtual address but because we are in the kernel we can't use the process page table.
				So we modify the physical address and store how much we need to subtract to get the virtual address for RSP. */
			uintptr_t SubtractStack = (uintptr_t)this->Stack->GetStackPhysicalTop() - (uintptr_t)Stack;
			debug("SubtractStack: %#lx", SubtractStack);

			// Set the stack pointer to the new stack
			this->Registers.rsp = ((uintptr_t)this->Stack->GetStackTop() - SubtractStack);

			if (ArgvSize > 0)
				delete[] ArgvStrings;
			if (EnvpSize > 0)
				delete[] EnvpStrings;

#ifdef DEBUG
			DumpData("Stack Data", (void *)((uintptr_t)this->Stack->GetStackPhysicalTop() - (uintptr_t)SubtractStack), SubtractStack);
#endif

			this->Registers.rdi = (uintptr_t)ArgvSize;										 // argc
			this->Registers.rsi = (uintptr_t)(this->Registers.rsp + 8);						 // argv
			this->Registers.rcx = (uintptr_t)EnvpSize;										 // envc
			this->Registers.rdx = (uintptr_t)(this->Registers.rsp + 8 + (8 * ArgvSize) + 8); // envp

#pragma endregion

#elif defined(a32)
#elif defined(aa64)
#endif
#ifdef DEBUG_TASKING
			DumpData(this->Name, this->Stack, STACK_SIZE);
#endif
			break;
		}
		default:
			assert(false);
		}

		this->Info.SpawnTime = TimeManager->GetCounter();

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
		debug("Created thread \"%s\"(%d) in process \"%s\"(%d)",
			  this->Name, this->ID, this->Parent->Name,
			  this->Parent->ID);
#endif

		this->Parent->Threads.push_back(this);

		if (this->Parent->Threads.size() == 1 &&
			this->Parent->Status == Zombie &&
			ThreadNotReady == false)
		{
			this->Parent->Status = Ready;
		}
	}

	TCB::~TCB()
	{
		std::vector<Tasking::TCB *> &Threads = this->Parent->Threads;
		Threads.erase(std::find(Threads.begin(),
								Threads.end(),
								this));

		delete[] this->Name;
		delete this->Stack;
		delete this->Memory;
	}
}
