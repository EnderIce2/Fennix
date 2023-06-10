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

#define SysFrm SyscallsFrame

using InterProcessCommunication::IPC;
using InterProcessCommunication::IPCID;
using Tasking::Token;
using Tasking::TTL;
using Tasking::TaskStatus::Ready;
using Tasking::TaskStatus::Terminated;
using Tasking::TTL::Trusted;
using Tasking::TTL::TrustedByKernel;
using Tasking::TTL::UnknownTrustLevel;
using Tasking::TTL::Untrusted;

__noreturn static void sys_exit(SysFrm *, int code)
{
	trace("Userspace thread %s(%d) exited with code %d (%#x)",
		  TaskManager->GetCurrentThread()->Name,
		  TaskManager->GetCurrentThread()->ID, code,
		  code < 0 ? -code : code);

	TaskManager->GetCurrentThread()->ExitCode = code;
	TaskManager->GetCurrentThread()->Status = Terminated;
	TaskManager->Schedule();
	__builtin_unreachable();
}

static int sys_print(SysFrm *, char Char, int Index)
{
	char ret = Display->Print(Char, Index, true);
	if (!Config.BootAnimation && Index == 0)
		Display->SetBuffer(Index);
	return ret;
}

static uintptr_t sys_request_pages(SysFrm *, size_t Count)
{
	Memory::MemMgr *MemMgr = TaskManager->GetCurrentThread()->Memory;
	return (uintptr_t)MemMgr->RequestPages(Count + 1, true);
}

static int sys_free_pages(SysFrm *, uintptr_t Address, size_t Count)
{
	Memory::MemMgr *MemMgr = TaskManager->GetCurrentThread()->Memory;
	MemMgr->FreePages((void *)Address, Count + 1);
	return SYSCALL_OK;
}

static int sys_detach_address(SysFrm *, uintptr_t Address)
{
	Memory::MemMgr *MemMgr = TaskManager->GetCurrentThread()->Memory;
	MemMgr->DetachAddress((void *)Address);
	return SYSCALL_OK;
}

static int sys_memory_map(SysFrm *, uintptr_t VirtualAddress,
						  uintptr_t PhysicalAddress, size_t Size,
						  enum MemoryMapFlags Flags)
{
	if (Flags > 7) /* (MAP_PRESENT | MAP_WRITABLE | MAP_USER) */
		return SYSCALL_INVALID_ARGUMENT;

	Memory::PageTable *PageTable = TaskManager->GetCurrentProcess()->PageTable;
	{
		Memory::Virtual vmm = Memory::Virtual(PageTable);
		vmm.Map((void *)VirtualAddress,
				(void *)PhysicalAddress,
				Size, Flags);
	}

	return SYSCALL_OK;
}

static int sys_memory_unmap(SysFrm *, uintptr_t VirtualAddress,
							size_t Size)
{
	Memory::PageTable *PageTable = TaskManager->GetCurrentProcess()->PageTable;
	{
		Memory::Virtual vmm = Memory::Virtual(PageTable);
		vmm.Unmap((void *)VirtualAddress,
				  Size);
	}

	return SYSCALL_OK;
}

