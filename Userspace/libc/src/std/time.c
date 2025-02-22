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

static int __is_leap(int year)
{
	return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

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
	time_t t = *timer;
	long days, rem;
	rem = t % 86400;
	days = t / 86400;
	if (rem < 0)
	{
		rem += 86400;
		days--;
	}
	result->tm_hour = rem / 3600;
	rem %= 3600;
	result->tm_min = rem / 60;
	result->tm_sec = rem % 60;
	int wday = (4 + days) % 7;
	if (wday < 0)
		wday += 7;
	result->tm_wday = wday;
	int year = 1970;
	while (days < 0 || days >= (__is_leap(year) ? 366 : 365))
	{
		int yd = __is_leap(year) ? 366 : 365;
		if (days >= yd)
		{
			days -= yd;
			year++;
		}
		else
		{
			year--;
			days += __is_leap(year) ? 366 : 365;
		}
	}
	result->tm_year = year - 1900;
	result->tm_yday = days;
	static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int mon;
	for (mon = 0; mon < 12; mon++)
	{
		int dim = mdays[mon];
		if (mon == 1 && __is_leap(year))
			dim++;
		if (days >= dim)
			days -= dim;
		else
			break;
	}
	result->tm_mon = mon;
	result->tm_mday = days + 1;
	result->tm_isdst = 0;
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
	time_t t = *timer - timezone;
	return gmtime_r(&t, result);
}

export time_t mktime(struct tm *timeptr)
{
	int sec = timeptr->tm_sec;
	int min = timeptr->tm_min;
	int hour = timeptr->tm_hour;
	int mon = timeptr->tm_mon;
	int day = timeptr->tm_mday;
	int year = timeptr->tm_year + 1900;
	if (sec >= 60 || sec < 0)
	{
		min += sec / 60;
		sec %= 60;
		if (sec < 0)
		{
			sec += 60;
			min--;
		}
	}
	if (min >= 60 || min < 0)
	{
		hour += min / 60;
		min %= 60;
		if (min < 0)
		{
			min += 60;
			hour--;
		}
	}
	if (hour >= 24 || hour < 0)
	{
		day += hour / 24;
		hour %= 24;
		if (hour < 0)
		{
			hour += 24;
			day--;
		}
	}
	if (mon >= 12 || mon < 0)
	{
		year += mon / 12;
		mon %= 12;
		if (mon < 0)
		{
			mon += 12;
			year--;
		}
	}
	long days = 0;
	if (year >= 1970)
	{
		for (int y = 1970; y < year; y++)
			days += __is_leap(y) ? 366 : 365;
	}
	else
	{
		for (int y = year; y < 1970; y++)
			days -= __is_leap(y) ? 366 : 365;
	}
	static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	for (int i = 0; i < mon; i++)
	{
		days += mdays[i];
		if (i == 1 && __is_leap(year))
			days++;
	}
	days += (day - 1);
	time_t t = days * 86400 + hour * 3600 + min * 60 + sec;
	t = t + timezone - (timeptr->tm_isdst > 0 ? 3600 : 0);
	struct tm temp;
	localtime_r(&t, &temp);
	*timeptr = temp;
	return t;
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
	if (!tz || tz[0] == '\0')
	{
		tzname[0] = "UTC";
		tzname[1] = "UTC";
		timezone = 0;
		daylight = 0;
		return;
	}

	char std[16];
	int i = 0;

	while (tz[i] && ((tz[i] >= 'A' && tz[i] <= 'Z') || (tz[i] >= 'a' && tz[i] <= 'z')))
	{
		std[i] = tz[i];
		i++;
	}

	std[i] = '\0';
	tzname[0] = strdup(std);
	int sign = 1;

	if (tz[i] == '-')
	{
		sign = -1;
		i++;
	}
	else if (tz[i] == '+')
	{
		sign = 1;
		i++;
	}

	int offset = 0;
	while (tz[i] && tz[i] >= '0' && tz[i] <= '9')
	{
		offset = offset * 10 + (tz[i] - '0');
		i++;
	}

	timezone = offset * 3600 * sign;
	char dst[16];
	int j = 0;

	if (tz[i] != '\0')
	{
		while (tz[i] && ((tz[i] >= 'A' && tz[i] <= 'Z') || (tz[i] >= 'a' && tz[i] <= 'z')))
			dst[j++] = tz[i++];

		dst[j] = '\0';
		tzname[1] = strdup(dst);
		daylight = 1;
	}
	else
	{
		tzname[1] = tzname[0];
		daylight = 0;
	}
}
