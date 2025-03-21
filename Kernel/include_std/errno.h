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

#ifndef __FENNIX_KERNEL_STD_ERRNO_H__
#define __FENNIX_KERNEL_STD_ERRNO_H__

#include <interface/errno.h>

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

#endif // !__FENNIX_KERNEL_STD_ERRNO_H__
