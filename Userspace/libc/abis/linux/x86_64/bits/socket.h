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

#ifndef _BITS_SOCKET_H
#define _BITS_SOCKET_H

#define __socklen_t_defined
typedef __UINT32_TYPE__ socklen_t;

#define __sa_family_t_defined
typedef unsigned int sa_family_t;

#define __sockaddr_defined
struct sockaddr
{
	sa_family_t sa_family;
	char sa_data[14];
};

#endif // _BITS_SOCKET_H
