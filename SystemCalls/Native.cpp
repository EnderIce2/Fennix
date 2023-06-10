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

using InterProcessCommunication::IPC;
using InterProcessCommunication::IPCID;
using Tasking::Token;
using Tasking::TTL;
using Tasking::TTL::Trusted;
using Tasking::TTL::TrustedByKernel;
using Tasking::TTL::UnknownTrustLevel;
using Tasking::TTL::Untrusted;

static inline bool CheckTrust(int TrustLevel)
{
	Token token = TaskManager->GetCurrentThread()->Security.UniqueToken;
	if (likely(TaskManager->GetSecurityManager()->IsTokenTrusted(token, TrustLevel)))
		return true;

	warn("Thread %s(%lld) tried to access a system call \"%s\" with insufficient trust level",
		 TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID,
		 KernelSymbolTable->GetSymbolFromAddress((uintptr_t)__builtin_extract_return_addr(__builtin_return_address(0))));
	debug("token=%#lx, trust=%d", token, TaskManager->GetSecurityManager()->GetTokenTrustLevel(token));
	return false;
}

static int sys_exit(SyscallsFrame *Frame, int code)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted | UnknownTrustLevel))
		return SYSCALL_ACCESS_DENIED;

	trace("Userspace thread %s(%lld) exited with code %#llx", TaskManager->GetCurrentThread()->Name, TaskManager->GetCurrentThread()->ID, code);
	TaskManager->GetCurrentThread()->ExitCode = code;
	TaskManager->GetCurrentThread()->Status = Tasking::TaskStatus::Terminated;
	TaskManager->Schedule();

	UNUSED(Frame);
	return SYSCALL_OK;
}

static int sys_print(SyscallsFrame *Frame, char Char, int Index)
{
	if (!CheckTrust(TrustedByKernel | Trusted))
		return SYSCALL_ACCESS_DENIED;

	char ret = Display->Print(Char, Index, true);
	if (!Config.BootAnimation && Index == 0)
#ifdef DEBUG
		Display->SetBuffer(Index);
#else
		if (Char == '\n')
			Display->SetBuffer(Index);
#endif
	UNUSED(Frame);
	return ret;
}

static uintptr_t sys_request_pages(SyscallsFrame *Frame, size_t Count)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	UNUSED(Frame);
	return (uintptr_t)TaskManager->GetCurrentThread()->Memory->RequestPages(Count + 1, true);
}

static int sys_free_pages(SyscallsFrame *Frame, uintptr_t Address, size_t Count)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	TaskManager->GetCurrentThread()->Memory->FreePages((void *)Address, Count + 1);
	UNUSED(Frame);
	return SYSCALL_OK;
}

static int sys_detach_address(SyscallsFrame *Frame, uintptr_t Address)
{
	if (!CheckTrust(TrustedByKernel | Trusted))
		return SYSCALL_ACCESS_DENIED;

	TaskManager->GetCurrentThread()->Memory->DetachAddress((void *)Address);
	UNUSED(Frame);
	return SYSCALL_OK;
}

static int sys_memory_map(SyscallsFrame *Frame, uintptr_t VirtualAddress, uintptr_t PhysicalAddress, size_t Size, enum MemoryMapFlags Flags)
{
	if (!CheckTrust(TrustedByKernel))
		return SYSCALL_ACCESS_DENIED;

	if (Flags > 7) /* (MAP_PRESENT | MAP_WRITABLE | MAP_USER) */
		return SYSCALL_INVALID_ARGUMENT;

	Memory::PageTable *PageTable = TaskManager->GetCurrentProcess()->PageTable;
	{
		Memory::Virtual vmm = Memory::Virtual(PageTable);
		vmm.Map((void *)VirtualAddress,
				(void *)PhysicalAddress,
				Size, Flags);
	}

	UNUSED(Frame);
	return SYSCALL_OK;
}

static int sys_memory_unmap(SyscallsFrame *Frame, uintptr_t VirtualAddress, size_t Size)
{
	if (!CheckTrust(TrustedByKernel))
		return SYSCALL_ACCESS_DENIED;

	Memory::PageTable *PageTable = TaskManager->GetCurrentProcess()->PageTable;
	{
		Memory::Virtual vmm = Memory::Virtual(PageTable);
		vmm.Unmap((void *)VirtualAddress,
				  Size);
	}

	UNUSED(Frame);
	return SYSCALL_OK;
}

