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

#include <fennix/syscalls.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static_assert(__SYS_PROT_READ == PROT_READ);
static_assert(__SYS_PROT_WRITE == PROT_WRITE);
static_assert(__SYS_PROT_EXEC == PROT_EXEC);
static_assert(__SYS_PROT_NONE == PROT_NONE);

static_assert(__SYS_MAP_SHARED == MAP_SHARED);
static_assert(__SYS_MAP_PRIVATE == MAP_PRIVATE);
static_assert(__SYS_MAP_FIXED == MAP_FIXED);
static_assert(__SYS_MAP_ANONYMOUS == MAP_ANONYMOUS);
static_assert(__SYS_MAP_ANON == MAP_ANON);

static_assert(__SYS_F_OK == F_OK);
static_assert(__SYS_R_OK == R_OK);
static_assert(__SYS_W_OK == W_OK);
static_assert(__SYS_X_OK == X_OK);

static_assert(sizeof(__SYS_clockid_t) == sizeof(clockid_t));
// static_assert(sizeof(__SYS_socklen_t) == sizeof(socklen_t));

static_assert(__SYS_SEEK_SET == SEEK_SET);
static_assert(__SYS_SEEK_CUR == SEEK_CUR);
static_assert(__SYS_SEEK_END == SEEK_END);

static_assert(__SYS_SA_NOCLDSTOP == SA_NOCLDSTOP);
static_assert(__SYS_SA_NOCLDWAIT == SA_NOCLDWAIT);
static_assert(__SYS_SA_SIGINFO == SA_SIGINFO);
static_assert(__SYS_SA_ONSTACK == SA_ONSTACK);
static_assert(__SYS_SA_RESTART == SA_RESTART);
static_assert(__SYS_SA_NODEFER == SA_NODEFER);
static_assert(__SYS_SA_RESETHAND == SA_RESETHAND);

static_assert(__SYS_CLOCK_MONOTONIC == CLOCK_MONOTONIC);
static_assert(__SYS_CLOCK_PROCESS_CPUTIME_ID == CLOCK_PROCESS_CPUTIME_ID);
static_assert(__SYS_CLOCK_REALTIME == CLOCK_REALTIME);
static_assert(__SYS_CLOCK_THREAD_CPUTIME_ID == CLOCK_THREAD_CPUTIME_ID);

// static_assert( == );
