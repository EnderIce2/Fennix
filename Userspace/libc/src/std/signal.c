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

#include <signal.h>
#include <errno.h>
#include <stdio.h>

export int kill(pid_t pid, int sig)
{
	return syscall2(SYS_KILL, pid, sig);
}

export int killpg(pid_t, int);
export void psiginfo(const siginfo_t *, const char *);
export void psignal(int, const char *);
export int pthread_kill(pthread_t, int);
export int pthread_sigmask(int, const sigset_t *restrict, sigset_t *restrict);
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

export int sigaddset(sigset_t *, int);
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

export int sigfillset(sigset_t *);
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
export int sigprocmask(int, const sigset_t *restrict, sigset_t *restrict);
export int sigqueue(pid_t, int, union sigval);
export int sigsuspend(const sigset_t *);
export int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict, const struct timespec *restrict);
export int sigwait(const sigset_t *restrict, int *restrict);
export int sigwaitinfo(const sigset_t *restrict, siginfo_t *restrict);
export int str2sig(const char *restrict, int *restrict);
