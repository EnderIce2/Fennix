#include <time.hpp>
#include <debug.h>
#include <io.h>

namespace Time
{
    Clock ReadClock()
    {
        Clock tm;
#if defined(__amd64__) || defined(__i386__)
        uint32_t t = 0;
        outb(0x70, 0x00);
        t = inb(0x71);
        tm.Second = ((t & 0x0F) + ((t >> 4) * 10));
        outb(0x70, 0x02);
        t = inb(0x71);
        tm.Minute = ((t & 0x0F) + ((t >> 4) * 10));
        outb(0x70, 0x04);
        t = inb(0x71);
        tm.Hour = ((t & 0x0F) + ((t >> 4) * 10));
        outb(0x70, 0x07);
        t = inb(0x71);
        tm.Day = ((t & 0x0F) + ((t >> 4) * 10));
        outb(0x70, 0x08);
        t = inb(0x71);
        tm.Month = ((t & 0x0F) + ((t >> 4) * 10));
        outb(0x70, 0x09);
        t = inb(0x71);
        tm.Year = ((t & 0x0F) + ((t >> 4) * 10));
        tm.Counter = 0;
#elif defined(__aarch64__)
        tm.Year = 0;
        tm.Month = 0;
        tm.Day = 0;
        tm.Hour = 0;
        tm.Minute = 0;
        tm.Second = 0;
        tm.Counter = 0;
#endif
        return tm;
    }

    Clock ConvertFromUnix(int Timestamp)
    {

        Clock result;
        if (Timestamp == 0)
            return result;

        int SecondsSinceYearStart = Timestamp % (60 * 60 * 24 * 365);

        result.Year = Timestamp / (60 * 60 * 24 * 365);
        result.Month = SecondsSinceYearStart / (60 * 60 * 24 * 30);
        result.Day = SecondsSinceYearStart / (60 * 60 * 24);
        result.Hour = SecondsSinceYearStart / (60 * 60);
        result.Minute = SecondsSinceYearStart / 60;
        result.Second = SecondsSinceYearStart;

#ifdef DEBUG
        int DaysInYear;
        if (result.Year % 4 != 0)
            DaysInYear = 365;
        else if (result.Year % 100 != 0)
            DaysInYear = 366;
        else if (result.Year % 400 == 0)
            DaysInYear = 366;
        else
            DaysInYear = 365;
        debug("Days in year: %d", DaysInYear);
#endif

        return result;
    }
}
