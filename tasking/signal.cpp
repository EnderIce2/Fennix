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

#include <signal.hpp>
#include <dumper.hpp>
#include <task.hpp>
#include <errno.h>

#if defined(a64)
#include "../arch/amd64/cpu/gdt.hpp"
#elif defined(a32)
#include "../arch/i386/cpu/gdt.hpp"
#elif defined(aa64)
#endif

#include "../kernel.h"

#ifdef DEBUG
const char *lSigStr[] = {
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGPOLL",
	"SIGPWR",
	"SIGSYS",
	"SIGUNUSED",
};

const char *sigStr[] = {
	"INVALID  ",
	"SIGABRT  ",
	"SIGALRM  ",
	"SIGBUS   ",
	"SIGCHLD  ",
	"SIGCONT  ",
	"SIGFPE   ",
	"SIGHUP   ",
	"SIGILL   ",
	"SIGINT   ",
	"SIGKILL  ",
	"SIGPIPE  ",
	"SIGQUIT  ",
	"SIGSEGV  ",
	"SIGSTOP  ",
	"SIGTERM  ",
	"SIGTSTP  ",
	"SIGTTIN  ",
	"SIGTTOU  ",
	"SIGUSR1  ",
	"SIGUSR2  ",
	"SIGPOLL  ",
	"SIGPROF  ",
	"SIGSYS   ",
	"SIGTRAP  ",
	"SIGURG   ",
	"SIGVTALRM",
	"SIGXCPU  ",
	"SIGXFSZ  ",
	"SIGRSV1  ",
	"SIGRSV2  ",
};

const char *dispStr[] = {
	"SIG_TERM",
	"SIG_IGN ",
	"SIG_CORE",
	"SIG_STOP",
	"SIG_CONT",
};
#endif

extern "C" uintptr_t _sig_native_trampoline_start, _sig_native_trampoline_end;
extern "C" uintptr_t _sig_linux_trampoline_start, _sig_linux_trampoline_end;

static const struct
{
	int linuxSignal;
	int nativeSignal;
} signalMapping[] = {
	{linux_SIGHUP, SIGHUP},
	{linux_SIGINT, SIGINT},
	{linux_SIGQUIT, SIGQUIT},
	{linux_SIGILL, SIGILL},
	{linux_SIGTRAP, SIGTRAP},
	{linux_SIGABRT, SIGABRT},
	{linux_SIGBUS, SIGBUS},
	{linux_SIGFPE, SIGFPE},
	{linux_SIGKILL, SIGKILL},
	{linux_SIGUSR1, SIGUSR1},
	{linux_SIGSEGV, SIGSEGV},
	{linux_SIGUSR2, SIGUSR2},
	{linux_SIGPIPE, SIGPIPE},
	{linux_SIGALRM, SIGALRM},
	{linux_SIGTERM, SIGTERM},
	{linux_SIGSTKFLT, SIGRSV1},
	{linux_SIGCHLD, SIGCHLD},
	{linux_SIGCONT, SIGCONT},
	{linux_SIGSTOP, SIGSTOP},
	{linux_SIGTSTP, SIGTSTP},
	{linux_SIGTTIN, SIGTTIN},
	{linux_SIGTTOU, SIGTTOU},
	{linux_SIGURG, SIGURG},
	{linux_SIGXCPU, SIGXCPU},
	{linux_SIGXFSZ, SIGXFSZ},
	{linux_SIGVTALRM, SIGVTALRM},
	{linux_SIGPROF, SIGPROF},
	{linux_SIGPOLL, SIGPOLL},
	{linux_SIGPWR, SIGRSV2},
	{linux_SIGSYS, SIGSYS},
	{linux_SIGUNUSED, SIGSYS},
};

static_assert(linux_SIGUNUSED == SIGNAL_MAX);

#define CTLif(x) ConvertToLinuxIfNecessary(x)
#define CTNif(x) ConvertToNativeIfNecessary(x)
#define CSigTLif(x) ConvertSigsetToLinuxIfNecessary(x)
#define CSigTNif(x) ConvertSigsetToNativeIfNecessary(x)

/* TODO: CTLif & CTNif may need optimization */

namespace Tasking
{
	bool Signal::LinuxSig()
	{
		return ((PCB *)ctx)->Info.Compatibility == Linux;
	}

	int Signal::ConvertToLinuxIfNecessary(int sig)
	{
		if (!LinuxSig())
		{
			debug("Not linux sig: %d", sig);
			return sig;
		}

		foreach (auto &mapping in signalMapping)
		{
			if (mapping.nativeSignal == sig)
			{
				// debug("Converted %d to %d", sig, mapping.linuxSignal);
				return mapping.linuxSignal;
			}
		}

		return -1;
	}

