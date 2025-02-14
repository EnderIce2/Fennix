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

#ifndef ARPA_INET_H
#define ARPA_INET_H

#include <netinet/in.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

in_addr_t inet_addr(const char *);
char *inet_ntoa(struct in_addr);
const char *inet_ntop(int, const void *restrict, char *restrict, socklen_t);
int inet_pton(int, const char *restrict, void *restrict);

#endif // ARPA_INET_H
