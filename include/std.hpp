#ifndef __FENNIX_KERNEL_STD_H__
#define __FENNIX_KERNEL_STD_H__

#include <types.h>

/**
 * @brief // stub namespace for std::align_val_t and new operator
 * @note // Found on https://gcc.gnu.org/legacy-ml/gcc-patches/2016-09/msg00628.html for "_ZnwmSt11align_val_t" compiler error
 */
namespace std
{
    typedef __SIZE_TYPE__ size_t;
    enum class align_val_t : std::size_t
    {
    };
}

#endif // !__FENNIX_KERNEL_STD_H__