	int Signal::ConvertToNativeIfNecessary(int sig)
	{
		if (!LinuxSig())
		{
			debug("Not native sig: %d", sig);
			return sig;
		}

		foreach (auto &mapping in signalMapping)
		{
			if (mapping.linuxSignal == sig)
			{
				// debug("Converted %d to %d", sig, mapping.nativeSignal);
				return mapping.nativeSignal;
			}
		}

		return -1;
	}

	sigset_t Signal::ConvertSigsetToLinuxIfNecessary(sigset_t sig)
	{
		if (!LinuxSig())
		{
			debug("Not linux sigset: %#lx", sig);
			return 0;
		}

		sigset_t ret = 0;
		for (int i = 0; i < SIGNAL_MAX; i++)
		{
			if (sig & ToFlag(i))
				ret |= ToFlag(CTLif(i));
		}

		return ret;
	}

	sigset_t Signal::ConvertSigsetToNativeIfNecessary(sigset_t sig)
	{
		if (!LinuxSig())
		{
			debug("Not native sigset: %#lx", sig);
			return 0;
		}

		sigset_t ret = 0;
		for (int i = 0; i < linux_SIGUNUSED; i++)
		{
			if (sig & ToFlag(i))
				ret |= ToFlag(CTNif(i));
		}

		return ret;
	}

	int Signal::MakeExitCode(int sig)
	{
		if (this->LinuxSig())
			return 128 + sig;
		else
			return 100 + sig;
	}

	void Signal::InitTrampoline()
	{
		if (unlikely(TrampAddr))
			return;

		PCB *pcb = (PCB *)ctx;

		debug("Trampoline not set up yet");
		switch (pcb->Info.Compatibility)
		{
		case Native:
		{
			debug("%#lx - %#lx",
				  &_sig_native_trampoline_end,
				  &_sig_native_trampoline_start);

			TrampSz = (size_t)&_sig_native_trampoline_end -
					  (size_t)&_sig_native_trampoline_start;
			TrampAddr = pcb->vma->RequestPages(TO_PAGES(TrampSz), true);
			memcpy((void *)TrampAddr,
				   (void *)&_sig_native_trampoline_start,
				   TrampSz);
			debug("Trampoline at %#lx with size %lld",
				  TrampAddr, TrampSz);
			break;
		}
		case Linux:
		{
			debug("%#lx - %#lx",
				  &_sig_linux_trampoline_end,
				  &_sig_linux_trampoline_start);

			TrampSz = (size_t)&_sig_linux_trampoline_end -
					  (size_t)&_sig_linux_trampoline_start;
			TrampAddr = pcb->vma->RequestPages(TO_PAGES(TrampSz), true);
			memcpy((void *)TrampAddr,
				   (void *)&_sig_linux_trampoline_start,
				   TrampSz);
			debug("Trampoline at %#lx with size %lld",
				  TrampAddr, TrampSz);
			break;
		}
		case Windows:
		{
			fixme("Windows compatibility");
			break;
		}
		default:
			/* Process not fully initalized. */
			return;
		}
	}

	/* ------------------------------------------------------ */

	int Signal::AddWatcher(Signal *who, int sig)
	{
		SmartLock(SignalLock);
		SignalInfo info;
		info.sig = sig;
		info.val.sival_ptr = who;

		Watchers.push_back(info);
		return 0;
	}

	int Signal::RemoveWatcher(Signal *who, int sig)
	{
		SmartLock(SignalLock);
		forItr(itr, Watchers)
		{
			if (itr->sig == sig &&
				itr->val.sival_ptr == who)
			{
				Watchers.erase(itr);
				return 0;
			}
		}
		return -ENOENT;
	}

	int Signal::AddSignal(int sig, union sigval val)
	{
		SmartLock(SignalLock);
		SignalInfo info{.sig = sig, .val = val};
		SignalQueue.push_back(info);
		return 0;
	}

	int Signal::RemoveSignal(int sig)
	{
		SmartLock(SignalLock);
		forItr(itr, SignalQueue)
		{
			if (itr->sig == sig)
			{
				SignalQueue.erase(itr);
				return 0;
			}
		}
		return -ENOENT;
	}

