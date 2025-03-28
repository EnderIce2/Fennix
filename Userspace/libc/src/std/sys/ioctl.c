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

#include <sys/ioctl.h>
#include <bits/libc.h>
#include <stdarg.h>
#include <errno.h>

export int ioctl(int fd, unsigned long op, ...)
{
	void *arg;
	va_list args;
	va_start(args, op);
	arg = va_arg(args, void *);
	va_end(args);
	int ret = sysdep(IOControl)(fd, op, arg);
	return __check_errno(ret, -1);
}
