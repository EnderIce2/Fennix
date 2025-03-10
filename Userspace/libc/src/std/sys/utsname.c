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
#include <errno.h>
#include <fennix/syscalls.h>

export int uname(struct utsname *name)
{
	if (name == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	struct kutsname kname;
	int result = call_uname(&kname);
	if (result == 0)
	{
		strcpy(name->sysname, kname.sysname);
		strcpy(name->release, kname.release);
		strcpy(name->version, kname.version);
		strcpy(name->machine, kname.machine);
	}
	else
	{
		errno = result;
		return -1;
	}

	if (gethostname(name->nodename, sizeof(name->nodename)) != 0)
	{
		errno = EIO;
		return -1;
	}

	return 0;
}
