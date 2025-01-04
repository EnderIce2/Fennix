/*
	This file is part of Fennix Userspace.

	Fennix Userspace is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Userspace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Userspace. If not, see <https://www.gnu.org/licenses/>.
*/

#include <fennix/syscalls.h>

int __init_pthread(void);
void __init_stdio(void);

__attribute__((visibility("default"))) void __libc_init(void)
{
	__init_pthread();
	__init_stdio();
}

__attribute__((visibility("default"))) void _exit(int Code)
{
	call_exit(Code);
	while (1)
		;
}