static uintptr_t sys_kernelctl(SysFrm *,
							   enum KCtl Command,
							   uint64_t Arg1, uint64_t Arg2,
							   uint64_t Arg3, uint64_t Arg4)
{
	switch (Command)
	{
	case KCTL_GET_PID:
		return TaskManager->GetCurrentProcess()->ID;
	case KCTL_GET_TID:
		return TaskManager->GetCurrentThread()->ID;
	case KCTL_GET_PAGE_SIZE:
		return PAGE_SIZE;
	case KCTL_IS_CRITICAL:
		return TaskManager->GetCurrentThread()->Security.IsCritical;
	case KCTL_REGISTER_ELF_LIB:
	{
		char *Identifier = (char *)Arg1;
		const char *Path = (const char *)Arg2;

		if (!Identifier || !Path)
			return SYSCALL_INVALID_ARGUMENT;

		std::string FullPath = Path;
		int retries = 0;
	RetryReadPath:
		debug("KCTL_REGISTER_ELF_LIB: Trying to open %s",
			  FullPath.c_str());
		VirtualFileSystem::File f = vfs->Open(FullPath.c_str());

		if (!f.IsOK())
		{
			FullPath.clear();
			switch (retries)
			{
			case 0:
				FullPath = "/lib/";
				break;
			case 1:
				FullPath = "/usr/lib/";
				break;
			case 2:
				FullPath = "/";
				break;
			case 3:
			{
				VirtualFileSystem::Node *cwd =
					TaskManager->GetCurrentProcess()->CurrentWorkingDirectory;
				FullPath = vfs->GetPathFromNode(cwd).get();
				break;
			}
			default:
			{
				vfs->Close(f);
				return SYSCALL_INVALID_ARGUMENT;
			}
			}
			FullPath += Path;
			vfs->Close(f);
			retries++;
			goto RetryReadPath;
		}

		if (Execute::AddLibrary(Identifier, f))
		{
			vfs->Close(f);
			return SYSCALL_OK;
		}
		else
		{
			vfs->Close(f);
			return SYSCALL_INTERNAL_ERROR;
		}
	}
	case KCTL_GET_ELF_LIB_MEMORY_IMAGE:
	{
		char *Identifier = (char *)Arg1;
		if (!Identifier)
			return 0;

		Execute::SharedLibrary lib = Execute::GetLibrary(Identifier);

		if (!lib.MemoryImage)
		{
			debug("Failed to get library memory image %#lx",
				  (uintptr_t)lib.MemoryImage);
		}

		debug("Returning memory image %#lx (%s)",
			  (uintptr_t)lib.MemoryImage, Identifier);
		return (uintptr_t)lib.MemoryImage;
	}
	case KCTL_GET_ABSOLUTE_PATH:
	{
		char *Identifier = (char *)Arg1;
		void *Buffer = (void *)Arg2;
		size_t BufferSize = Arg3;

		if (!Identifier || !Buffer || !BufferSize)
			return SYSCALL_INVALID_ARGUMENT;

		Execute::SharedLibrary lib = Execute::GetLibrary(Identifier);

		if (!lib.MemoryImage)
			return SYSCALL_INTERNAL_ERROR;

		if (BufferSize < sizeof(lib.Path))
			return SYSCALL_INVALID_ARGUMENT;

		memcpy(Buffer, lib.Path, sizeof(lib.Path));
		return SYSCALL_OK;
	}
	default:
	{
		warn("KernelCTL: Unknown command: %lld", Command);
		return SYSCALL_INVALID_ARGUMENT;
	}
	}

	UNUSED(Arg1);
	UNUSED(Arg2);
	UNUSED(Arg3);
	UNUSED(Arg4);
}

static uint64_t sys_file_open(SysFrm *, const char *Path, uint64_t Flags)
{
	function("%s, %#lx", Path, Flags);
	VirtualFileSystem::Node *cwd = nullptr;
	if (vfs->PathIsRelative(Path))
		cwd = TaskManager->GetCurrentProcess()->CurrentWorkingDirectory;
	else
		cwd = vfs->GetRootNode();

	VirtualFileSystem::File KPObj = vfs->Open(Path, cwd);
	if (!KPObj.IsOK())
	{
		debug("Failed to open file %s (%d)", Path, KPObj.Status);
		vfs->Close(KPObj);
		return SYSCALL_INTERNAL_ERROR;
	}

	Memory::MemMgr *MemMgr = TaskManager->GetCurrentThread()->Memory;

	constexpr size_t FileStructPages =
		TO_PAGES(sizeof(VirtualFileSystem::File));

	VirtualFileSystem::File *KernelPrivate =
		(VirtualFileSystem::File *)MemMgr->RequestPages(FileStructPages);
	*KernelPrivate = KPObj;
	debug("Opened file %s (%d)", KPObj.Name, KPObj.Status);
	return (uint64_t)KernelPrivate;
	UNUSED(Flags);
}

