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

#ifndef _STDDEF_H
#define _STDDEF_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifndef NULL
#define NULL ((void *)0)
#endif

	typedef struct
	{
		long long __a;
		long double __b;
	} max_align_t;

	typedef __PTRDIFF_TYPE__ ptrdiff_t;
	typedef __WCHAR_TYPE__ wchar_t;
	typedef __SIZE_TYPE__ size_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_STDDEF_H
