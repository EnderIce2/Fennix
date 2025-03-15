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

#include <bits/libc.h>
#include <arpa/inet.h>

#include <stdint.h>

export uint32_t htonl(uint32_t hostlong)
{
	return ((hostlong & 0x000000FF) << 24) |
		   ((hostlong & 0x0000FF00) << 8) |
		   ((hostlong & 0x00FF0000) >> 8) |
		   ((hostlong & 0xFF000000) >> 24);
}

export uint16_t htons(uint16_t hostshort)
{
	return ((hostshort & 0x00FF) << 8) |
		   ((hostshort & 0xFF00) >> 8);
}

export uint32_t ntohl(uint32_t netlong)
{
	return htonl(netlong);
}

export uint16_t ntohs(uint16_t netshort)
{
	return htons(netshort);
}

export in_addr_t inet_addr(const char *);
export char *inet_ntoa(struct in_addr);
export const char *inet_ntop(int, const void *restrict, char *restrict, socklen_t);
export int inet_pton(int, const char *restrict, void *restrict);
