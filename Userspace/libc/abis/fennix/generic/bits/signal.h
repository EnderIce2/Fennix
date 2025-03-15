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

#ifndef __BITS_SIGNAL_H
#define __BITS_SIGNAL_H

#include <bits/syscalls.h>

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
// #define SIG_IGN __SYS_SIG_IGN
#define SIG_CORE __SYS_SIG_CORE
#define SIG_STOP __SYS_SIG_STOP
#define SIG_CONT __SYS_SIG_CONT

#define SIG_BLOCK __SYS_SIG_BLOCK
#define SIG_UNBLOCK __SYS_SIG_UNBLOCK
#define SIG_SETMASK __SYS_SIG_SETMASK

#define SA_NOCLDSTOP __SYS_SA_NOCLDSTOP
#define SA_ONSTACK __SYS_SA_ONSTACK
#define SA_RESETHAND __SYS_SA_RESETHAND
#define SA_RESTART __SYS_SA_RESTART
#define SA_SIGINFO __SYS_SA_SIGINFO
#define SA_NOCLDWAIT __SYS_SA_NOCLDWAIT
#define SA_NODEFER __SYS_SA_NODEFER

#define SS_ONSTACK
#define SS_DISABLE

#define MINSIGSTKSZ
#define SIGSTKSZ

#define SIG_ERR ((void (*)(int))__SYS_SIG_ERR)
#define SIG_DFL ((void (*)(int))__SYS_SIG_DFL)
#define SIG_IGN ((void (*)(int))__SYS_SIG_IGN)

#define SIGEV_NONE
#define SIGEV_SIGNAL
#define SIGEV_THREAD

typedef unsigned long sigset_t;

#endif // __BITS_SIGNAL_H
