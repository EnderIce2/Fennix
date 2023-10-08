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

#include <types.h>

enum Signals
{
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
	 * Child process terminated, stopped,
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
};

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

#endif // !__FENNIX_KERNEL_SIGNAL_H__
