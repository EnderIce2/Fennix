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

#include <interface/syscalls.h>

#include <syscalls.hpp>
#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../kernel.h"

using Tasking::PCB;
using Tasking::TCB;

__no_stack_protector void __ForkReturn(void *tableAddr)
{
#if defined(__amd64__)
	asmv("movq %0, %%cr3" ::"r"(tableAddr)); /* Load process page table */
	asmv("movq $0, %rax\n");				 /* Return 0 */
	asmv("movq %r8, %rsp\n");				 /* Restore stack pointer */
	asmv("movq %r8, %rbp\n");				 /* Restore base pointer */
	asmv("swapgs\n");						 /* Swap GS back to the user GS */
	asmv("sti\n");							 /* Enable interrupts */
	asmv("sysretq\n");						 /* Return to rcx address in user mode */
#elif defined(__i386__)
#warning "__ForkReturn not implemented for i386"
#endif
	__builtin_unreachable();
}

__noreturn void sys_exit(SysFrm *Frame, int status)
{
	TCB *t = thisThread;
	{
		CriticalSection cs;
		trace("Userspace thread %s(%d) exited with code %d (%#x)",
			  t->Name,
			  t->ID, status,
			  status < 0 ? -status : status);

		t->SetState(Tasking::Zombie);
		t->SetExitCode(status);
	}
	while (true)
		t->GetContext()->Yield();
	__builtin_unreachable();
}

pid_t sys_fork(SysFrm *Frame)
{
	TCB *Thread = thisThread;
	PCB *Parent = Thread->Parent;

	PCB *NewProcess =
		TaskManager->CreateProcess(Parent, Parent->Name,
								   Parent->Security.ExecutionMode,
								   true);
	if (unlikely(!NewProcess))
	{
		error("Failed to create process for fork");
		return -EAGAIN;
	}

	NewProcess->Security.ProcessGroupID = Parent->Security.ProcessGroupID;
	NewProcess->Security.SessionID = Parent->Security.SessionID;

	NewProcess->PageTable = Parent->PageTable->Fork();
	NewProcess->vma->Table = NewProcess->PageTable;
	NewProcess->vma->Fork(Parent->vma);
	NewProcess->ProgramBreak->SetTable(NewProcess->PageTable);
	NewProcess->FileDescriptors->Fork(Parent->FileDescriptors);
	NewProcess->Executable = Parent->Executable;
	NewProcess->CWD = Parent->CWD;
	NewProcess->FileCreationMask = Parent->FileCreationMask;

	TCB *NewThread =
		TaskManager->CreateThread(NewProcess,
								  0,
								  nullptr,
								  nullptr,
								  std::vector<AuxiliaryVector>(),
								  Thread->Info.Architecture,
								  Thread->Info.Compatibility,
								  true);
	if (!NewThread)
	{
		error("Failed to create thread for fork");
		delete NewProcess;
		return -EAGAIN;
	}
	NewThread->Rename(Thread->Name);

	TaskManager->UpdateFrame();

#if defined(__amd64__) || defined(__i386__)
	NewThread->FPU = Thread->FPU;
#endif
	NewThread->Stack->Fork(Thread->Stack);
	NewThread->Info.Architecture = Thread->Info.Architecture;
	NewThread->Info.Compatibility = Thread->Info.Compatibility;
	NewThread->Security.IsCritical = Thread->Security.IsCritical;
	NewThread->Registers = Thread->Registers;
#if defined(__amd64__)
	NewThread->Registers.rip = (uintptr_t)__ForkReturn;
	/* For sysretq */
	NewThread->Registers.rdi = (uintptr_t)NewProcess->PageTable;
	NewThread->Registers.rcx = Frame->ReturnAddress;
	NewThread->Registers.r8 = Frame->StackPointer;
#else
#warning "sys_fork not implemented for other platforms"
#endif

#if defined(__amd64__) || defined(__i386__)
	NewThread->GSBase = NewThread->ShadowGSBase;
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("ret addr: %#lx, stack: %#lx ip: %#lx", Frame->ReturnAddress,
		  Frame->StackPointer, (uintptr_t)__ForkReturn);
	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)",
		  Thread->Name, Thread->ID,
		  NewThread->Name, NewThread->ID);
	NewThread->SetState(Tasking::Ready);

	// Parent->GetContext()->Yield();
	return (int)NewProcess->ID;
}

int sys_execve(SysFrm *Frame, const char *pathname, char *const argv[], char *const envp[])
{
	return -ENOSYS;
}

pid_t sys_getpid(SysFrm *Frame)
{
	return -ENOSYS;
}

pid_t sys_getppid(SysFrm *Frame)
{
	return -ENOSYS;
}

pid_t sys_waitpid(pid_t pid, int *wstatus, int options)
{
	return -ENOSYS;
}

int sys_kill(SysFrm *Frame, pid_t pid, int sig)
{
	PCB *pcb = thisProcess->GetContext()->GetProcessByID(pid);
	if (!pcb)
		return -ESRCH;

	/* TODO: Check permissions */

	if (sig == 0)
		return 0;

	if (pid == 0)
	{
		bool found = false;
		for (auto proc : pcb->GetContext()->GetProcessList())
		{
			if (proc->Security.ProcessGroupID == thisProcess->Security.ProcessGroupID)
			{
				debug("Sending signal %d to %s(%d)", sig, proc->Name, proc->ID);
				proc->SendSignal(sig);
				found = true;
			}
		}
		if (!found)
			return -ESRCH;
		return 0;
	}

	if (pid == -1)
	{
		fixme("Sending signal %d to all processes except init", sig);
		return -ENOSYS;
	}

	if (pid < -1)
	{
		fixme("Sending signal %d to process group %d", sig, pid);
		return -ENOSYS;
	}

	return pcb->SendSignal(sig);
}

int sys_prctl(SysFrm *Frame, prctl_options_t option, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4)
{
	PCB *pcb = thisProcess;
	Memory::VirtualMemoryArea *vma = pcb->vma;

	switch (option)
	{
	case __SYS_GET_GS:
	{
		auto arg = vma->UserCheckAndGetAddress((void *)arg1);
		if (arg == nullptr)
			return -EFAULT;

#if defined(__amd64__) || defined(__i386__)
		*r_cst(uintptr_t *, arg) = CPU::x86::rdmsr(CPU::x86::MSRID::MSR_GS_BASE);
#endif
		return 0;
	}
	case __SYS_SET_GS:
	{
#if defined(__amd64__) || defined(__i386__)
		CPU::x86::wrmsr(CPU::x86::MSRID::MSR_GS_BASE, arg1);
#endif
		return 0;
	}
	case __SYS_GET_FS:
	{
		auto arg = vma->UserCheckAndGetAddress((void *)arg1);
		if (arg == nullptr)
			return -EFAULT;

#if defined(__amd64__) || defined(__i386__)
		*r_cst(uintptr_t *, arg) = CPU::x86::rdmsr(CPU::x86::MSRID::MSR_FS_BASE);
#endif
		return 0;
	}
	case __SYS_SET_FS:
	{
#if defined(__amd64__) || defined(__i386__)
		CPU::x86::wrmsr(CPU::x86::MSRID::MSR_FS_BASE, arg1);
#endif
		return 0;
	}
	default:
		return -EINVAL;
	}
}
