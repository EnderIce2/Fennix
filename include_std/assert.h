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

EXTERNC void __attribute__((noreturn)) HandleAssertionFailed(const char *File, int Line,
															 const char *Expression);

#define assert(x)                                          \
	do                                                     \
	{                                                      \
		if (__builtin_expect(!!(!(x)), 0))                 \
		{                                                  \
			error("Assertion failed! [%s]", #x);           \
			HandleAssertionFailed(__FILE__, __LINE__, #x); \
			__builtin_unreachable();                       \
		}                                                  \
	} while (0)

#define assert_allow_continue(x)                 \
	do                                           \
	{                                            \
		if (__builtin_expect(!!(!(x)), 0))       \
		{                                        \
			error("Assertion failed! [%s]", #x); \
		}                                        \
	} while (0)

#if __STDC_VERSION__ >= 201112L && !defined(__cplusplus)
#define static_assert _Static_assert
#endif

#endif // !__FENNIX_KERNEL_ASSERT_H__
