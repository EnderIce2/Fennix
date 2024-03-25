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

#include <unordered_map>
#include <syscalls.hpp>
#include <lock.hpp>
#include <types.h>
#include <bitset>
#include <list>

enum Signals : int
{
	SIG_NULL = 0,
	/* Process abort signal. */
	SIGABRT = 1,
	/* Alarm clock. */
	SIGALRM = 2,
	/* Access to an undefined portion of a memory object. */
	SIGBUS = 3,
	/* Child process terminated, stopped, or continued. */
	SIGCHLD = 4,
	/* Continue executing, if stopped. */
	SIGCONT = 5,
	/* Erroneous arithmetic operation. */
	SIGFPE = 6,
	/* Hangup. */
	SIGHUP = 7,
	/* Illegal instruction. */
	SIGILL = 8,
	/* Terminal interrupt signal. */
	SIGINT = 9,
	/* Kill (cannot be caught or ignored). */
	SIGKILL = 10,
	/* Write on a pipe with no one to read it. */
	SIGPIPE = 11,
	/* Terminal quit signal. */
	SIGQUIT = 12,
	/* Invalid memory reference. */
	SIGSEGV = 13,
	/* Stop executing (cannot be caught or ignored). */
	SIGSTOP = 14,
	/* Termination signal. */
	SIGTERM = 15,
	/* Terminal stop signal. */
	SIGTSTP = 16,
	/* Background process attempting read. */
	SIGTTIN = 17,
	/* Background process attempting write. */
	SIGTTOU = 18,
	/* User-defined signal 1. */
	SIGUSR1 = 19,
	/* User-defined signal 2. */
	SIGUSR2 = 20,
	/* Pollable event. */
	SIGPOLL = 21,
	/* Profiling timer expired. */
	SIGPROF = 22,
	/* Bad system call. */
	SIGSYS = 23,
	/* Trace/breakpoint trap. */
	SIGTRAP = 24,
	/* High bandwidth data is available at a socket. */
	SIGURG = 25,
	/* Virtual timer expired. */
	SIGVTALRM = 26,
	/* CPU time limit exceeded. */
	SIGXCPU = 27,
	/* File size limit exceeded. */
	SIGXFSZ = 28,

	/**
	 * Reserved
	 * These are just to match Linux's signal numbers.
	 */
	SIGCOMP1 = 29,
	SIGCOMP2 = 30,
	SIGCOMP3 = 31,

	/* Real-time signals. */
	SIGRTMIN = 32,
	SIGRT_1 = 33,
	SIGRT_2 = 34,
	SIGRT_3 = 35,
	SIGRT_4 = 36,
	SIGRT_5 = 37,
	SIGRT_6 = 38,
	SIGRT_7 = 39,
	SIGRT_8 = 40,
	SIGRT_9 = 41,
	SIGRT_10 = 42,
	SIGRT_11 = 43,
	SIGRT_12 = 44,
	SIGRT_13 = 45,
	SIGRT_14 = 46,
	SIGRT_15 = 47,
	SIGRT_16 = 48,
	SIGRT_17 = 49,
	SIGRT_18 = 50,
	SIGRT_19 = 51,
	SIGRT_20 = 52,
	SIGRT_21 = 53,
	SIGRT_22 = 54,
	SIGRT_23 = 55,
	SIGRT_24 = 56,
	SIGRT_25 = 57,
	SIGRT_26 = 58,
	SIGRT_27 = 59,
	SIGRT_28 = 60,
	SIGRT_29 = 61,
	SIGRT_30 = 62,
	SIGRT_31 = 63,
	SIGRTMAX = 64,

	/* Maximum signal number. */
	SIGNAL_MAX = SIGRTMAX
};

enum SignalDispositions
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

enum SignalActions
{
	SIG_BLOCK,
	SIG_UNBLOCK,
	SIG_SETMASK
};

enum SignalActionDisposition : long
{
	SAD_ERR = -1,
	SAD_DFL = 0,
	SAD_IGN = 1,
};

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_RESTORER 0x04000000
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000

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

struct SignalAction
{
	union
	{
		void (*Handler)(int);
		void (*Action)(int, siginfo_t *, void *);
		sigset_t Disposition;
	} sa_handler;
	std::bitset<64> Mask;
	unsigned long Flags;
	void (*Restorer)(void);
};

namespace Tasking
{
	class Signal
	{
	private:
		struct SignalInfo
		{
			int sig = SIG_NULL;
			union sigval val
			{
				0
			};
			pid_t tid = -1;
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

		std::list<SignalInfo> Queue;
		SignalAction sa[64 + 1]{};
		// std::bitset<SIGNAL_MAX> GlobalMask;
		// SignalDispositions Disposition[SIGNAL_MAX];
		std::list<SignalInfo> Watchers;
		std::unordered_map<Signals, SignalDispositions> Disposition = {
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
			{SIGRTMAX, SIG_IGN}};

		bool LinuxSig();
		int MakeExitCode(Signals sig);
		void InitTrampoline();
		SignalInfo GetAvailableSignal(void *thread);

	public:
		void *GetContext() { return ctx; }
		Signals GetLastSignal() { return LastSignal; }

		int AddWatcher(Signal *who, Signals sig);
		int RemoveWatcher(Signal *who, Signals sig);

		int AddSignal(Signals sig, union sigval val = {0}, pid_t tid = -1);
		int RemoveSignal(Signals sig);

		bool HandleSignal(CPU::TrapFrame *tf, void *thread);
		void RestoreHandleSignal(SyscallsFrame *tf, void *thread);

		int SetAction(Signals sig, const SignalAction *act);
		int GetAction(Signals sig, SignalAction *act);

		/**
		 * Send a signal to the process
		 *
		 * @param sig The signal to send
		 * (compatibility specific)
		 * @param val The value to send
		 * @param tid The thread ID to send the signal to
		 *
		 * @return 0 on success, -errno on error
		 */
		int SendSignal(Signals sig, sigval val = {0}, pid_t tid = -1);

		int WaitAnySignal();
		bool HasPendingSignal() { return !Queue.empty(); }

		/**
		 * Wait for a signal
		 *
		 * @param sig The signal to wait for
		 * @param val The value to wait for
		 *
		 * @return 0 on success, -errno on error
		 */
		int WaitSignal(Signals sig, union sigval *val);

		/**
		 * Wait for a signal with a timeout
		 *
		 * @param sig The signal to wait for
		 * @param val The value to wait for
		 * @param timeout The timeout to wait for
		 *
		 * @return 0 on success, -errno on error
		 */
		int WaitSignalTimeout(Signals sig, union sigval *val, uint64_t timeout);

		Signal(void *ctx);
		~Signal();

		friend class ThreadSignal;
	};

	class ThreadSignal
	{
	private:
		Signal &pSig;

	public:
		std::bitset<64> Mask = 0;

		sigset_t Block(sigset_t sig);
		sigset_t Unblock(sigset_t sig);
		sigset_t SetMask(sigset_t sig);
		sigset_t GetMask();

		ThreadSignal(Signal &sig) : pSig(sig) {}
	};
}

#endif // !__FENNIX_KERNEL_SIGNAL_H__
