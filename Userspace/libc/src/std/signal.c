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

#include <bits/libc.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

export int kill(pid_t pid, int sig)
{
	return sysdep(Kill)(pid, sig);
}

export int killpg(pid_t, int);
export void psiginfo(const siginfo_t *, const char *);
export void psignal(int, const char *);
export int pthread_kill(pthread_t, int);

export int pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict oset)
{
	return sigprocmask(how, set, oset);
}

export int raise(int);
export int sig2str(int, char *);

export int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact)
{
	if (sig == SIGKILL || sig == SIGSTOP)
	{
		errno = EINVAL;
		return -1;
	}

	if (oact != NULL)
	{
		printf("sigaction() is unimplemented\n");
		// if (syscall3(SYS_RT_SIGACTION, sig, NULL, oact, sizeof(sigset_t)) < 0)
		return -1;
	}

	if (act != NULL)
	{
		printf("sigaction() is unimplemented\n");
		// if (syscall3(SYS_RT_SIGACTION, sig, act, NULL, sizeof(sigset_t)) < 0)
		return -1;
	}

	return 0;
}

export int sigaddset(sigset_t *set, int signo)
{
#ifndef SIGNAL_MAX
#ifdef NSIG
#define SIGNAL_MAX NSIG
#else
#error "NSIG is not defined"
#endif // NSIG
#endif // SIGNAL_MAX

	if (set == NULL || signo <= 0 || signo >= SIGNAL_MAX)
	{
		errno = EINVAL;
		return -1;
	}

	*set |= (1U << (signo - 1));
	return 0;
}

export int sigaltstack(const stack_t *restrict, stack_t *restrict);
export int sigdelset(sigset_t *, int);

export int sigemptyset(sigset_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	*set = 0;
	return 0;
}

export int sigfillset(sigset_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	*set = ~((sigset_t)0);
	return 0;
}

export int sigismember(const sigset_t *, int);

export void (*signal(int sig, void (*func)(int)))(int)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(sig, &act, &oact) < 0)
		return SIG_ERR;

	return oact.sa_handler;
}

export int sigpending(sigset_t *);

export int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset)
{
	if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
	{
		errno = EINVAL;
		return -1;
	}

	if (set != NULL)
	{
		if (how == SIG_BLOCK)
			*oset |= *set;
		else if (how == SIG_UNBLOCK)
			*oset &= ~(*set);
		else if (how == SIG_SETMASK)
			*oset = *set;
	}

	if (oset != NULL)
		*oset = *set;

	return 0;
}

export int sigqueue(pid_t, int, union sigval);
export int sigsuspend(const sigset_t *);
export int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict, const struct timespec *restrict);
export int sigwait(const sigset_t *restrict, int *restrict);
export int sigwaitinfo(const sigset_t *restrict, siginfo_t *restrict);
export int str2sig(const char *restrict, int *restrict);