static uintptr_t sys_kernelctl(SyscallsFrame *Frame, enum KCtl Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	switch (Command)
	{
	case KCTL_GET_PID:
		return TaskManager->GetCurrentThread()->Parent->ID;
	case KCTL_GET_TID:
		return TaskManager->GetCurrentThread()->ID;
	case KCTL_GET_PAGE_SIZE:
		return PAGE_SIZE;
	case KCTL_IS_CRITICAL:
		return TaskManager->GetCurrentThread()->Security.IsCritical;
	case KCTL_REGISTER_ELF_LIB:
	{
		if (!CheckTrust(TrustedByKernel | Trusted))
			return SYSCALL_ACCESS_DENIED;

		char *Identifier = (char *)Arg1;
		const char *Path = (const char *)Arg2;

		if (!Identifier || !Path)
			return SYSCALL_INVALID_ARGUMENT;

		std::string FullPath = Path;
		int retries = 0;
	RetryReadPath:
		debug("KCTL_REGISTER_ELF_LIB: Trying to open %s", FullPath.c_str());
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
				VirtualFileSystem::Node *cwd = TaskManager->GetCurrentProcess()->CurrentWorkingDirectory;
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
		if (!CheckTrust(TrustedByKernel | Trusted))
			return SYSCALL_ACCESS_DENIED;

		char *Identifier = (char *)Arg1;
		if (!Identifier)
			return 0;

		Execute::SharedLibrary lib = Execute::GetLibrary(Identifier);

		if (!lib.MemoryImage)
		{
			debug("Failed to get library memory image %#lx", (uintptr_t)lib.MemoryImage);
		}

		debug("Returning memory image %#lx (%s)", (uintptr_t)lib.MemoryImage, Identifier);
		return (uintptr_t)lib.MemoryImage;
	}
	case KCTL_GET_ABSOLUTE_PATH:
	{
		if (!CheckTrust(TrustedByKernel | Trusted))
			return SYSCALL_ACCESS_DENIED;

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
	UNUSED(Frame);
}

static uint64_t sys_file_open(SyscallsFrame *Frame, const char *Path, uint64_t Flags)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	debug("(Path: %s, Flags: %#lx)", Path, Flags);
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

	VirtualFileSystem::File *KernelPrivate = (VirtualFileSystem::File *)TaskManager->GetCurrentThread()->Memory->RequestPages(TO_PAGES(sizeof(VirtualFileSystem::File)));
	*KernelPrivate = KPObj;
	debug("Opened file %s (%d)", KPObj.Name, KPObj.Status);
	return (uint64_t)KernelPrivate;
	UNUSED(Frame);
	UNUSED(Flags);
}

static int sys_file_close(SyscallsFrame *Frame, void *KernelPrivate)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	debug("(KernelPrivate: %#lx)", KernelPrivate);

	if (KernelPrivate)
	{
		VirtualFileSystem::File KPObj = *(VirtualFileSystem::File *)KernelPrivate;
		debug("Closed file %s (%d)", KPObj.Name, KPObj.Status);
		vfs->Close(KPObj);
		TaskManager->GetCurrentThread()->Memory->FreePages(KernelPrivate, TO_PAGES(sizeof(VirtualFileSystem::File)));
		return SYSCALL_OK;
	}
	return SYSCALL_INVALID_ARGUMENT;
	UNUSED(Frame);
}

static uint64_t sys_file_read(SyscallsFrame *Frame, void *KernelPrivate, uint8_t *Buffer, uint64_t Size)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Buffer: %#lx, Size: %#lx)", KernelPrivate, Buffer, Size);
	return vfs->Read(*(VirtualFileSystem::File *)KernelPrivate, Buffer, (size_t)Size);
	UNUSED(Frame);
}

static uint64_t sys_file_write(SyscallsFrame *Frame, void *KernelPrivate, uint8_t *Buffer, uint64_t Size)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Buffer: %#lx, Size: %#lx)", KernelPrivate, Buffer, Size);
	return vfs->Write(*(VirtualFileSystem::File *)KernelPrivate, Buffer, (size_t)Size);
	UNUSED(Frame);
}

static off_t sys_file_seek(SyscallsFrame *Frame, void *KernelPrivate, off_t Offset, int Whence)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	if (KernelPrivate == nullptr)
		return 0;

	debug("(KernelPrivate: %#lx, Offset: %#lx, Whence: %d)", KernelPrivate, Offset, Whence);
	VirtualFileSystem::File *KPObj = (VirtualFileSystem::File *)KernelPrivate;

	off_t ret = vfs->Seek(*KPObj, (off_t)Offset, (uint8_t)Whence);
	debug("Seek %s %ld", KPObj->Name, ret);
	return ret;
	UNUSED(Frame);
}