static int sys_file_close(SysFrm *, void *KernelPrivate)
{
	function("%#lx", KernelPrivate);

	if (KernelPrivate)
	{
		VirtualFileSystem::File KPObj = *(VirtualFileSystem::File *)KernelPrivate;
		debug("Closed file %s (%d)", KPObj.Name, KPObj.Status);
		vfs->Close(KPObj);
		Memory::MemMgr *MemMgr = TaskManager->GetCurrentThread()->Memory;
		MemMgr->FreePages(KernelPrivate,
						  TO_PAGES(sizeof(VirtualFileSystem::File)));
		return SYSCALL_OK;
	}
	return SYSCALL_INVALID_ARGUMENT;
}

static uint64_t sys_file_read(SysFrm *, void *KernelPrivate,
							  uint8_t *Buffer, uint64_t Size)
{
	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Buffer: %#lx, Size: %#lx)",
		  KernelPrivate, Buffer, Size);

	VirtualFileSystem::File *KPObj = (VirtualFileSystem::File *)KernelPrivate;
	return vfs->Read(*KPObj, Buffer, (size_t)Size);
}

static uint64_t sys_file_write(SysFrm *, void *KernelPrivate,
							   uint8_t *Buffer, uint64_t Size)
{
	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Buffer: %#lx, Size: %#lx)",
		  KernelPrivate, Buffer, Size);

	VirtualFileSystem::File *KPObj = (VirtualFileSystem::File *)KernelPrivate;
	return vfs->Write(*KPObj, Buffer, (size_t)Size);
}

static off_t sys_file_seek(SysFrm *, void *KernelPrivate,
						   off_t Offset, int Whence)
{
	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Whence: %d)",
		  KernelPrivate, Offset, Whence);

	VirtualFileSystem::File *KPObj = (VirtualFileSystem::File *)KernelPrivate;
	off_t ret = vfs->Seek(*KPObj, (off_t)Offset, (uint8_t)Whence);
	debug("Seek %s %ld", KPObj->Name, ret);
	return ret;
}

static int sys_file_status(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_ipc(SysFrm *, enum IPCCommand Command,
				   enum IPCType Type, int ID, int Flags,
				   void *Buffer, size_t Size)
{
	InterProcessCommunication::IPC *ipc = TaskManager->GetCurrentProcess()->IPC;
	return ipc->HandleSyscall(Command, Type, ID, Flags, Buffer, Size);
}

static int sys_sleep(SysFrm *, uint64_t Milliseconds)
{
	TaskManager->Sleep(Milliseconds, true);
	return 0;
}

static int sys_fork(SysFrm *Frame)
{
	Tasking::PCB *Parent = TaskManager->GetCurrentThread()->Parent;
	Tasking::TCB *Thread = TaskManager->GetCurrentThread();

	void *ProcSymTable = nullptr;
	if (Parent->ELFSymbolTable)
		ProcSymTable = Parent->ELFSymbolTable->GetImage();

	Tasking::PCB *NewProcess =
		TaskManager->CreateProcess(Parent,
								   Parent->Name,
								   Parent->Security.TrustLevel,
								   ProcSymTable);

	if (!NewProcess)
	{
		error("Failed to create process for fork");
		return SYSCALL_ERROR;
	}

	strncpy(NewProcess->Name, Parent->Name, sizeof(NewProcess->Name));
	NewProcess->IPC->Fork(Parent->IPC);

	Tasking::TCB *NewThread =
		TaskManager->CreateThread(NewProcess,
								  0,
								  nullptr,
								  nullptr,
								  std::vector<AuxiliaryVector>(),
								  Thread->Info.Architecture,
								  Thread->Info.Compatibility,
								  true);

	strncpy(NewThread->Name, Thread->Name, sizeof(Thread->Name));

	if (!NewThread)
	{
		error("Failed to create thread for fork");
		return SYSCALL_ERROR;
	}

	static int RetChild = 0;
	static uint64_t ReturnAddress = 0;
	static uint64_t ChildStackPointer = 0;

	TaskManager->Schedule();

	if (RetChild--)
	{
		/* We can't just return 0; because the
			CPUData->SystemCallStack is no
			longer valid */
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
	}
	RetChild = 1;
	ReturnAddress = Frame->ReturnAddress;
	ChildStackPointer = Frame->StackPointer;

	memcpy(NewThread->FPU, Thread->FPU, sizeof(CPU::x64::FXState));
	NewThread->Stack->Fork(Thread->Stack);
	NewThread->Info = Thread->Info;
	NewThread->Registers = Thread->Registers;

	if (Thread->Security.IsCritical)
		NewThread->SetCritical(true);

	Tasking::Security *Sec = TaskManager->GetSecurityManager();
	Sec->TrustToken(NewProcess->Security.UniqueToken,
					(TTL)Sec->GetTokenTrustLevel(Parent->Security.UniqueToken));
	Sec->TrustToken(NewThread->Security.UniqueToken,
					(TTL)Sec->GetTokenTrustLevel(Thread->Security.UniqueToken));

#ifdef a86
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->GSBase = Thread->GSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)", Thread->Name, Thread->ID, NewThread->Name, NewThread->ID);
	NewThread->Status = Ready;
	return (int)NewThread->ID;
}

