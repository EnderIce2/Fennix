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

#include <syscalls.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <exec.hpp>
#include <errno.h>
#include <debug.h>

#include "../syscalls.h"
#include "../kernel.h"
#include "../ipc.h"

#if defined(a64) || defined(aa64)
static ssize_t ConvertErrno(ssize_t r)
{
	if (r >= 0)
		return r;
	return -errno;
}
#endif

static int ConvertErrno(int r)
{
	if (r >= 0)
		return r;
	return -errno;
}

struct SyscallData
{
	const char *Name;
	void *Handler;
	int RequiredID;
};

using InterProcessCommunication::IPC;
using InterProcessCommunication::IPCID;
using Tasking::PCB;
using Tasking::TCB;
using Tasking::TaskStatus::Ready;
using Tasking::TaskStatus::Terminated;
using namespace Memory;

#define SysFrm SyscallsFrame

#if defined(a64)
typedef long arch_t;
#elif defined(a32)
typedef int arch_t;
#endif

__noreturn static void sys_exit(SysFrm *, int Code)
{
	trace("Userspace thread %s(%d) exited with code %d (%#x)",
		  thisThread->Name,
		  thisThread->ID, Code,
		  Code < 0 ? -Code : Code);

	thisThread->ExitCode = Code;
	thisThread->Status = Terminated;
	TaskManager->Yield();
	__builtin_unreachable();
}

static uintptr_t sys_request_pages(SysFrm *, size_t Count)
{
	MemMgr *MemMgr = thisThread->Memory;
	return (uintptr_t)MemMgr->RequestPages(Count + 1, true);
}

static int sys_free_pages(SysFrm *, uintptr_t Address,
						  size_t Count)
{
	MemMgr *MemMgr = thisThread->Memory;
	MemMgr->FreePages((void *)Address, Count + 1);
	return 0;
}

static int sys_detach_address(SysFrm *, uintptr_t Address)
{
	MemMgr *MemMgr = thisThread->Memory;
	MemMgr->DetachAddress((void *)Address);
	return 0;
}

static int sys_memory_map(SysFrm *, uintptr_t VirtualAddress,
						  uintptr_t PhysicalAddress, size_t Size,
						  int Flags)
{
	if (Flags > 7) /* (MAP_PRESENT | MAP_WRITABLE | MAP_USER) */
		return -EINVAL;

	PageTable *PageTable = thisProcess->PageTable;
	{
		Virtual vmm = Virtual(PageTable);
		vmm.Map((void *)VirtualAddress,
				(void *)PhysicalAddress,
				Size, Flags);
	}

	return 0;
}

static int sys_memory_unmap(SysFrm *, uintptr_t VirtualAddress,
							size_t Size)
{
	PageTable *PageTable = thisProcess->PageTable;
	{
		Virtual vmm = Virtual(PageTable);
		vmm.Unmap((void *)VirtualAddress,
				  Size);
	}

	return 0;
}

static arch_t sys_kernelctl(SysFrm *, KCtl Command,
							arch_t Arg1, arch_t Arg2,
							arch_t Arg3, arch_t Arg4)
{
	UNUSED(Arg2);
	UNUSED(Arg3);
	UNUSED(Arg4);

	switch (Command)
	{
	case KCTL_PRINT:
	{
		SmartHeap sh(strlen((const char *)Arg1) + 1);
		sh = Arg1;
		KPrint(sh);
		return 0;
	}
	case KCTL_GET_PAGE_SIZE:
		return PAGE_SIZE;
	case KCTL_IS_CRITICAL:
		return thisThread->Security.IsCritical;
	default:
	{
		warn("KernelCTL: Unknown command: %d", Command);
		return -EINVAL;
	}
	}
}

static int sys_file_open(SysFrm *, const char *Path,
						 int Flags, mode_t Mode)
{
	function("%s, %d, %d", Path, Flags, Mode);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_open(Path, Flags, Mode));
}

static int sys_file_close(SysFrm *, int FileDescriptor)
{
	function("%d", FileDescriptor);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_close(FileDescriptor));
}

static uint64_t sys_file_read(SysFrm *, int FileDescriptor,
							  void *Buffer, size_t Count)
{
	function("%d, %p, %d", FileDescriptor, Buffer, Count);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_read(FileDescriptor, Buffer, Count));
}

static uint64_t sys_file_write(SysFrm *, int FileDescriptor,
							   const void *Buffer, size_t Count)
{
	function("%d, %p, %d", FileDescriptor, Buffer, Count);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_write(FileDescriptor, Buffer, Count));
}

