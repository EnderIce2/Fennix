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

#include <sys/utsname.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

export int uname(struct utsname *name)
{
	if (name == NULL)
		return -1;

	if (gethostname(name->nodename, sizeof(name->nodename)) != 0)
		return -1;

	strcpy(name->sysname, "Fennix");
	strcpy(name->release, "1.0.0");
	strcpy(name->version, "Fennix Version 1.0.0");
	strcpy(name->machine, "x86_64");

	return 0;
}
