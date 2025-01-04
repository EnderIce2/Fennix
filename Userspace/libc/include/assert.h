/*
	This file is part of Fennix C Library.

	Fennix C Library is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix C Library. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	void __assert_fail(const char *file, int line, const char *func) __attribute__((noreturn));

#ifdef NDEBUG
#define assert(e) ((void)0)
#else // NDEBUG
#define assert(e)                                        \
	do                                                   \
	{                                                    \
		if (__builtin_expect(!!(!(e)), 0))               \
			__assert_fail(__FILE__, __LINE__, __func__); \
	} while (0)
#endif // NDEBUG

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_ASSERT_H
