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

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <signal.h>

/* waitpid() */
#define WCONTINUED 0x00000008
#define WNOHANG 0x00000001
#define WUNTRACED 0x00000002

/* waitid() */
#define WEXITED 0x00000004
#define WNOWAIT 0x00000020
#define WSTOPPED 0x00000002

#define WCOREDUMP(status) ((status) & 0x80)
#define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define WIFCONTINUED(status) ((status) == 0xFFFF)
#define WIFEXITED(status) (((status) & 0x7F) == 0)
#define WIFSIGNALED(status) (((status) & 0x7F) > 0 && (((status) & 0x7F) != 0x7F))
#define WIFSTOPPED(status) (((status) & 0xFF) == 0x7F)
#define WSTOPSIG(status) WEXITSTATUS(status)
#define WTERMSIG(status) ((status) & 0x7F)

	typedef enum
	{
		P_ALL,
		P_PGID,
		P_PID
	} idtype_t;

	typedef unsigned int id_t;
	typedef int pid_t;

	pid_t wait(int *);
	int waitid(idtype_t, id_t, siginfo_t *, int);
	pid_t waitpid(pid_t, int *, int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SYS_WAIT_H
