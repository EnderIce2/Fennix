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

#ifndef _ERRNO_H
#define _ERRNO_H

#include <bits/errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

	int *__errno_location(void) __attribute__((const));
	char *strerror(int errnum);

#ifdef __cplusplus
}
#endif

#define errno (*__errno_location())

#endif // _ERRNO_H