static int sys_wait(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn_thread(SysFrm *, uint64_t InstructionPointer)
{
	Tasking::TCB *thread =
		TaskManager->CreateThread(TaskManager->GetCurrentProcess(),
								  (Tasking::IP)InstructionPointer);
	if (thread)
		return (int)thread->ID;
	return SYSCALL_ERROR;
}

static int sys_get_thread_list_of_process(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_process(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_thread(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_process_id(SysFrm *)
{
	return (int)TaskManager->GetCurrentProcess()->ID;
}

static int sys_get_current_thread_id(SysFrm *)
{
	return (int)TaskManager->GetCurrentThread()->ID;
}

static int sys_get_process_by_pid(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_thread_by_tid(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_process(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_thread(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_process(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_thread(SysFrm *)
{
	stub;
	return SYSCALL_NOT_IMPLEMENTED;
}

struct SyscallData
{
	const char *Name;
	void *Handler;
	int TrustLevel;
};

static SyscallData NativeSyscallsTable[_MaxSyscall] = {
	/**
	 *
	 * Basic syscalls
	 *
	 */

	[_Exit] = {
		"Exit",
		(void *)sys_exit,
		TrustedByKernel | Trusted | Untrusted | UnknownTrustLevel,
	},
	[_Print] = {
		"Print",
		(void *)sys_print,
		TrustedByKernel | Trusted,
	},

	/**
	 *
	 * Memory syscalls
	 *
	 */

	[_RequestPages] = {
		"RequestPages",
		(void *)sys_request_pages,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FreePages] = {
		"FreePages",
		(void *)sys_free_pages,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_DetachAddress] = {
		"DetachAddress",
		(void *)sys_detach_address,
		TrustedByKernel | Trusted,
	},
	[_MemoryMap] = {
		"MemoryMap",
		(void *)sys_memory_map,
		TrustedByKernel,
	},
	[_MemoryUnmap] = {
		"MemoryUnmap",
		(void *)sys_memory_unmap,
		TrustedByKernel,
	},

	/**
	 *
	 * Kernel Control syscalls
	 *
	 */

	[_KernelCTL] = {
		"KernelCTL",
		(void *)sys_kernelctl,
		TrustedByKernel | Trusted | Untrusted,
	},

	/**
	 *
	 * File syscalls
	 *
	 */

	[_FileOpen] = {
		"FileOpen",
		(void *)sys_file_open,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FileClose] = {
		"FileClose",
		(void *)sys_file_close,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FileRead] = {
		"FileRead",
		(void *)sys_file_read,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FileWrite] = {
		"FileWrite",
		(void *)sys_file_write,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FileSeek] = {
		"FileSeek",
		(void *)sys_file_seek,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_FileStatus] = {
		"FileStatus",
		(void *)sys_file_status,
		TrustedByKernel | Trusted | Untrusted,
	},

	/**
	 *
	 * Process syscalls
	 *
	 */

	[_IPC] = {
		"IPC",
		(void *)sys_ipc,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_Sleep] = {
		"Sleep",
		(void *)sys_sleep,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_Fork] = {
		"Fork",
		(void *)sys_fork,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_Wait] = {
		"Wait",
		(void *)sys_wait,
		0,
	},
	[_Kill] = {
		"Kill",
		(void *)sys_kill,
		0,
	},
	[_Spawn] = {
		"Spawn",
		(void *)sys_spawn,
		0,
	},
	[_SpawnThread] = {
		"SpawnThread",
		(void *)sys_spawn_thread,
		0,
	},
	[_GetThreadListOfProcess] = {
		"GetThreadListOfProcess",
		(void *)sys_get_thread_list_of_process,
		0,
	},
	[_GetCurrentProcess] = {
		"GetCurrentProcess",
		(void *)sys_get_current_process,
		0,
	},
	[_GetCurrentThread] = {
		"GetCurrentThread",
		(void *)sys_get_current_thread,
		0,
	},
	[_GetCurrentProcessID] = {
		"GetCurrentProcessID",
		(void *)sys_get_current_process_id,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_GetCurrentThreadID] = {
		"GetCurrentThreadID",
		(void *)sys_get_current_thread_id,
		TrustedByKernel | Trusted | Untrusted,
	},
	[_GetProcessByPID] = {
		"GetProcessByPID",
		(void *)sys_get_process_by_pid,
		0,
	},
	[_GetThreadByTID] = {
		"GetThreadByTID",
		(void *)sys_get_thread_by_tid,
		0,
	},
	[_KillProcess] = {
		"KillProcess",
		(void *)sys_kill_process,
		0,
	},
	[_KillThread] = {
		"KillThread",
		(void *)sys_kill_thread,
		0,
	},
	[_SysReservedCreateProcess] = {
		"SysReservedCreateProcess",
		(void *)sys_sys_reserved_create_process,
		0,
	},
	[_SysReservedCreateThread] = {
		"SysReservedCreateThread",
		(void *)sys_sys_reserved_create_thread,
		0,
	},
};

uintptr_t HandleNativeSyscalls(SysFrm *Frame)
{
#if defined(a64)
	if (unlikely(Frame->rax > _MaxSyscall))
	{
		fixme("Syscall %ld not implemented.", Frame->rax);
		return SYSCALL_NOT_IMPLEMENTED;
	}

	SyscallData Syscall = NativeSyscallsTable[Frame->rax];

	uintptr_t (*call)(SysFrm *, uintptr_t, ...) =
		r_cst(uintptr_t(*)(SysFrm *, uintptr_t, ...),
			  Syscall.Handler);

	if (unlikely(!call))
	{
		error("Syscall %#lx not implemented.", Frame->rax);
		return SYSCALL_NOT_IMPLEMENTED;
	}

	Token token = TaskManager->GetCurrentThread()->Security.UniqueToken;
	Tasking::Security *Sec = TaskManager->GetSecurityManager();
	if (unlikely(!Sec->IsTokenTrusted(token, Syscall.TrustLevel)))
	{
		warn("Thread %s(%d) tried to access a system call \"%s\" with insufficient trust level",
			 TaskManager->GetCurrentThread()->Name,
			 TaskManager->GetCurrentThread()->ID,
			 Syscall.Name);

#ifdef DEBUG
		int TknTl = Sec->GetTokenTrustLevel(token);
		debug("token=%#lx, trust=%d%d%d%d",token,
		TknTl & TrustedByKernel ? 1 : 0,
		TknTl & Trusted ? 1 : 0,
		TknTl & Untrusted ? 1 : 0,
		TknTl & UnknownTrustLevel ? 1 : 0);
#endif
		return SYSCALL_ACCESS_DENIED;
	}

	debug("[%d:\"%s\"]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax, Syscall.Name,
		  Frame->rdi, Frame->rsi, Frame->rdx,
		  Frame->r10, Frame->r8, Frame->r9);

	return call(Frame,
				Frame->rdi, Frame->rsi, Frame->rdx,
				Frame->r10, Frame->r8, Frame->r9);
#elif defined(a32)
	return SYSCALL_NOT_IMPLEMENTED;
#elif defined(aa64)
	return SYSCALL_NOT_IMPLEMENTED;
#endif
}
