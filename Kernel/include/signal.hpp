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

#include <interface/syscalls.h>
#include <unordered_map>
#include <syscalls.hpp>
#include <lock.hpp>
#include <types.h>
#include <bitset>
#include <list>

#define SIGNULL __SYS_SIGNULL
#define SIGABRT __SYS_SIGABRT
#define SIGALRM __SYS_SIGALRM
#define SIGBUS __SYS_SIGBUS
#define SIGCHLD __SYS_SIGCHLD
#define SIGCONT __SYS_SIGCONT
#define SIGFPE __SYS_SIGFPE
#define SIGHUP __SYS_SIGHUP
#define SIGILL __SYS_SIGILL
#define SIGINT __SYS_SIGINT
#define SIGKILL __SYS_SIGKILL
#define SIGPIPE __SYS_SIGPIPE
#define SIGQUIT __SYS_SIGQUIT
#define SIGSEGV __SYS_SIGSEGV
#define SIGSTOP __SYS_SIGSTOP
#define SIGTERM __SYS_SIGTERM
#define SIGTSTP __SYS_SIGTSTP
#define SIGTTIN __SYS_SIGTTIN
#define SIGTTOU __SYS_SIGTTOU
#define SIGUSR1 __SYS_SIGUSR1
#define SIGUSR2 __SYS_SIGUSR2
#define SIGPOLL __SYS_SIGPOLL
#define SIGPROF __SYS_SIGPROF
#define SIGSYS __SYS_SIGSYS
#define SIGTRAP __SYS_SIGTRAP
#define SIGURG __SYS_SIGURG
#define SIGVTALRM __SYS_SIGVTALRM
#define SIGXCPU __SYS_SIGXCPU
#define SIGXFSZ __SYS_SIGXFSZ
#define SIGCOMP1 __SYS_SIGCOMP1
#define SIGCOMP2 __SYS_SIGCOMP2
#define SIGCOMP3 __SYS_SIGCOMP3
#define SIGRTMIN __SYS_SIGRTMIN
#define SIGRT_1 __SYS_SIGRT_1
#define SIGRT_2 __SYS_SIGRT_2
#define SIGRT_3 __SYS_SIGRT_3
#define SIGRT_4 __SYS_SIGRT_4
#define SIGRT_5 __SYS_SIGRT_5
#define SIGRT_6 __SYS_SIGRT_6
#define SIGRT_7 __SYS_SIGRT_7
#define SIGRT_8 __SYS_SIGRT_8
#define SIGRT_9 __SYS_SIGRT_9
#define SIGRT_10 __SYS_SIGRT_10
#define SIGRT_11 __SYS_SIGRT_11
#define SIGRT_12 __SYS_SIGRT_12
#define SIGRT_13 __SYS_SIGRT_13
#define SIGRT_14 __SYS_SIGRT_14
#define SIGRT_15 __SYS_SIGRT_15
#define SIGRT_16 __SYS_SIGRT_16
#define SIGRT_17 __SYS_SIGRT_17
#define SIGRT_18 __SYS_SIGRT_18
#define SIGRT_19 __SYS_SIGRT_19
#define SIGRT_20 __SYS_SIGRT_20
#define SIGRT_21 __SYS_SIGRT_21
#define SIGRT_22 __SYS_SIGRT_22
#define SIGRT_23 __SYS_SIGRT_23
#define SIGRT_24 __SYS_SIGRT_24
#define SIGRT_25 __SYS_SIGRT_25
#define SIGRT_26 __SYS_SIGRT_26
#define SIGRT_27 __SYS_SIGRT_27
#define SIGRT_28 __SYS_SIGRT_28
#define SIGRT_29 __SYS_SIGRT_29
#define SIGRT_30 __SYS_SIGRT_30
#define SIGRT_31 __SYS_SIGRT_31
#define SIGRTMAX __SYS_SIGRTMAX
#define SIGNAL_MAX __SYS_SIGNAL_MAX

#define SIG_TERM __SYS_SIG_TERM
#define SIG_IGN __SYS_SIG_IGN
#define SIG_CORE __SYS_SIG_CORE
#define SIG_STOP __SYS_SIG_STOP
#define SIG_CONT __SYS_SIG_CONT

#define SIG_BLOCK __SYS_SIG_BLOCK
#define SIG_UNBLOCK __SYS_SIG_UNBLOCK
#define SIG_SETMASK __SYS_SIG_SETMASK

#define SIG_ERR __SYS_SIG_ERR
#define SIG_DFL __SYS_SIG_DFL
#define SIG_IGN __SYS_SIG_IGN

#define SA_NOCLDSTOP 1
#define SA_NOCLDWAIT 2
#define SA_SIGINFO 4
#define SA_ONSTACK 0x08000000
#define SA_RESTART 0x10000000
#define SA_NODEFER 0x40000000
#define SA_RESETHAND 0x80000000

static_assert(SA_NOCLDSTOP == __SYS_SA_NOCLDSTOP);
static_assert(SA_NOCLDWAIT == __SYS_SA_NOCLDWAIT);
static_assert(SA_SIGINFO == __SYS_SA_SIGINFO);
static_assert(SA_ONSTACK == __SYS_SA_ONSTACK);
static_assert(SA_RESTART == __SYS_SA_RESTART);
static_assert(SA_NODEFER == __SYS_SA_NODEFER);
static_assert(SA_RESETHAND == __SYS_SA_RESETHAND);

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
			int sig = SIGNULL;
			union sigval val
			{
				0
			};
			pid_t tid = -1;
		};

		struct StackInfo
		{
#ifdef __amd64__
			CPU::x64::FXState fx;
			CPU::x64::SchedulerFrame tf;
			uintptr_t GSBase, FSBase, ShadowGSBase;
#else
			CPU::x32::FXState fx;
			CPU::x32::SchedulerFrame tf;
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
		signal_t LastSignal = SIGNULL;

		// Signal trampoline
		void *TrampAddr = nullptr;
		size_t TrampSz = 0;

		std::list<SignalInfo> Queue;
		SignalAction sa[64 + 1]{};
		// std::bitset<SIGNAL_MAX> GlobalMask;
		// signal_disposition_t Disposition[SIGNAL_MAX];
		std::list<SignalInfo> Watchers;
		std::unordered_map<signal_t, signal_disposition_t> Disposition = {
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
		int MakeExitCode(signal_t sig);
		void InitTrampoline();
		SignalInfo GetAvailableSignal(void *thread);

	public:
		void *GetContext() { return ctx; }
		signal_t GetLastSignal() { return LastSignal; }

		int AddWatcher(Signal *who, signal_t sig);
		int RemoveWatcher(Signal *who, signal_t sig);

		int AddSignal(signal_t sig, union sigval val = {0}, pid_t tid = -1);
		int RemoveSignal(signal_t sig);

		bool HandleSignal(CPU::SchedulerFrame *tf, void *thread);
		void RestoreHandleSignal(SyscallsFrame *tf, void *thread);

		int SetAction(signal_t sig, const SignalAction *act);
		int GetAction(signal_t sig, SignalAction *act);

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
		int SendSignal(signal_t sig, sigval val = {0}, pid_t tid = -1);

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
		int WaitSignal(signal_t sig, union sigval *val);

		/**
		 * Wait for a signal with a timeout
		 *
		 * @param sig The signal to wait for
		 * @param val The value to wait for
		 * @param timeout The timeout to wait for
		 *
		 * @return 0 on success, -errno on error
		 */
		int WaitSignalTimeout(signal_t sig, union sigval *val, uint64_t timeout);

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
