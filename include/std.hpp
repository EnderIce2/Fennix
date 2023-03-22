#ifndef __FENNIX_KERNEL_STD_H__
#define __FENNIX_KERNEL_STD_H__

#include <types.h>
#include <vector.hpp>
#include <string.hpp>

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

    template <class T>
    class vector : public Vector<T>
    {
    public:
        using Vector<T>::Vector;
        using Vector<T>::operator=;
        using Vector<T>::operator[];
        using typename Vector<T>::iterator;
    };

    class string : public String
    {
    public:
        using String::String;
        using String::operator=;
        using String::operator+;
        using String::operator<<;
        using String::operator[];
        using String::operator==;
        using String::operator!=;
        using typename String::iterator;
    };
}

#endif // !__FENNIX_KERNEL_STD_H__
