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

#ifndef __FENNIX_KERNEL_LINUX_SIGNALS_H__
#define __FENNIX_KERNEL_LINUX_SIGNALS_H__

#define linux_NSIG 64

#define linux_SIGHUP 1
#define linux_SIGINT 2
#define linux_SIGQUIT 3
#define linux_SIGILL 4
#define linux_SIGTRAP 5
#define linux_SIGABRT 6
#define linux_SIGBUS 7
#define linux_SIGFPE 8
#define linux_SIGKILL 9
#define linux_SIGUSR1 10
#define linux_SIGSEGV 11
#define linux_SIGUSR2 12
#define linux_SIGPIPE 13
#define linux_SIGALRM 14
#define linux_SIGTERM 15
#define linux_SIGSTKFLT 16
#define linux_SIGCHLD 17
#define linux_SIGCONT 18
#define linux_SIGSTOP 19
#define linux_SIGTSTP 20
#define linux_SIGTTIN 21
#define linux_SIGTTOU 22
#define linux_SIGURG 23
#define linux_SIGXCPU 24
#define linux_SIGXFSZ 25
#define linux_SIGVTALRM 26
#define linux_SIGPROF 27
#define linux_SIGWINCH 28
#define linux_SIGPOLL 29
#define linux_SIGPWR 30
#define linux_SIGSYS 31
#define linux_SIGUNUSED linux_SIGSYS

#define linux_SIGRTMIN 32
#define linux_SIGRTMAX linux_NSIG

struct k_sigaction
{
	void (*handler)(int);
	unsigned long flags;
	void (*restorer)(void);
	unsigned mask[2];
};

#endif // !__FENNIX_KERNEL_LINUX_SIGNALS_H__
