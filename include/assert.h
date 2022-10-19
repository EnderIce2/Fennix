#ifndef __FENNIX_KERNEL_ASSERT_H__
#define __FENNIX_KERNEL_ASSERT_H__

#include <types.h>

#define assert(x)     \
    do                \
    {                 \
        if (!(x))     \
            while (1) \
                ;     \
    } while (0)

#define static_assert(x) \
    switch (x)           \
    case 0:              \
    case (x):

#endif // !__FENNIX_KERNEL_ASSERT_H__
