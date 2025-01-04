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
export int sigaction(int, const struct sigaction *restrict, struct sigaction *restrict);
export int sigaddset(sigset_t *, int);
export int sigaltstack(const stack_t *restrict, stack_t *restrict);
export int sigdelset(sigset_t *, int);
export int sigemptyset(sigset_t *);
export int sigfillset(sigset_t *);
export int sigismember(const sigset_t *, int);
export void (*signal(int, void (*)(int)))(int);
export int sigpending(sigset_t *);
export int sigprocmask(int, const sigset_t *restrict, sigset_t *restrict);
export int sigqueue(pid_t, int, union sigval);
export int sigsuspend(const sigset_t *);
export int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict, const struct timespec *restrict);
export int sigwait(const sigset_t *restrict, int *restrict);
export int sigwaitinfo(const sigset_t *restrict, siginfo_t *restrict);
export int str2sig(const char *restrict, int *restrict);
