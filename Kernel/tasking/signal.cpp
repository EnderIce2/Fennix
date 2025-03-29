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

#include "../kernel.h"

#ifdef DEBUG
const char *sigStr[] = {
	/*  0 */ "INVALID",
	/*  1 */ "SIGABRT",
	/*  2 */ "SIGALRM",
	/*  3 */ "SIGBUS",
	/*  4 */ "SIGCHLD",
	/*  5 */ "SIGCONT",
	/*  6 */ "SIGFPE",
	/*  7 */ "SIGHUP",
	/*  8 */ "SIGILL",
	/*  9 */ "SIGINT",
	/* 10 */ "SIGKILL",
	/* 11 */ "SIGPIPE",
	/* 12 */ "SIGQUIT",
	/* 13 */ "SIGSEGV",
	/* 14 */ "SIGSTOP",
	/* 15 */ "SIGTERM",
	/* 16 */ "SIGTSTP",
	/* 17 */ "SIGTTIN",
	/* 18 */ "SIGTTOU",
	/* 19 */ "SIGUSR1",
	/* 20 */ "SIGUSR2",
	/* 21 */ "SIGPOLL",
	/* 22 */ "SIGPROF",
	/* 23 */ "SIGSYS",
	/* 24 */ "SIGTRAP",
	/* 25 */ "SIGURG",
	/* 26 */ "SIGVTALRM",
	/* 27 */ "SIGXCPU",
	/* 28 */ "SIGXFSZ",
	/* 29 */ "SIGCOMP1",
	/* 30 */ "SIGCOMP2",
	/* 31 */ "SIGCOMP3",
	/* 32 */ "SIGRTMIN",
	/* 33 */ "SIGRT_1",
	/* 34 */ "SIGRT_2",
	/* 35 */ "SIGRT_3",
	/* 36 */ "SIGRT_4",
	/* 37 */ "SIGRT_5",
	/* 38 */ "SIGRT_6",
	/* 39 */ "SIGRT_7",
	/* 40 */ "SIGRT_8",
	/* 41 */ "SIGRT_9",
	/* 42 */ "SIGRT_10",
	/* 43 */ "SIGRT_11",
	/* 44 */ "SIGRT_12",
	/* 45 */ "SIGRT_13",
	/* 46 */ "SIGRT_14",
	/* 47 */ "SIGRT_15",
	/* 48 */ "SIGRT_16",
	/* 49 */ "SIGRT_17",
	/* 50 */ "SIGRT_18",
	/* 51 */ "SIGRT_19",
	/* 52 */ "SIGRT_20",
	/* 53 */ "SIGRT_21",
	/* 54 */ "SIGRT_22",
	/* 55 */ "SIGRT_23",
	/* 56 */ "SIGRT_24",
	/* 57 */ "SIGRT_25",
	/* 58 */ "SIGRT_26",
	/* 59 */ "SIGRT_27",
	/* 60 */ "SIGRT_28",
	/* 61 */ "SIGRT_29",
	/* 62 */ "SIGRT_30",
	/* 63 */ "SIGRT_31",
	/* 64 */ "SIGRTMAX",
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
	signal_t Signal;
	signal_disposition_t Disposition;
} SignalDisposition[] = {
	{SIGHUP, SIG_TERM},
	{SIGINT, SIG_TERM},
	{SIGQUIT, SIG_TERM},
	{SIGILL, SIG_CORE},
	{SIGTRAP, SIG_CORE},
	{SIGABRT, SIG_CORE},
	{SIGBUS, SIG_CORE},
	{SIGFPE, SIG_CORE},
	{SIGKILL, SIG_TERM},
	{SIGUSR1, SIG_TERM},
	{SIGSEGV, SIG_CORE},
	{SIGUSR2, SIG_TERM},
	{SIGPIPE, SIG_TERM},
	{SIGALRM, SIG_TERM},
	{SIGTERM, SIG_TERM},
	{SIGCOMP1, SIG_IGN},
	{SIGCHLD, SIG_IGN},
	{SIGCONT, SIG_CONT},
	{SIGSTOP, SIG_STOP},
	{SIGTSTP, SIG_STOP},
	{SIGTTIN, SIG_STOP},
	{SIGTTOU, SIG_STOP},
	{SIGURG, SIG_IGN},
	{SIGXCPU, SIG_CORE},
	{SIGXFSZ, SIG_CORE},
	{SIGVTALRM, SIG_TERM},
	{SIGPROF, SIG_TERM},
	{SIGCOMP2, SIG_IGN},
	{SIGPOLL, SIG_TERM},
	{SIGCOMP3, SIG_IGN},
	{SIGSYS, SIG_CORE},
	{SIGRTMIN, SIG_IGN},
	{SIGRT_1, SIG_IGN},
	{SIGRT_2, SIG_IGN},
	{SIGRT_3, SIG_IGN},
	{SIGRT_4, SIG_IGN},
	{SIGRT_5, SIG_IGN},
	{SIGRT_6, SIG_IGN},
	{SIGRT_7, SIG_IGN},
	{SIGRT_8, SIG_IGN},
	{SIGRT_9, SIG_IGN},
	{SIGRT_10, SIG_IGN},
	{SIGRT_11, SIG_IGN},
	{SIGRT_12, SIG_IGN},
	{SIGRT_13, SIG_IGN},
	{SIGRT_14, SIG_IGN},
	{SIGRT_15, SIG_IGN},
	{SIGRT_16, SIG_IGN},
	{SIGRT_17, SIG_IGN},
	{SIGRT_18, SIG_IGN},
	{SIGRT_19, SIG_IGN},
	{SIGRT_20, SIG_IGN},
	{SIGRT_21, SIG_IGN},
	{SIGRT_22, SIG_IGN},
	{SIGRT_23, SIG_IGN},
	{SIGRT_24, SIG_IGN},
	{SIGRT_25, SIG_IGN},
	{SIGRT_26, SIG_IGN},
	{SIGRT_27, SIG_IGN},
	{SIGRT_28, SIG_IGN},
	{SIGRT_29, SIG_IGN},
	{SIGRT_30, SIG_IGN},
	{SIGRT_31, SIG_IGN},
	{SIGRTMAX, SIG_IGN},
};

