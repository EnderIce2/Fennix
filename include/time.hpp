#ifndef __FENNIX_KERNEL_TIME_H__
#define __FENNIX_KERNEL_TIME_H__

#include <types.h>

struct Time
{
    uint64_t Year, Month, Day, Hour, Minute, Second;
    uint64_t Counter;
};

Time ReadClock();

#endif // !__FENNIX_KERNEL_TIME_H__
