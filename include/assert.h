#ifndef __FENNIX_KERNEL_ASSERT_H__
#define __FENNIX_KERNEL_ASSERT_H__

#include <debug.h>

#define assert(x)                                                                                                    \
    do                                                                                                               \
    {                                                                                                                \
        if (!(x))                                                                                                    \
        {                                                                                                            \
            void *CallerAddress = __builtin_extract_return_addr(__builtin_return_address(0));                        \
            error("Assertion failed! [%s] [%#lx => %s:%s:%d]", #x, CallerAddress, __FILE__, __FUNCTION__, __LINE__); \
            while (1)                                                                                                \
                ;                                                                                                    \
        }                                                                                                            \
    } while (0)

#define assert_allow_continue(x)                                                                                     \
    do                                                                                                               \
    {                                                                                                                \
        if (!(x))                                                                                                    \
        {                                                                                                            \
            void *CallerAddress = __builtin_extract_return_addr(__builtin_return_address(0));                        \
            error("Assertion failed! [%s] [%#lx => %s:%s:%d]", #x, CallerAddress, __FILE__, __FUNCTION__, __LINE__); \
        }                                                                                                            \
    } while (0)

#define static_assert(x) \
    switch (x)           \
    case 0:              \
    case (x):

#endif // !__FENNIX_KERNEL_ASSERT_H__
