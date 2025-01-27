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

#include <sys/wait.h>
#include <fennix/syscalls.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

export pid_t wait(int *stat_loc)
{
	return waitpid((pid_t)-1, stat_loc, 0);
}

export int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
	printf("waitid() is unimplemented\n");
	return __check_errno(-ENOSYS, -1);
}

export pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
	return __check_errno(call_waitpid(pid, stat_loc, options), -1);
}