static off_t sys_file_seek(SysFrm *, int FileDescriptor,
						   off_t Offset, int Whence)
{
	function("%d, %d, %d", FileDescriptor, Offset, Whence);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_lseek(FileDescriptor, Offset, Whence));
}

static int sys_file_status(SysFrm *, int FileDescriptor,
						   struct stat *StatBuffer)
{
	function("%d", FileDescriptor);
	PCB *pcb = thisProcess;
	VirtualFileSystem::FileDescriptorTable *fdt = pcb->FileDescriptors;
	return ConvertErrno(fdt->_fstat(FileDescriptor, StatBuffer));
}

static int sys_ipc(SysFrm *, enum IPCCommand Command,
				   enum IPCType Type, int ID, int Flags,
				   void *Buffer, size_t Size)
{
	InterProcessCommunication::IPC *ipc = thisProcess->IPC;
	return ipc->HandleSyscall(Command, Type, ID, Flags, Buffer, Size);
}

static long sys_local_thread_state(SysFrm *, int Code,
								   unsigned long Address)
{
	/* TODO: return EFAULT if Address is not mapped */
	/* TODO: return EINVAL if Code is invalid */
	/* TODO: return EPERM if Address is outside of process address space */
#if defined(a64) || defined(aa64)
	switch (Code)
	{
	case LTS_SET_GS:
	{
		wrmsr(CPU::x64::MSR_GS_BASE, Address);
		return 0;
	}
	case LTS_GET_GS:
	{
		return rdmsr(CPU::x64::MSR_GS_BASE);
	}
	case LTS_SET_FS:
	{
		wrmsr(CPU::x64::MSR_FS_BASE, Address);
		return 0;
	}
	case LTS_GET_FS:
	{
		return rdmsr(CPU::x64::MSR_FS_BASE);
	}
	case LTS_SET_CPUID:
	{
		fixme("TLS_SET_CPUID");
		return -ENOSYS;
	}
	case LTS_GET_CPUID:
	{
		fixme("TLS_GET_CPUID");
		return -ENOSYS;
	}
	default:
		return -EINVAL;
	}
#endif
	return -ENOSYS;
}

static int sys_sleep(SysFrm *, uint64_t Milliseconds)
{
	TaskManager->Sleep(Milliseconds, true);
	return 0;
}

static int sys_fork(SysFrm *Frame)
{
#ifdef a32
	return -ENOSYS;
#endif
	PCB *Parent = thisThread->Parent;
	TCB *Thread = thisThread;

	void *ProcSymTable = nullptr;
	if (Parent->ELFSymbolTable)
		ProcSymTable = Parent->ELFSymbolTable->GetImage();

	PCB *NewProcess =
		TaskManager->CreateProcess(Parent,
								   Parent->Name,
								   Parent->Security.ExecutionMode,
								   ProcSymTable);

	if (!NewProcess)
	{
		error("Failed to create process for fork");
		return -EAGAIN;
	}

	NewProcess->IPC->Fork(Parent->IPC);

	TCB *NewThread =
		TaskManager->CreateThread(NewProcess,
								  0,
								  nullptr,
								  nullptr,
								  std::vector<AuxiliaryVector>(),
								  Thread->Info.Architecture,
								  Thread->Info.Compatibility,
								  true);

	NewThread->Rename(Thread->Name);

	if (!NewThread)
	{
		error("Failed to create thread for fork");
		return -EAGAIN;
	}

	static int RetChild = 0;
	static uint64_t ReturnAddress = 0;
	static uint64_t ChildStackPointer = 0;

	TaskManager->UpdateFrame();

	if (RetChild--)
	{
		/* We can't just return 0; because the
			CPUData->SystemCallStack is no
			longer valid */
#if defined(a64) || defined(aa64)
		asmv("movq %0, %%rcx\n"
			 :
			 : "r"(ReturnAddress));
		asmv("mov %0, %%rsp\n"
			 :
			 : "r"(ChildStackPointer));
		asmv("mov %0, %%rbp\n"
			 :
			 : "r"(ChildStackPointer));
		asmv("movq $0, %rax\n"); /* Return 0 to the child */
		asmv("swapgs\n");		 /* Swap GS back to the user GS */
		asmv("sti\n");			 /* Enable interrupts */
		asmv("sysretq\n");		 /* Return to rcx address in user mode */
#elif defined(a32)
		UNUSED(ReturnAddress);
		UNUSED(ChildStackPointer);
#endif
	}
	RetChild = 1;
	ReturnAddress = Frame->ReturnAddress;
	ChildStackPointer = Frame->StackPointer;

	memcpy(&NewThread->FPU, &Thread->FPU, sizeof(CPU::x64::FXState));
	NewThread->Stack->Fork(Thread->Stack);
	NewThread->Info = Thread->Info;
	NewThread->Registers = Thread->Registers;

	if (Thread->Security.IsCritical)
		NewThread->SetCritical(true);

#ifdef a86
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->GSBase = Thread->GSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)",
		  Thread->Name, Thread->ID,
		  NewThread->Name, NewThread->ID);
	NewThread->Status = Ready;
	return (int)NewThread->ID;
}

