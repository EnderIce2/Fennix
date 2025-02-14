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

#ifndef NETINET_IN_H
#define NETINET_IN_H

#include <inttypes.h>
#include <sys/socket.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr
{
	in_addr_t s_addr;
};

struct sockaddr_in
{
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
};

struct in6_addr
{
	uint8_t s6_addr[16];
};

struct sockaddr_in6
{
	sa_family_t sin6_family;
	in_port_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#define IN6ADDR_ANY_INIT                    \
	{                                       \
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}} \
	}

#define IN6ADDR_LOOPBACK_INIT               \
	{                                       \
		{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}} \
	}

struct ipv6_mreq
{
	struct in6_addr ipv6mr_multiaddr;
	unsigned ipv6mr_interface;
};

#define IPPROTO_IP 0
#define IPPROTO_IPV6 41
#define IPPROTO_ICMP 1
#define IPPROTO_RAW 255
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

#define INADDR_ANY ((in_addr_t)0x00000000)
#define INADDR_BROADCAST ((in_addr_t)0xffffffff)

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

#define IPV6_JOIN_GROUP 20
#define IPV6_LEAVE_GROUP 21
#define IPV6_MULTICAST_HOPS 18
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_LOOP 19
#define IPV6_UNICAST_HOPS 16
#define IPV6_V6ONLY 26

#define IN6_IS_ADDR_UNSPECIFIED(a)      \
	(((const uint32_t *)(a))[0] == 0 && \
	 ((const uint32_t *)(a))[1] == 0 && \
	 ((const uint32_t *)(a))[2] == 0 && \
	 ((const uint32_t *)(a))[3] == 0)

#define IN6_IS_ADDR_LOOPBACK(a)         \
	(((const uint32_t *)(a))[0] == 0 && \
	 ((const uint32_t *)(a))[1] == 0 && \
	 ((const uint32_t *)(a))[2] == 0 && \
	 ((const uint32_t *)(a))[3] == htonl(1))

#define IN6_IS_ADDR_MULTICAST(a) \
	(((const uint8_t *)(a))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(a)            \
	((((const uint8_t *)(a))[0] == 0xfe) && \
	 (((const uint8_t *)(a))[1] == 0x80))

#define IN6_IS_ADDR_SITELOCAL(a)            \
	((((const uint8_t *)(a))[0] == 0xfe) && \
	 (((const uint8_t *)(a))[1] == 0xc0))

#define IN6_IS_ADDR_V4MAPPED(a)           \
	((((const uint32_t *)(a))[0] == 0) && \
	 (((const uint32_t *)(a))[1] == 0) && \
	 (((const uint32_t *)(a))[2] == htonl(0xffff)))

#define IN6_IS_ADDR_V4COMPAT(a)           \
	((((const uint32_t *)(a))[0] == 0) && \
	 (((const uint32_t *)(a))[1] == 0) && \
	 (((const uint32_t *)(a))[2] == 0) && \
	 (ntohl(((const uint32_t *)(a))[3]) > 1))

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	 ((((const uint8_t *)(a))[1] & 0xf) == 1))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	 ((((const uint8_t *)(a))[1] & 0xf) == 2))

#define IN6_IS_ADDR_MC_SITELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	 ((((const uint8_t *)(a))[1] & 0xf) == 5))

#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) &&   \
	 ((((const uint8_t *)(a))[1] & 0xf) == 8))

#define IN6_IS_ADDR_MC_GLOBAL(a) \
	(IN6_IS_ADDR_MULTICAST(a) && \
	 ((((const uint8_t *)(a))[1] & 0xf) == 0xe))

#endif // NETINET_IN_H