signal_disposition_t GetDefaultSignalDisposition(signal_t sig)
{
	for (auto var : SignalDisposition)
	{
		if (var.Signal == sig)
			return var.Disposition;
	}

	error("Invalid signal: %d", sig);
	return SIG_TERM;
}

/* subsystem/linux/syscall.cpp */
extern int ConvertSignalToLinux(signal_t sig);

namespace Tasking
{
	bool Signal::LinuxSig()
	{
		return ((PCB *)ctx)->Info.Compatibility == Linux;
	}

	int Signal::MakeExitCode(signal_t sig)
	{
		if (this->LinuxSig())
			return 128 + ConvertSignalToLinux(sig);
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
			assert(!"Windows compatibility not implemented");
			break;
		}
		default:
			/* Process not fully initalized. */
			return;
		}
	}

	/* ------------------------------------------------------ */

	int Signal::AddWatcher(Signal *who, signal_t sig)
	{
		SignalInfo info;
		info.sig = sig;
		info.val.sival_ptr = who;

		SmartLock(SignalLock);
		Watchers.push_back(info);
		return 0;
	}

	int Signal::RemoveWatcher(Signal *who, signal_t sig)
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

	int Signal::AddSignal(signal_t sig, union sigval val, pid_t tid)
	{
		SignalInfo info{.sig = sig, .val = val, .tid = tid};
		Queue.push_back(info);
		return 0;
	}

	int Signal::RemoveSignal(signal_t sig)
	{
		size_t n = Queue.remove_if([sig](SignalInfo &info)
								   { return info.sig == sig; });
		debug("Removed %d signals", n);
		return n ? 0 : -ENOENT;
	}

	Signal::SignalInfo Signal::GetAvailableSignal(void *thread)
	{
		forItr(itr, Queue)
		{
			// if (GlobalMask.test(itr->sig - 1))
			// {
			// 	debug("Signal %s is blocked by global mask",
			// 		  sigStr[itr->sig]);
			// 	continue;
			// }

			if (((TCB *)thread)->Signals.Mask.test(itr->sig - 1))
			{
				debug("Signal %s is blocked by thread mask",
					  sigStr[itr->sig]);
				continue;
			}

			if (((TCB *)thread)->ID != itr->tid && itr->tid != -1)
			{
				debug("Signal %s is not for this thread",
					  sigStr[itr->sig]);
				continue;
			}

			assert(sa[itr->sig].sa_handler.Disposition != SIG_IGN);
			assert(sa[itr->sig].sa_handler.Disposition != SIG_DFL);

			Queue.erase(itr);
			debug("Signal %s is available", sigStr[itr->sig]);
			return *itr;
		}

		debug("No signal available");
		return {};
	}

	int Signal::SetAction(signal_t sig, const SignalAction *act)
	{
		SmartLock(SignalLock);
		if ((size_t)sig > sizeof(sa) / sizeof(sa[0]))
		{
			debug("Invalid signal: %d", sig);
			return -EINVAL;
		}

		if ((long)act->sa_handler.Disposition == SIG_IGN)
		{
			Disposition[sig] = SIG_IGN;
			debug("Set disposition for %s to SIG_IGN", sigStr[sig]);
			debug("Discarding pending signals %s", sigStr[sig]);

			bool found = false;
			forItr(itr, Queue)
			{
				if (itr->sig == sig)
				{
					Queue.erase(itr);
					found = true;
					break;
				}
			}

			if (!found)
			{
				debug("No pending signal %s", sigStr[sig]);
			}
		}

		if ((long)act->sa_handler.Disposition == SIG_DFL)
		{
			Disposition[sig] = GetDefaultSignalDisposition(sig);
			debug("Set disposition for %s to %s (default)", sigStr[sig],
				  dispStr[Disposition[sig]]);
		}

		sa[sig].sa_handler.Handler = act->sa_handler.Handler;
		sa[sig].Mask = act->Mask;
		sa[sig].Flags = act->Flags;
		sa[sig].Restorer = act->Restorer;

		debug("Set action for %s with handler %#lx, mask %#lx and flags %#lx",
			  sigStr[sig], sa[sig].sa_handler.Handler, sa[sig].Mask, sa[sig].Flags);
		return 0;
	}

	int Signal::GetAction(signal_t sig, SignalAction *act)
	{
		SmartLock(SignalLock);
		if ((size_t)sig > sizeof(sa) / sizeof(sa[0]))
		{
			debug("Invalid signal: %d", sig);
			return -EINVAL;
		}

		act->sa_handler.Handler = sa[sig].sa_handler.Handler;
		act->Mask = sa[sig].Mask;
		act->Flags = sa[sig].Flags;
		act->Restorer = sa[sig].Restorer;
		debug("Got action for %s with handler %#lx, mask %#lx and flags %#lx",
			  sigStr[sig], sa[sig].sa_handler.Handler, sa[sig].Mask, sa[sig].Flags);
		return 0;
	}

	int Signal::SendSignal(signal_t sig, sigval val, pid_t tid)
	{
		SmartLock(SignalLock);
		PCB *pcb = (PCB *)ctx;
		LastSignal = sig;

		debug("Sending signal %s to %s(%d)",
			  sigStr[sig], pcb->Name, pcb->ID);

		if (sa[sig].sa_handler.Handler)
		{
			if (pcb->Security.ExecutionMode == Kernel)
			{
				info("Kernel processes cannot have signal handlers");
				return -EINVAL;
			}

			if (sa[sig].sa_handler.Disposition == SIG_IGN)
			{
				debug("Ignoring signal %s", sigStr[sig]);
				return 0;
			}

			debug("sa_handler: %#lx", sa[sig].sa_handler.Handler);
			debug("Adding signal %s to queue", sigStr[sig]);
			goto CompleteSignal;
		}
		else
		{
			debug("No handler for signal %s", sigStr[sig]);
		}

		if (thisThread->Signals.Mask.test(sig - 1))
		{
			debug("Signal %s is blocked by thread mask",
				  sigStr[sig]);
			return 0;
		}

		debug("Signal disposition: %s", dispStr[Disposition[sig]]);
		switch (Disposition[sig])
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
			return val.sival_int == Tasking::KILL_CRASH ? -EFAULT : 0;
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
			return val.sival_int == Tasking::KILL_CRASH ? -EFAULT : 0;
		}
		case SIG_STOP:
		{
			debug("Stopping process %s(%d) with signal %s(%d)",
				  pcb->Name, pcb->ID, sigStr[sig], sig);
			pcb->SetState(Stopped);
			return 0;
		}
		case SIG_CONT:
		{
			debug("Continuing process %s(%d) with signal %s(%d)",
				  pcb->Name, pcb->ID, sigStr[sig], sig);
			pcb->SetState(Ready);
			return 0;
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
		if (Disposition[sig] != SIG_IGN)
		{
			for (auto info : Watchers)
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
		Queue.push_back({.sig = sig, .val = val, .tid = tid});
		return 0;
	}

	int Signal::WaitAnySignal()
	{
		debug("Waiting for any signal");

		size_t oldSize = Queue.size();
	Reset:
		while (Queue.size() == oldSize)
			TaskManager->Yield();

		if (Queue.size() > oldSize)
		{
			debug("Added signal to queue %d > %d",
				  Queue.size(), oldSize);
			oldSize = Queue.size();
			goto Reset;
		}

		debug("Signal received");
		return -EINTR;
	}

	int Signal::WaitSignal(signal_t sig, union sigval *val)
	{
		assert(!"WaitSignal not implemented");
		return 0;
	}

	int Signal::WaitSignalTimeout(signal_t sig, union sigval *val, uint64_t timeout)
	{
		assert(!"WaitSignalTimeout not implemented");
		return 0;
	}

	Signal::Signal(void *_ctx)
	{
		assert(_ctx != nullptr);
		this->ctx = _ctx;

		// for (int i = 1; i < SIGNAL_MAX; i++)
		// 	Disposition[i] = GetDefaultSignalDisposition(i);

#ifdef DEBUG
		static int once = 0;
		if (!once++)
		{
			for (int i = 1; i < SIGNAL_MAX; i++)
				debug("%s: %s",
					  sigStr[i],
					  dispStr[Disposition[(signal_t)i]]);
		}
#endif
	}

	Signal::~Signal() {}

	sigset_t ThreadSignal::Block(sigset_t sig)
	{
		sigset_t oldMask = Mask.to_ulong();
		Mask |= sig;
		debug("%#lx -> %#lx", oldMask, Mask);
		return oldMask;
	}

	sigset_t ThreadSignal::Unblock(sigset_t sig)
	{
		sigset_t oldMask = Mask.to_ulong();
		Mask &= ~sig;
		debug("%#lx -> %#lx", oldMask, Mask);
		return oldMask;
	}

	sigset_t ThreadSignal::SetMask(sigset_t sig)
	{
		sigset_t oldMask = Mask.to_ulong();
		Mask = sig;
		debug("%#lx -> %#lx", oldMask, Mask);
		return oldMask;
	}

	sigset_t ThreadSignal::GetMask()
	{
		return Mask.to_ulong();
	}
}