	bool Signal::HandleSignal(CPU::TrapFrame *tf)
	{
		SmartLock(SignalLock);
		if (SignalQueue.empty())
			return false;

		/* We don't want to do this in kernel mode */
		if (unlikely(tf->cs != GDT_USER_CODE))
			return false;

		debug("We have %d signals to handle", SignalQueue.size());

		SignalInfo sigI = SignalQueue.front();
		SignalQueue.erase(SignalQueue.begin());

		uintptr_t _p_rsp = ((PCB *)ctx)->PageTable->Get(tf->rsp);
		uint64_t paRsp = _p_rsp;
		paRsp &= ~0xF; /* Align */
		paRsp -= 128;  /* Red zone */

		/* Calculate the virtual rsp */
		uintptr_t _v_rsp = tf->rsp;
		_v_rsp &= ~0xF; /* Align */
		_v_rsp -= 128;	/* Red zone */
		uint64_t *vRsp = (uint64_t *)(_v_rsp - sizeof(StackInfo));
		vRsp--; /* Alignment */
		vRsp--; /* Handler Address */
		assert(!((uintptr_t)vRsp & 0xF));

		/* Add the stack info */
		StackInfo si{};
		CPU::x64::fxsave(&si.fx);
		si.tf = *tf;
		si.GSBase = CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
		si.FSBase = CPU::x64::rdmsr(CPU::x64::MSR_FS_BASE);
		si.ShadowGSBase = CPU::x64::rdmsr(CPU::x64::MSR_SHADOW_GS_BASE);
		debug("gs: %#lx fs: %#lx shadow: %#lx",
			  si.GSBase, si.FSBase, si.ShadowGSBase);
		si.SignalMask = SignalMask.load();
		si.Compatibility = ((PCB *)ctx)->Info.Compatibility;

		/* Copy the stack info */
		uint64_t *pRsp = (uint64_t *)(paRsp - sizeof(StackInfo));
		memcpy(pRsp, &si, sizeof(StackInfo));

		/* Set the handler address */
		pRsp--; /* Alignment */
		pRsp--;
		*pRsp = uint64_t(SignalAction[sigI.sig].__sa_handler.sa_handler);

		assert(!((uintptr_t)pRsp & 0xF));

#ifdef DEBUG
		DumpData("Stack Data", (void *)pRsp,
				 paRsp - uint64_t(pRsp));
		debug("initial stack tf->rsp: %#lx after: %#lx",
			  tf->rsp, uint64_t(vRsp));
		debug("sig: %d -> %d", sigI.sig, CTLif(sigI.sig));
#endif

		tf->rsp = uint64_t(vRsp);
		tf->rip = uint64_t(TrampAddr);
		tf->rdi = CTLif(sigI.sig);
		tf->rsi = uint64_t(sigI.val.sival_ptr);

		assert(TrampAddr != nullptr);
		return true;
	}

	void Signal::RestoreHandleSignal(SyscallsFrame *sf)
	{
		SmartLock(SignalLock);
		debug("Restoring signal handler");
		gsTCB *gs = (gsTCB *)CPU::x64::rdmsr(CPU::x64::MSR_GS_BASE);
		uint64_t *sp = (uint64_t *)((PCB *)ctx)->PageTable->Get(gs->TempStack);
		sp++; /* Alignment */
		sp++; /* Handler Address */

		assert(!((uintptr_t)sp & 0xF));

		StackInfo *si = (StackInfo *)sp;
		assert(si != nullptr);

		sf->r15 = si->tf.r15;
		sf->r14 = si->tf.r14;
		sf->r13 = si->tf.r13;
		sf->r12 = si->tf.r12;
		sf->r11 = si->tf.r11;
		sf->r10 = si->tf.r10;
		sf->r9 = si->tf.r9;
		sf->r8 = si->tf.r8;
		sf->rbp = si->tf.rbp;
		sf->rdi = si->tf.rdi;
		sf->rsi = si->tf.rsi;
		sf->rdx = si->tf.rdx;
		sf->rcx = si->tf.rcx;
		sf->rbx = si->tf.rbx;
		sf->rax = si->tf.rax;
		sf->Flags = si->tf.rflags.raw;
		sf->ReturnAddress = si->tf.rip;
		gs->TempStack = (void *)si->tf.rsp;

		SignalMask.store(si->SignalMask);

		CPU::x64::fxrstor(&si->fx);
		CPU::x64::wrmsr(CPU::x64::MSR_GS_BASE, si->ShadowGSBase);
		CPU::x64::wrmsr(CPU::x64::MSR_FS_BASE, si->FSBase);
		CPU::x64::wrmsr(CPU::x64::MSR_SHADOW_GS_BASE, si->GSBase);
		debug("gs: %#lx fs: %#lx shadow: %#lx",
			  si->GSBase, si->FSBase, si->ShadowGSBase);

		// ((PCB *)ctx)->GetContext()->Yield();
		// __builtin_unreachable();
		/* Return because we will restore at sysretq */
	}

