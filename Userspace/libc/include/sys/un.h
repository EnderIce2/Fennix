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

#ifndef _SYS_UN_H
#define _SYS_UN_H

#include <sys/socket.h>

#define SUN_PATH_SIZE 108

struct sockaddr_un
{
	sa_family_t sun_family;		  /* Address family */
	char sun_path[SUN_PATH_SIZE]; /* Socket pathname storage */
};

#endif // _SYS_UN_H
