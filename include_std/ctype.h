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

#ifndef __FENNIX_KERNEL_C_TYPE_H__
#define __FENNIX_KERNEL_C_TYPE_H__

#include <types.h>

START_EXTERNC

int isalnum(int);
int isalpha(int);
int isascii(int);

int isblank(int);
int iscntrl(int);
int isdigit(int);
int isgraph(int);
int islower(int);
int isprint(int);
int ispunct(int);
int isspace(int);
int isupper(int);
int isxdigit(int);

int toascii(int);
int tolower(int);
int toupper(int);

#ifndef __cplusplus /* This conflicts with std */
#define _toupper(c) ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define _tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#endif

END_EXTERNC

#endif // !__FENNIX_KERNEL_C_TYPE_H__
