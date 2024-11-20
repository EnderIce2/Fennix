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

struct SyscallData
{
	const char *Name;
	void *Handler;
	int RequiredID;
};

using namespace Memory;

#if defined(a64)
typedef long arch_t;
#elif defined(a32)
typedef int arch_t;
#endif

void sys_0() { stub; }
void sys_1() { stub; }

#define sc_MaxSyscall 2

static SyscallData NativeSyscallsTable[sc_MaxSyscall] = {
	[0] = {
		"0 syscall",
		(void *)sys_0,
		UINT16_MAX,
	},
	[1] = {
		"1 syscall",
		(void *)sys_1,
		UINT16_MAX,
	}};

uintptr_t HandleNativeSyscalls(SysFrm *Frame)
{
#if defined(a64)
	if (unlikely(Frame->rax > sc_MaxSyscall))
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

	debug("> [%d:\"%s\"]( %#lx  %#lx  %#lx  %#lx  %#lx  %#lx )",
		  Frame->rax, Syscall.Name,
		  Frame->rdi, Frame->rsi, Frame->rdx,
		  Frame->r10, Frame->r8, Frame->r9);

	long sc_ret = call(Frame,
					   Frame->rdi, Frame->rsi, Frame->rdx,
					   Frame->r10, Frame->r8, Frame->r9);

	debug("< [%d:\"%s\"] = %d",
		  Frame->rax, Syscall.Name, sc_ret);
	return sc_ret;
#elif defined(a32)
	if (unlikely(Frame->eax > sc_MaxSyscall))
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

	debug("> [%d:\"%s\"]( %#x  %#x  %#x  %#x  %#x  %#x )",
		  Frame->eax, Syscall.Name,
		  Frame->ebx, Frame->ecx, Frame->edx,
		  Frame->esi, Frame->edi, Frame->ebp);

	int sc_ret = call(Frame,
					  Frame->ebx, Frame->ecx, Frame->edx,
					  Frame->esi, Frame->edi, Frame->ebp);

	debug("< [%d:\"%s\"] = %d",
		  Frame->eax, Syscall.Name, sc_ret);
	return sc_ret;
#elif defined(aa64)
	return -ENOSYS;
#endif
}