static int sys_file_status(SyscallsFrame *Frame)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	fixme("sys_file_status: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_ipc(SyscallsFrame *Frame, enum IPCCommand Command, enum IPCType Type, int ID, int Flags, void *Buffer, size_t Size)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	UNUSED(Frame);
	return TaskManager->GetCurrentProcess()->IPC->HandleSyscall(Command, Type, ID, Flags, Buffer, Size);
}

static int sys_sleep(SyscallsFrame *Frame, uint64_t Milliseconds)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	UNUSED(Frame);
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;
	TaskManager->Sleep(Milliseconds);
	return 0;
}

static int sys_fork(SyscallsFrame *Frame)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	Tasking::PCB *Parent = TaskManager->GetCurrentThread()->Parent;
	Tasking::TCB *Thread = TaskManager->GetCurrentThread();

	Tasking::PCB *NewProcess = TaskManager->CreateProcess(Parent,
														  Parent->Name,
														  Parent->Security.TrustLevel,
														  Parent->ELFSymbolTable ? Parent->ELFSymbolTable->GetImage() : nullptr);

	if (!NewProcess)
	{
		error("Failed to create process for fork");
		return SYSCALL_ERROR;
	}

	strncpy(NewProcess->Name, Parent->Name, sizeof(NewProcess->Name));
	NewProcess->IPC->Fork(Parent->IPC);

	Tasking::TCB *NewThread = TaskManager->CreateThread(NewProcess,
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
		/* We can't just return 0; because the CPUData->SystemCallStack is no longer valid */
		asmv("movq %0, %%rcx\n"
			 :
			 : "r"(ReturnAddress));
		asmv("mov %0, %%rsp\n"
			 :
			 : "r"(ChildStackPointer));
		asmv("mov %0, %%rbp\n"
			 :
			 : "r"(ChildStackPointer));
		asmv("movq $0, %rax\n"); // Return 0 to the child
		asmv("swapgs\n");		 // Swap GS back to the user GS
		asmv("sti\n");			 // Enable interrupts
		asmv("sysretq\n");		 // Return to rcx address in user mode
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
	TaskManager->GetSecurityManager()->TrustToken(NewProcess->Security.UniqueToken,
												  (Tasking::TTL)TaskManager->GetSecurityManager()->GetTokenTrustLevel(Parent->Security.UniqueToken));
	TaskManager->GetSecurityManager()->TrustToken(NewThread->Security.UniqueToken,
												  (Tasking::TTL)TaskManager->GetSecurityManager()->GetTokenTrustLevel(Thread->Security.UniqueToken));

#ifdef a86
	NewThread->ShadowGSBase = Thread->ShadowGSBase;
	NewThread->GSBase = Thread->GSBase;
	NewThread->FSBase = Thread->FSBase;
#endif

	debug("Forked thread \"%s\"(%d) to \"%s\"(%d)", Thread->Name, Thread->ID, NewThread->Name, NewThread->ID);
	NewThread->Status = Tasking::TaskStatus::Ready;
	return (int)NewThread->ID;
	UNUSED(Frame);
}

