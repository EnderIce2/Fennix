/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _SIGNAL_H
#define _SIGNAL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>
#include <bits/libc.h>
#include <bits/types/timespec.h>

#include <bits/signal.h>
#include <bits/types/signal.h>

	typedef struct siginfo_t
	{
		int si_signo; /* Signal number. */
		int si_code;  /* Signal code. */

		int si_errno; /* If non-zero, an errno value associated with */
					  /* this signal, as described in <errno.h>. */

		pid_t si_pid;		   /* Sending process ID. */
		uid_t si_uid;		   /* Real user ID of sending process. */
		void *si_addr;		   /* Address that caused fault. */
		int si_status;		   /* Exit value or signal. */
		union sigval si_value; /* Signal value. */
	} siginfo_t;

	struct sigaction
	{
		void (*sa_handler)(int);						/* Pointer to a signal-catching function or one of the SIG_IGN or SIG_DFL. */
		sigset_t sa_mask;								/* Set of signals to be blocked during execution of the signal handling function. */
		int sa_flags;									/* Special flags. */
		void (*sa_sigaction)(int, siginfo_t *, void *); /* Pointer to a signal-catching function. */
	};

	typedef struct stack_t
	{
		void *ss_sp;	/* Stack base or pointer. */
		size_t ss_size; /* Stack size. */
		int ss_flags;	/* Flags. */
	} stack_t;

	typedef struct mcontext_t
	{
		__UINTPTR_TYPE__ gregs[32]; /* General-purpose registers. */
		__UINTPTR_TYPE__ sp;		/* Stack pointer. */
		__UINTPTR_TYPE__ pc;		/* Program counter. */
		__UINTPTR_TYPE__ pstate;	/* Processor state. */
	} mcontext_t;

	typedef struct ucontext_t
	{
		struct ucontext_t *uc_link; /* Pointer to the context that is resumed when this context returns. */
		sigset_t uc_sigmask;		/* The set of signals that are blocked when this context is active. */
		stack_t uc_stack;			/* The stack used by this context. */
		mcontext_t uc_mcontext;		/* A machine-specific representation of the saved context. */
	} ucontext_t;

	int kill(pid_t pid, int sig);
	int killpg(pid_t, int);
	void psiginfo(const siginfo_t *, const char *);
	void psignal(int, const char *);
	int pthread_kill(pthread_t, int);
	int pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
	int raise(int);
	int sig2str(int, char *);
	int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact);
	int sigaddset(sigset_t *set, int signo);
	int sigaltstack(const stack_t *restrict, stack_t *restrict);
	int sigdelset(sigset_t *, int);
	int sigemptyset(sigset_t *set);
	int sigfillset(sigset_t *set);
	int sigismember(const sigset_t *, int);
	void (*signal(int sig, void (*func)(int)))(int);
	int sigpending(sigset_t *);
	int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
	int sigqueue(pid_t, int, union sigval);
	int sigsuspend(const sigset_t *);
	int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict, const struct timespec *restrict);
	int sigwait(const sigset_t *restrict, int *restrict);
	int sigwaitinfo(const sigset_t *restrict, siginfo_t *restrict);
	int str2sig(const char *restrict, int *restrict);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SIGNAL_H