static int sys_wait(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_kill(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_spawn(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_spawn_thread(SysFrm *, uint64_t InstructionPointer)
{
	TCB *thread =
		TaskManager->CreateThread(thisProcess,
								  Tasking::IP(InstructionPointer));
	if (thread)
		return (int)thread->ID;
	return -EAGAIN;
}

static int sys_get_thread_list_of_process(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_get_current_process(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_get_current_thread(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_get_current_process_id(SysFrm *)
{
	return (int)thisProcess->ID;
}

static int sys_get_current_thread_id(SysFrm *)
{
	return (int)thisThread->ID;
}

static int sys_get_process_by_pid(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_get_thread_by_tid(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_kill_process(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_kill_thread(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_sys_reserved_create_process(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static int sys_sys_reserved_create_thread(SysFrm *)
{
	stub;
	return -ENOSYS;
}

static SyscallData NativeSyscallsTable[sys_MaxSyscall] = {
	/**
	 *
	 * Basic syscalls
	 *
	 */

	[sys_Exit] = {
		"Exit",
		(void *)sys_exit,
		UINT16_MAX,
	},

	/**
	 *
	 * Memory syscalls
	 *
	 */

	[sys_RequestPages] = {
		"RequestPages",
		(void *)sys_request_pages,
		UINT16_MAX,
	},
	[sys_FreePages] = {
		"FreePages",
		(void *)sys_free_pages,
		UINT16_MAX,
	},
	[sys_DetachAddress] = {
		"DetachAddress",
		(void *)sys_detach_address,
		99,
	},
	[sys_MemoryMap] = {
		"MemoryMap",
		(void *)sys_memory_map,
		99,
	},
	[sys_MemoryUnmap] = {
		"MemoryUnmap",
		(void *)sys_memory_unmap,
		99,
	},

	/**
	 *
	 * Kernel Control syscalls
	 *
	 */

	[sys_KernelCTL] = {
		"KernelCTL",
		(void *)sys_kernelctl,
		99,
	},

	/**
	 *
	 * File syscalls
	 *
	 */

	[sys_FileOpen] = {
		"FileOpen",
		(void *)sys_file_open,
		UINT16_MAX,
	},
	[sys_FileClose] = {
		"FileClose",
		(void *)sys_file_close,
		UINT16_MAX,
	},
	[sys_FileRead] = {
		"FileRead",
		(void *)sys_file_read,
		UINT16_MAX,
	},
	[sys_FileWrite] = {
		"FileWrite",
		(void *)sys_file_write,
		UINT16_MAX,
	},
	[sys_FileSeek] = {
		"FileSeek",
		(void *)sys_file_seek,
		UINT16_MAX,
	},
	[sys_FileStatus] = {
		"FileStatus",
		(void *)sys_file_status,
		UINT16_MAX,
	},

	/**
	 *
	 * Process syscalls
	 *
	 */

	[sys_IPC] = {
		"IPC",
		(void *)sys_ipc,
		UINT16_MAX,
	},
	[sys_LocalThreadState] = {
		"LocalThreadState",
		(void *)sys_local_thread_state,
		UINT16_MAX,
	},
	[sys_Sleep] = {
		"Sleep",
		(void *)sys_sleep,
		UINT16_MAX,
	},
	[sys_Fork] = {
		"Fork",
		(void *)sys_fork,
		UINT16_MAX,
	},
	[sys_Wait] = {
		"Wait",
		(void *)sys_wait,
		0,
	},
	[sys_Kill] = {
		"Kill",
		(void *)sys_kill,
		0,
	},
	[sys_Spawn] = {
		"Spawn",
		(void *)sys_spawn,
		0,
	},
	[sys_SpawnThread] = {
		"SpawnThread",
		(void *)sys_spawn_thread,
		0,
	},
	[sys_GetThreadListOfProcess] = {
		"GetThreadListOfProcess",
		(void *)sys_get_thread_list_of_process,
		0,
	},
	[sys_GetCurrentProcess] = {
		"GetCurrentProcess",
		(void *)sys_get_current_process,
		0,
	},
	[sys_GetCurrentThread] = {
		"GetCurrentThread",
		(void *)sys_get_current_thread,
		0,
	},
	[sys_GetCurrentProcessID] = {
		"GetCurrentProcessID",
		(void *)sys_get_current_process_id,
		UINT16_MAX,
	},
	[sys_GetCurrentThreadID] = {
		"GetCurrentThreadID",
		(void *)sys_get_current_thread_id,
		UINT16_MAX,
	},
	[sys_GetProcessByPID] = {
		"GetProcessByPID",
		(void *)sys_get_process_by_pid,
		0,
	},
	[sys_GetThreadByTID] = {
		"GetThreadByTID",
		(void *)sys_get_thread_by_tid,
		0,
	},
	[sys_KillProcess] = {
		"KillProcess",
		(void *)sys_kill_process,
		0,
	},
	[sys_KillThread] = {
		"KillThread",
		(void *)sys_kill_thread,
		0,
	},
	[sys_SysReservedCreateProcess] = {
		"SysReservedCreateProcess",
		(void *)sys_sys_reserved_create_process,
		0,
	},
	[sys_SysReservedCreateThread] = {
		"SysReservedCreateThread",
		(void *)sys_sys_reserved_create_thread,
		0,
	},
};

uintptr_t HandleNativeSyscalls(SysFrm *Frame)
{
#if defined(a64)
	if (unlikely(Frame->rax > sys_MaxSyscall))
	{
		fixme("Syscall %ld not implemented.", Frame->rax);
		return -ENOSYS;
	}

	SyscallData Syscall = NativeSyscallsTable[Frame->rax];

	uintptr_t (*call)(SysFrm *, uintptr_t, ...) =
		r_cst(uintptr_t(*)(SysFrm *, uintptr_t, ...),
			  Syscall.Handler);

	if (unlikely(!call))
	{
		error("Syscall %s(%d) not implemented.",
			  Syscall.Name, Frame->rax);
		return -ENOSYS;
	}

	int euid = thisProcess->Security.Effective.UserID;
	int egid = thisProcess->Security.Effective.GroupID;
	int reqID = Syscall.RequiredID;
	if (euid > reqID || egid > reqID)
	{
		warn("Process %s(%d) tried to access a system call \"%s\" with insufficient privileges.",
			 thisProcess->Name, thisProcess->ID, Syscall.Name);
		debug("Required: %d; Effective u:%d, g:%d", reqID, euid, egid);
		return -EPERM;
	}

	debug("[%d:\"%s\"]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax, Syscall.Name,
		  Frame->rdi, Frame->rsi, Frame->rdx,
		  Frame->r10, Frame->r8, Frame->r9);

	return call(Frame,
				Frame->rdi, Frame->rsi, Frame->rdx,
				Frame->r10, Frame->r8, Frame->r9);
#elif defined(a32)
	if (unlikely(Frame->eax > sys_MaxSyscall))
	{
		fixme("Syscall %ld not implemented.", Frame->eax);
		return -ENOSYS;
	}

	SyscallData Syscall = NativeSyscallsTable[Frame->eax];

	uintptr_t (*call)(SysFrm *, uintptr_t, ...) =
		r_cst(uintptr_t(*)(SysFrm *, uintptr_t, ...),
			  Syscall.Handler);

	if (unlikely(!call))
	{
		error("Syscall %s(%d) not implemented.",
			  Syscall.Name, Frame->eax);
		return -ENOSYS;
	}

	int euid = thisProcess->Security.Effective.UserID;
	int egid = thisProcess->Security.Effective.GroupID;
	int reqID = Syscall.RequiredID;
	if (euid > reqID || egid > reqID)
	{
		warn("Process %s(%d) tried to access a system call \"%s\" with insufficient privileges.",
			 thisProcess->Name, thisProcess->ID, Syscall.Name);
		debug("Required: %d; Effective u:%d, g:%d", reqID, euid, egid);
		return -EPERM;
	}

	debug("[%d:\"%s\"]->( %#x  %#x  %#x  %#x  %#x  %#x )",
		  Frame->eax, Syscall.Name,
		  Frame->ebx, Frame->ecx, Frame->edx,
		  Frame->esi, Frame->edi, Frame->ebp);

	return call(Frame,
				Frame->ebx, Frame->ecx, Frame->edx,
				Frame->esi, Frame->edi, Frame->ebp);
#elif defined(aa64)
	return -ENOSYS;
#endif
}