	sigset_t Signal::Block(sigset_t sig)
	{
		SmartLock(SignalLock);
		sig = CSigTNif(sig);
		sigset_t oldMask = SignalMask.fetch_or(sig);
		debug("%#lx -> %#lx", oldMask, SignalMask);
		return CSigTLif(oldMask);
	}

	sigset_t Signal::Unblock(sigset_t sig)
	{
		SmartLock(SignalLock);
		sig = CSigTNif(sig);
		sigset_t oldMask = SignalMask.fetch_and(~sig);
		debug("%#lx -> %#lx", oldMask, SignalMask);
		return CSigTLif(oldMask);
	}

	sigset_t Signal::SetMask(sigset_t sig)
	{
		SmartLock(SignalLock);
		sig = CSigTNif(sig);
		sigset_t oldMask = SignalMask.exchange(sig);
		debug("%#lx -> %#lx", oldMask, SignalMask);
		return CSigTLif(oldMask);
	}

	int Signal::SetAction(int sig, const sigaction act)
	{
		SmartLock(SignalLock);
		if ((size_t)sig > sizeof(SignalAction) / sizeof(SignalAction[0]))
			return -EINVAL;

		sig = CTNif(sig);

		SignalAction[sig].__sa_handler.sa_handler = act.__sa_handler.sa_handler;
		SignalAction[sig].sa_mask = act.sa_mask;
		SignalAction[sig].sa_flags = act.sa_flags;
		debug("Set action for %s with handler %#lx, mask %#lx and flags %#lx",
			  LinuxSig() ? lSigStr[sig] : sigStr[sig],
			  SignalAction[sig].__sa_handler.sa_handler,
			  SignalAction[sig].sa_mask,
			  SignalAction[sig].sa_flags);
		return 0;
	}

	int Signal::GetAction(int sig, sigaction *act)
	{
		SmartLock(SignalLock);

		if ((size_t)sig > sizeof(SignalAction) / sizeof(SignalAction[0]))
			return -EINVAL;

		sig = CTNif(sig);

		act->__sa_handler.sa_handler = SignalAction[sig].__sa_handler.sa_handler;
		act->sa_mask = SignalAction[sig].sa_mask;
		act->sa_flags = SignalAction[sig].sa_flags;
		debug("Got action for %s with handler %#lx, mask %#lx and flags %#lx",
			  LinuxSig() ? lSigStr[sig] : sigStr[sig],
			  SignalAction[sig].__sa_handler.sa_handler,
			  SignalAction[sig].sa_mask,
			  SignalAction[sig].sa_flags);
		return 0;
	}

	int Signal::SendSignal(int sig, union sigval val)
	{
		PCB *pcb = (PCB *)ctx;
		sig = CTNif(sig);
		LastSignal = (Signals)sig;

		debug("Sending signal %s to %s(%d)",
			  sigStr[sig], pcb->Name, pcb->ID);

		if (SignalAction[sig].__sa_handler.sa_handler)
		{
			if (pcb->Security.ExecutionMode == Kernel)
			{
				info("Kernel processes cannot have signal handlers");
				return -EINVAL;
			}

			debug("sa_handler: %#lx",
				  SignalAction[sig].__sa_handler.sa_handler);
			debug("Adding signal %s to queue", sigStr[sig]);
			goto CompleteSignal;
		}

		debug("Signal disposition: %s", dispStr[sigDisp[sig]]);
		switch (sigDisp[sig])
		{
		case SIG_TERM:
		{
			if (unlikely(pcb->Security.IsCritical))
			{
				debug("Critical process %s received signal %s(%d): Terminated",
					  pcb->Name, sigStr[sig], sig);
				// int3;
			}

			pcb->SetExitCode(MakeExitCode(sig));
			debug("We have %d watchers", this->Watchers.size());
			if (this->Watchers.size() > 0)
				pcb->SetState(Zombie);
			else
				pcb->SetState(Terminated);
			break;
		}
		case SIG_IGN:
		{
			debug("Ignoring signal %d", sig);
			return 0;
		}
		case SIG_CORE:
		{
			fixme("core dump");

			if (unlikely(pcb->Security.IsCritical))
			{
				debug("Critical process %s received signal %s(%d): Core dumped",
					  pcb->Name, sigStr[sig], sig);
				// int3;
			}

			pcb->SetExitCode(MakeExitCode(sig));
			debug("We have %d watchers", this->Watchers.size());
			if (this->Watchers.size() > 0)
				pcb->SetState(CoreDump);
			else
				pcb->SetState(Terminated);
			break;
		}
		case SIG_STOP:
		{
			pcb->SetState(Stopped);
			break;
		}
		case SIG_CONT:
		{
			pcb->SetState(Ready);
			break;
		}
		default:
			assert(!"Invalid signal disposition");
			return -EINVAL;
		};

	CompleteSignal:
		if (pcb->Security.ExecutionMode == Kernel)
		{
			debug("Kernel process %s received signal %s(%d)! Ignoring... (with exceptions)",
				  pcb->Name, sigStr[sig], sig);
			return 0;
		}

		this->InitTrampoline();

		debug("Signal %s(%d) completed", sigStr[sig], sig);
		if (sigDisp[sig] != SIG_IGN)
		{
			foreach (auto info in Watchers)
			{
				Signal *who = (Signal *)info.val.sival_ptr;
				assert(who != nullptr);
				debug("Sending SIGCHLD to %s(%d)",
					  ((PCB *)who->GetContext())->Name,
					  ((PCB *)who->GetContext())->ID);
				who->SendSignal(SIGCHLD, val);
			}
		}

		debug("Adding signal to queue");
		SignalQueue.push_back({.sig = sig, .val = val});
		return 0;
	}

