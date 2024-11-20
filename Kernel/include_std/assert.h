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
