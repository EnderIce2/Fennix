/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

/* This function includes all the standard headers and defines some useful macros.
 * Note: This std implementation is not complete.
 */
#ifndef __FENNIX_KERNEL_STD_H__
#define __FENNIX_KERNEL_STD_H__

#include <types.h>
#include <stdexcept>
#include <atomic>
#include <vector>
#include <string>
#include <list>
#include <std/functional.hpp>
#include <std/smart_ptr.hpp>
#include <std/unordered_map.hpp>
#include <std/utility.hpp>

/**
 * @brief // stub namespace for std::align_val_t and new operator
 * @note // Found on https://gcc.gnu.org/legacy-ml/gcc-patches/2016-09/msg00628.html for "_ZnwmSt11align_val_t" compiler error
 */
namespace std
{
    typedef __SIZE_TYPE__ size_t;
    static const size_t npos = -1;

    enum class align_val_t : std::size_t
    {
    };

    template <typename InputIt, typename OutputIt, typename UnaryOperation>
    OutputIt transform(InputIt first, InputIt last, OutputIt result, UnaryOperation op)
    {
        while (first != last)
        {
            *result = op(*first);
            ++first;
            ++result;
        }
        return result;
    };

    inline __always_inline int tolower(int c)
    {
        if (c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        else
            return c;
    }

    inline __always_inline int toupper(int c)
    {
        if (c >= 'a' && c <= 'z')
            return c - ('a' - 'A');
        else
            return c;
    }
}

#endif // !__FENNIX_KERNEL_STD_H__
