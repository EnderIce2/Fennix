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

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <bits/libc.h>
#include <sys/types.h>
#include <sys/select.h>

typedef long time_t;
typedef long suseconds_t;

struct timeval
{
	time_t tv_sec;		 /* Seconds */
	suseconds_t tv_usec; /* Microseconds */
};

struct itimerval
{
	struct timeval it_interval; /* Timer interval */
	struct timeval it_value;	/* Current value */
};

/* Values for the which argument of getitimer() and setitimer() */
#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

int getitimer(int which, struct itimerval *value);
int gettimeofday(struct timeval *restrict tp, void *restrict tzp);
// int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout);
int setitimer(int which, const struct itimerval *restrict new_value, struct itimerval *restrict old_value);
int utimes(const char *filename, const struct timeval times[2]);

#endif // _SYS_TIME_H
