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

#ifndef __FENNIX_KERNEL_SIGNAL_H__
#define __FENNIX_KERNEL_SIGNAL_H__

#include <syscalls.hpp>
#include <lock.hpp>
#include <types.h>
#include <list>
#include <syscall/linux/signals.hpp>

enum Signals : int
{
	SIG_NULL = 0,

	/**
	 * Process abort signal.
	 */
	SIGABRT,

	/**
	 * Alarm clock.
	 */
	SIGALRM,

	/**
	 * Access to an undefined portion of a memory object.
	 */
	SIGBUS,

	/**
	 * Child process terminated, stopped, or continued.
	 */
	SIGCHLD,

	/**
	 * Continue executing, if stopped.
	 */
	SIGCONT,

	/**
	 * Erroneous arithmetic operation.
	 */
	SIGFPE,

	/**
	 * Hangup.
	 */
	SIGHUP,

	/**
	 * Illegal instruction.
	 */
	SIGILL,

	/**
	 * Terminal interrupt signal.
	 */
	SIGINT,

	/**
	 * Kill (cannot be caught or ignored).
	 */
	SIGKILL,

	/**
	 * Write on a pipe with no one to read it.
	 */
	SIGPIPE,

	/**
	 * Terminal quit signal.
	 */
	SIGQUIT,

	/**
	 * Invalid memory reference.
	 */
	SIGSEGV,

	/**
	 * Stop executing (cannot be caught or ignored).
	 */
	SIGSTOP,

	/**
	 * Termination signal.
	 */
	SIGTERM,

	/**
	 * Terminal stop signal.
	 */
	SIGTSTP,

	/**
	 * Background process attempting read.
	 */
	SIGTTIN,

	/**
	 * Background process attempting write.
	 */
	SIGTTOU,

	/**
	 * User-defined signal 1.
	 */
	SIGUSR1,

	/**
	 * User-defined signal 2.
	 */
	SIGUSR2,

	/**
	 * Pollable event.
	 */
	SIGPOLL,

	/**
	 * Profiling timer expired.
	 */
	SIGPROF,

	/**
	 * Bad system call.
	 */
	SIGSYS,

	/**
	 * Trace/breakpoint trap.
	 */
	SIGTRAP,

	/**
	 * High bandwidth data is available at a socket.
	 */
	SIGURG,

	/**
	 * Virtual timer expired.
	 */
	SIGVTALRM,

	/**
	 * CPU time limit exceeded.
	 */
	SIGXCPU,

	/**
	 * File size limit exceeded.
	 */
	SIGXFSZ,

	/**
	 * Reserved
	 *
	 * These are just to match Linux's signal numbers.
	 */
	SIGRSV1,
	SIGRSV2,

	/**
	 * Maximum signal number.
	 */
	SIGNAL_MAX
};

enum SignalDisposition
{
	/**
	 * Terminate the process.
	 */
	SIG_TERM,

	/**
	 * Ignore the signal.
	 */
	SIG_IGN,

	/**
	 * Dump core.
	 */
	SIG_CORE,

	/**
	 * Stop the process.
	 */
	SIG_STOP,

	/**
	 * Continue the process.
	 */
	SIG_CONT
};

enum SignalAction
{
	SIG_BLOCK,
	SIG_UNBLOCK,
	SIG_SETMASK
};

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000
#define SA_RESTORER 0x04000000

#define __SI_PAD_SIZE \
	(128 - 2 * sizeof(int) - sizeof(long))

typedef unsigned long sigset_t;

union sigval
{
	int sival_int;
	void *sival_ptr;
};

struct sched_param
{
	int sched_priority;
};

struct pthread_attr_t
{
	uint64_t sig;
	size_t guard_sz;
	bool detach;
	sched_param sched;
};

struct sigevent
{
	int sigev_notify;
	int sigev_signo;
	union sigval sigev_value;
	void (*sigev_notify_function)(union sigval);
	pthread_attr_t *sigev_notify_attributes;
};

typedef struct
{
	int si_signo;
	int si_errno;
	int si_code;
	union
	{
		char __pad[__SI_PAD_SIZE];
		struct
		{
			union
			{
				struct
				{
					pid_t si_pid;
					uid_t si_uid;
				} __piduid;
				struct
				{
					int si_timerid;
					int si_overrun;
				} __timer;
			} __first;
			union
			{
				union sigval si_value;
				struct
				{
					int si_status;
					clock_t si_utime, si_stime;
				} __sigchld;
			} __second;
		} __si_common;

		struct
		{
			void *si_addr;
			short si_addr_lsb;
			union
			{
				struct
				{
					void *si_lower;
					void *si_upper;
				} __addr_bnd;
				unsigned si_pkey;
			} __first;
		} __sigfault;

		struct
		{
			long si_band;
			int si_fd;
		} __sigpoll;

		struct
		{
			void *si_call_addr;
			int si_syscall;
			unsigned si_arch;
		} __sigsys;
	} __si_fields;
} siginfo_t;

struct sigaction
{
	union
	{
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	} __sa_handler;
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};

