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

#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

export int daylight;
export long timezone;
export char *tzname[2];

export char *asctime(const struct tm *);
export clock_t clock(void);
export int clock_getcpuclockid(pid_t, clockid_t *);
export int clock_getres(clockid_t, struct timespec *);
export int clock_gettime(clockid_t, struct timespec *);
export int clock_nanosleep(clockid_t, int, const struct timespec *, struct timespec *);
export int clock_settime(clockid_t, const struct timespec *);
export char *ctime(const time_t *);
export double difftime(time_t, time_t);
export struct tm *getdate(const char *);

export struct tm *gmtime(const time_t *timer)
{
	static struct tm result;
	return gmtime_r(timer, &result);
}

export struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result)
{
	if (timer == NULL || result == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	time_t t = *timer;
	struct tm *res = localtime(&t);
	if (res == NULL)
		return NULL;

	*result = *res;
	result->tm_gmtoff = 0;
	result->tm_zone = "UTC";
	return result;
}

export struct tm *localtime(const time_t *timer)
{
	static struct tm result;
	return localtime_r(timer, &result);
}

export struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result)
{
	if (timer == NULL || result == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	tzset();
	time_t t = *timer;
	struct tm *gmt = gmtime(&t);
	if (gmt == NULL)
		return NULL;

	*result = *gmt;
	result->tm_isdst = -1;
	time_t local_time = mktime(result);
	if (local_time == (time_t)-1)
		return NULL;

	*result = *localtime(&local_time);
	return result;
}

export time_t mktime(struct tm *timeptr)
{
	time_t result;
	struct tm temp;
	tzset();

	temp = *timeptr;
	temp.tm_sec += temp.tm_min * 60 + temp.tm_hour * 3600 + temp.tm_mday * 86400;
	temp.tm_min = 0;
	temp.tm_hour = 0;
	temp.tm_mday = 1;
	temp.tm_mon = 0;
	temp.tm_year = 70;
	temp.tm_isdst = -1;

	result = mktime(&temp);
	if (result == (time_t)-1)
		return (time_t)-1;

	result += timeptr->tm_sec + timeptr->tm_min * 60 + timeptr->tm_hour * 3600;
	result += (timeptr->tm_mday - 1) * 86400;
	result += (timeptr->tm_mon) * 2629743;
	result += (timeptr->tm_year - 70) * 31556926;

	struct tm *local_time = localtime(&result);
	if (local_time == NULL)
		return (time_t)-1;
	*timeptr = *local_time;

	return result;
}

export int nanosleep(const struct timespec *, struct timespec *);
export size_t strftime(char *restrict, size_t, const char *restrict, const struct tm *restrict);
export size_t strftime_l(char *restrict, size_t, const char *restrict, const struct tm *restrict, locale_t);
export char *strptime(const char *restrict, const char *restrict, struct tm *restrict);
export time_t time(time_t *);
export int timer_create(clockid_t, struct sigevent *restrict, timer_t *restrict);
export int timer_delete(timer_t);
export int timer_getoverrun(timer_t);
export int timer_gettime(timer_t, struct itimerspec *);
export int timer_settime(timer_t, int, const struct itimerspec *restrict, struct itimerspec *restrict);
export int timespec_get(struct timespec *, int);

export void tzset(void)
{
	char *tz = getenv("TZ");
	if (tz == NULL)
	{
		tzname[0] = "UTC";
		tzname[1] = "UTC";
		daylight = 0;
		timezone = 0;
	}
	else
	{
		if (strcmp(tz, "EST5EDT") == 0)
		{
			tzname[0] = "EST";
			tzname[1] = "EDT";
			daylight = 1;
			timezone = 5 * 3600;
		}
		else if (strcmp(tz, "GMT0") == 0)
		{
			tzname[0] = "GMT";
			tzname[1] = "GMT";
			daylight = 0;
			timezone = 0;
		}
		else if (strcmp(tz, "JST-9") == 0)
		{
			tzname[0] = "JST";
			tzname[1] = "JST";
			daylight = 0;
			timezone = -9 * 3600;
		}
		else if (strcmp(tz, "MET-1MEST") == 0)
		{
			tzname[0] = "MET";
			tzname[1] = "MEST";
			daylight = 1;
			timezone = -1 * 3600;
		}
		else if (strcmp(tz, "MST7MDT") == 0)
		{
			tzname[0] = "MST";
			tzname[1] = "MDT";
			daylight = 1;
			timezone = 7 * 3600;
		}
		else if (strcmp(tz, "PST8PDT") == 0)
		{
			tzname[0] = "PST";
			tzname[1] = "PDT";
			daylight = 1;
			timezone = 8 * 3600;
		}
		else
		{
			tzname[0] = "UTC";
			tzname[1] = "UTC";
			daylight = 0;
			timezone = 0;
		}
	}
}
