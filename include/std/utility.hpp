#ifndef __FENNIX_KERNEL_STD_UTILITY_H__
#define __FENNIX_KERNEL_STD_UTILITY_H__

#include <types.h>

namespace std
{
    template <typename T1, typename T2>
    struct pair
    {
        typedef T1 first_type;
        typedef T2 second_type;

        T1 first;
        T2 second;

        pair() : first(T1()), second(T2()) {}
        pair(const T1 &x, const T2 &y) : first(x), second(y) {}
    };
}

#endif // !__FENNIX_KERNEL_STD_UTILITY_H__