	int Signal::WaitAnySignal()
	{
		/* Sleep until a signal that terminated or invokes
		the signal catch function */

		debug("Waiting for any signal");

		size_t oldSize = SignalQueue.size();
	Reset:
		while (SignalQueue.size() == oldSize)
			TaskManager->Yield();

		if (SignalQueue.size() > oldSize)
		{
			debug("Added signal to queue %d > %d",
				  SignalQueue.size(), oldSize);
			oldSize = SignalQueue.size();
			goto Reset;
		}

		debug("Signal received");
		return -EINTR;
	}

	bool Signal::HasPendingSignal()
	{
		return !SignalQueue.empty();
	}

	int Signal::WaitSignal(int sig, union sigval *val)
	{
		return 0;
	}

	int Signal::WaitSignalTimeout(int sig, union sigval *val, uint64_t timeout)
	{
		return 0;
	}

	Signal::Signal(void *ctx)
	{
		assert(ctx != nullptr);
		this->ctx = ctx;

		sigDisp[SIG_NULL] = SIG_IGN;
		sigDisp[SIGABRT] = SIG_CORE;
		sigDisp[SIGALRM] = SIG_TERM;
		sigDisp[SIGBUS] = SIG_CORE;
		sigDisp[SIGCHLD] = SIG_IGN;
		sigDisp[SIGCONT] = SIG_CONT;
		sigDisp[SIGFPE] = SIG_CORE;
		sigDisp[SIGHUP] = SIG_TERM;
		sigDisp[SIGILL] = SIG_CORE;
		sigDisp[SIGINT] = SIG_TERM;
		sigDisp[SIGKILL] = SIG_TERM;
		sigDisp[SIGPIPE] = SIG_TERM;
		sigDisp[SIGQUIT] = SIG_TERM;
		sigDisp[SIGSEGV] = SIG_CORE;
		sigDisp[SIGSTOP] = SIG_STOP;
		sigDisp[SIGTERM] = SIG_TERM;
		sigDisp[SIGTSTP] = SIG_STOP;
		sigDisp[SIGTTIN] = SIG_STOP;
		sigDisp[SIGTTOU] = SIG_STOP;
		sigDisp[SIGUSR1] = SIG_TERM;
		sigDisp[SIGUSR2] = SIG_TERM;
		sigDisp[SIGPOLL] = SIG_TERM;
		sigDisp[SIGPROF] = SIG_TERM;
		sigDisp[SIGSYS] = SIG_CORE;
		sigDisp[SIGTRAP] = SIG_CORE;
		sigDisp[SIGURG] = SIG_IGN;
		sigDisp[SIGVTALRM] = SIG_TERM;
		sigDisp[SIGXCPU] = SIG_CORE;
		sigDisp[SIGXFSZ] = SIG_CORE;

#ifdef DEBUG
		static int once = 0;
		if (!once++)
		{
			if (LinuxSig())
			{
				for (int i = 0; i <= linux_SIGUNUSED; i++)
					debug("%s: %s",
						  lSigStr[i],
						  dispStr[sigDisp[i]]);
			}
			else
			{
				for (int i = 0; i < SIGNAL_MAX; i++)
					debug("%s: %s",
						  sigStr[i],
						  dispStr[sigDisp[i]]);
			}
		}
#endif
	}

	Signal::~Signal() {}
}