namespace Tasking
{
	__no_sanitize("shift") inline __always_inline
		sigset_t
		ToFlag(sigset_t sig)
	{
		return 1 << (sig - 1);
	}

	inline __always_inline
		sigset_t
		ToSig(sigset_t flag)
	{
		return __builtin_ctzl(flag) + 1;
	}

	class Signal
	{
	private:
		struct SignalInfo
		{
			int sig;
			union sigval val;
		};

		struct StackInfo
		{
#ifdef a64
			CPU::x64::FXState fx;
			CPU::x64::TrapFrame tf;
			uintptr_t GSBase, FSBase, ShadowGSBase;
#else
			CPU::x32::FXState fx;
			CPU::x32::TrapFrame tf;
			uintptr_t GSBase, FSBase;
#endif
			sigset_t SignalMask;
			int Compatibility;

#ifdef DEBUG
			// For debugging purposes
			char dbg[6] = {'S', 'I', 'G', 'N', 'A', 'L'};
#endif
		} __aligned(16) __packed;

		NewLock(SignalLock);
		void *ctx;
		Signals LastSignal = SIG_NULL;

		// Signal trampoline
		void *TrampAddr = nullptr;
		size_t TrampSz = 0;

		std::list<SignalInfo> SignalQueue;
		std::atomic<sigset_t> SignalMask = 0;
		sigaction SignalAction[SIGNAL_MAX]{};
		SignalDisposition sigDisp[SIGNAL_MAX];
		std::list<SignalInfo> Watchers;

		bool LinuxSig();

		int ConvertToLinuxIfNecessary(int sig);
		int ConvertToNativeIfNecessary(int sig);

		sigset_t ConvertSigsetToLinuxIfNecessary(sigset_t sig);
		sigset_t ConvertSigsetToNativeIfNecessary(sigset_t sig);

		int MakeExitCode(int sig);

		void InitTrampoline();

		const sigset_t nMasks = ToFlag(SIGKILL) |
								ToFlag(SIGSTOP) |
								ToFlag(SIGCONT) |
								ToFlag(SIGSEGV) |
								ToFlag(SIGBUS) |
								ToFlag(SIGILL) |
								ToFlag(SIGFPE);

		const sigset_t lMasks = ToFlag(linux_SIGKILL) |
								ToFlag(linux_SIGSTOP) |
								ToFlag(linux_SIGCONT) |
								ToFlag(linux_SIGSEGV) |
								ToFlag(linux_SIGBUS) |
								ToFlag(linux_SIGILL) |
								ToFlag(linux_SIGFPE);

		void RemoveUnmaskable(sigset_t *sig)
		{
			if (LinuxSig())
				*sig &= ~lMasks;
			else
				*sig &= ~nMasks;
		}

		bool CanHaveHandler(sigset_t sig)
		{
			switch (sig)
			{
			case SIGKILL:
			case SIGSTOP:
			case SIGCONT:
				return false;
			default:
				return true;
			}
		}

	public:
		void *GetContext() { return ctx; }
		Signals GetLastSignal() { return LastSignal; }

		int AddWatcher(Signal *who, int sig);
		int RemoveWatcher(Signal *who, int sig);

		int AddSignal(int sig, union sigval val);
		int RemoveSignal(int sig);

		/**
		 * For scheduler use only
		 * @return True if there is a signal to handle
		 */
		bool HandleSignal(CPU::TrapFrame *tf);
		void RestoreHandleSignal(SyscallsFrame *tf);

		/**
		 * Mask a signal
		 *
		 * @param sig The signal to set
		 *
		 * @return Old mask
		 */
		sigset_t Block(sigset_t sig);

		/**
		 * Unmask a signal
		 *
		 * @param sig The signal to set
		 *
		 * @return Old mask
		 */
		sigset_t Unblock(sigset_t sig);

		/**
		 * Set the signal mask
		 *
		 * @param sig The signal to set
		 *
		 * @return Old mask
		 */
		sigset_t SetMask(sigset_t sig);

		sigset_t GetMask() { return SignalMask.load(); }

		int SetAction(int sig, const sigaction act);
		int GetAction(int sig, sigaction *act);

		/**
		 * Send a signal to the process
		 *
		 * @param sig The signal to send
		 * (compatibility specific)
		 * @param val The value to send
		 *
		 * @return 0 on success, -errno on error
		 */
		int SendSignal(int sig, union sigval val = {0});

		int WaitAnySignal();
		bool HasPendingSignal();

		/**
		 * Wait for a signal
		 *
		 * @param sig The signal to wait for
		 * (compatibility specific)
		 * @param val The value to wait for
		 *
		 * @return 0 on success, -errno on error
		 */
		int WaitSignal(int sig, union sigval *val);

		/**
		 * Wait for a signal with a timeout
		 *
		 * @param sig The signal to wait for
		 * (compatibility specific)
		 * @param val The value to wait for
		 * @param timeout The timeout to wait for
		 *
		 * @return 0 on success, -errno on error
		 */
		int WaitSignalTimeout(int sig, union sigval *val, uint64_t timeout);

		Signal(void *ctx);
		~Signal();
	};
}

#endif // !__FENNIX_KERNEL_SIGNAL_H__
