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

#ifndef _TIME_H
#define _TIME_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <bits/types/timespec.h>
#include <sys/types.h>
#include <bits/types/signal.h>
#include <locale.h>

	typedef struct tm
	{
		int tm_sec;			 /* Seconds [0,60]. */
		int tm_min;			 /* Minutes [0,59]. */
		int tm_hour;		 /* Hour [0,23]. */
		int tm_mday;		 /* Day of month [1,31]. */
		int tm_mon;			 /* Month of year [0,11]. */
		int tm_year;		 /* Years since 1900. */
		int tm_wday;		 /* Day of week [0,6] (Sunday =0). */
		int tm_yday;		 /* Day of year [0,365]. */
		int tm_isdst;		 /* Daylight Saving flag. */
		long tm_gmtoff;		 /* Seconds east of UTC. */
		const char *tm_zone; /* Timezone abbreviation. */
	} tm;

	typedef struct itimerspec
	{
		struct timespec it_interval; /* Timer period. */
		struct timespec it_value;	 /* Timer expiration. */
	} itimerspec;

#define CLOCKS_PER_SEC
#define TIME_UTC
#define CLOCK_MONOTONIC __SYS_CLOCK_MONOTONIC
#define CLOCK_PROCESS_CPUTIME_ID __SYS_CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_REALTIME __SYS_CLOCK_REALTIME
#define CLOCK_THREAD_CPUTIME_ID __SYS_CLOCK_THREAD_CPUTIME_ID
#define TIMER_ABSTIME

	extern int daylight;
	extern long timezone;
	extern char *tzname[2];

	char *asctime(const struct tm *);
	clock_t clock(void);
	int clock_getcpuclockid(pid_t, clockid_t *);
	int clock_getres(clockid_t, struct timespec *);
	int clock_gettime(clockid_t, struct timespec *);
	int clock_nanosleep(clockid_t, int, const struct timespec *, struct timespec *);
	int clock_settime(clockid_t, const struct timespec *);
	char *ctime(const time_t *);
	double difftime(time_t, time_t);
	struct tm *getdate(const char *);

	struct tm *gmtime(const time_t *timer);
	struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result);
	struct tm *localtime(const time_t *timer);
	struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result);
	time_t mktime(struct tm *timeptr);
	int nanosleep(const struct timespec *, struct timespec *);
	size_t strftime(char *restrict, size_t, const char *restrict, const struct tm *restrict);
	size_t strftime_l(char *restrict, size_t, const char *restrict, const struct tm *restrict, locale_t);
	char *strptime(const char *restrict, const char *restrict, struct tm *restrict);
	time_t time(time_t *);
	int timer_create(clockid_t, struct sigevent *restrict, timer_t *restrict);
	int timer_delete(timer_t);
	int timer_getoverrun(timer_t);
	int timer_gettime(timer_t, struct itimerspec *);
	int timer_settime(timer_t, int, const struct itimerspec *restrict, struct itimerspec *restrict);
	int timespec_get(struct timespec *, int);
	void tzset(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_TIME_H