static int sys_wait(SyscallsFrame *Frame)
{
	fixme("sys_wait: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill(SyscallsFrame *Frame)
{
	fixme("sys_kill: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn(SyscallsFrame *Frame)
{
	fixme("sys_spawn: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_spawn_thread(SyscallsFrame *Frame, uint64_t InstructionPointer)
{
	Tasking::TCB *thread = TaskManager->CreateThread(TaskManager->GetCurrentProcess(), (Tasking::IP)InstructionPointer);
	if (thread)
		return (int)thread->ID;
	return SYSCALL_ERROR;
}

static int sys_get_thread_list_of_process(SyscallsFrame *Frame)
{
	fixme("sys_get_thread_list_of_process: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_process(SyscallsFrame *Frame)
{
	fixme("sys_get_current_process: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_thread(SyscallsFrame *Frame)
{
	fixme("sys_get_current_thread: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_current_process_id(SyscallsFrame *Frame)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	return (int)TaskManager->GetCurrentProcess()->ID;
}

static int sys_get_current_thread_id(SyscallsFrame *Frame)
{
	if (!CheckTrust(TrustedByKernel | Trusted | Untrusted))
		return SYSCALL_ACCESS_DENIED;

	return (int)TaskManager->GetCurrentThread()->ID;
}

static int sys_get_process_by_pid(SyscallsFrame *Frame)
{
	fixme("sys_get_process_by_pid: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_get_thread_by_tid(SyscallsFrame *Frame)
{
	fixme("sys_get_thread_by_tid: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_process(SyscallsFrame *Frame)
{
	fixme("sys_kill_process: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_kill_thread(SyscallsFrame *Frame)
{
	fixme("sys_kill_thread: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_process(SyscallsFrame *Frame)
{
	fixme("sys_sys_reserved_create_process: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static int sys_sys_reserved_create_thread(SyscallsFrame *Frame)
{
	fixme("sys_sys_reserved_create_thread: %#lx", Frame);
	return SYSCALL_NOT_IMPLEMENTED;
}

static void *NativeSyscallsTable[_MaxSyscall] = {
	[_Exit] = (void *)sys_exit,
	[_Print] = (void *)sys_print,

	[_RequestPages] = (void *)sys_request_pages,
	[_FreePages] = (void *)sys_free_pages,
	[_DetachAddress] = (void *)sys_detach_address,
	[_MemoryMap] = (void *)sys_memory_map,
	[_MemoryUnmap] = (void *)sys_memory_unmap,

	[_KernelCTL] = (void *)sys_kernelctl,

	[_FileOpen] = (void *)sys_file_open,
	[_FileClose] = (void *)sys_file_close,
	[_FileRead] = (void *)sys_file_read,
	[_FileWrite] = (void *)sys_file_write,
	[_FileSeek] = (void *)sys_file_seek,
	[_FileStatus] = (void *)sys_file_status,

	[_IPC] = (void *)sys_ipc,
	[_Sleep] = (void *)sys_sleep,
	[_Fork] = (void *)sys_fork,
	[_Wait] = (void *)sys_wait,
	[_Kill] = (void *)sys_kill,
	[_Spawn] = (void *)sys_spawn,
	[_SpawnThread] = (void *)sys_spawn_thread,
	[_GetThreadListOfProcess] = (void *)sys_get_thread_list_of_process,
	[_GetCurrentProcess] = (void *)sys_get_current_process,
	[_GetCurrentThread] = (void *)sys_get_current_thread,
	[_GetCurrentProcessID] = (void *)sys_get_current_process_id,
	[_GetCurrentThreadID] = (void *)sys_get_current_thread_id,
	[_GetProcessByPID] = (void *)sys_get_process_by_pid,
	[_GetThreadByTID] = (void *)sys_get_thread_by_tid,
	[_KillProcess] = (void *)sys_kill_process,
	[_KillThread] = (void *)sys_kill_thread,
	[_SysReservedCreateProcess] = (void *)sys_sys_reserved_create_process,
	[_SysReservedCreateThread] = (void *)sys_sys_reserved_create_thread,
};

uintptr_t HandleNativeSyscalls(SyscallsFrame *Frame)
{
#if defined(a64)
	if (Frame->rax > sizeof(NativeSyscallsTable))
	{
		fixme("Syscall %ld not implemented", Frame->rax);
		return SYSCALL_NOT_IMPLEMENTED;
	}

	uintptr_t (*call)(uintptr_t, ...) = reinterpret_cast<uintptr_t (*)(uintptr_t, ...)>(NativeSyscallsTable[Frame->rax]);
	if (!call)
	{
		error("Syscall %#lx failed.", Frame->rax);
		return SYSCALL_INTERNAL_ERROR;
	}

	debug("[%#lx]->( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax,
		  Frame->rdi, Frame->rsi, Frame->rdx, Frame->rcx, Frame->r8, Frame->r9);

	uintptr_t ret = call((uintptr_t)Frame, Frame->rdi, Frame->rsi, Frame->rdx, Frame->r10, Frame->r8, Frame->r9);
	Frame->rax = ret;
	return ret;
#elif defined(a32)
	if (Frame->eax > sizeof(NativeSyscallsTable))
	{
		fixme("Syscall %ld not implemented", Frame->eax);
		return SYSCALL_NOT_IMPLEMENTED;
	}

	/* ... */

	return SYSCALL_INTERNAL_ERROR;
#elif defined(aa64)
	return SYSCALL_INTERNAL_ERROR;
#endif
}
