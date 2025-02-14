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

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>

typedef uint32_t socklen_t;
typedef unsigned int sa_family_t;

struct sockaddr
{
	sa_family_t sa_family;
	char sa_data[14];
};

#define _SS_MAXSIZE 128
#define _SS_ALIGNSIZE (sizeof(int64_t))
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof(sa_family_t))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof(sa_family_t) + _SS_PAD1SIZE + _SS_ALIGNSIZE))

struct sockaddr_storage
{
	sa_family_t ss_family;
	char _ss_pad1[_SS_PAD1SIZE];
	int64_t _ss_align;
	char _ss_pad2[_SS_PAD2SIZE];
};

struct msghdr
{
	void *msg_name;
	socklen_t msg_namelen;
	struct iovec *msg_iov;
	int msg_iovlen;
	void *msg_control;
	socklen_t msg_controllen;
	int msg_flags;
};

struct cmsghdr
{
	socklen_t cmsg_len;
	int cmsg_level;
	int cmsg_type;
};

struct linger
{
	int l_onoff;
	int l_linger;
};

#define SOCK_DGRAM 1
#define SOCK_RAW 2
#define SOCK_SEQPACKET 3
#define SOCK_STREAM 4
#define SOCK_NONBLOCK 0x800
#define SOCK_CLOEXEC 0x80000
#define SOCK_CLOFORK 0x100000

#define SOL_SOCKET 1

#define SO_ACCEPTCONN 1
#define SO_BROADCAST 2
#define SO_DEBUG 3
#define SO_DOMAIN 4
#define SO_DONTROUTE 5
#define SO_ERROR 6
#define SO_KEEPALIVE 7
#define SO_LINGER 8
#define SO_OOBINLINE 9
#define SO_PROTOCOL 10
#define SO_RCVBUF 11
#define SO_RCVLOWAT 12
#define SO_RCVTIMEO 13
#define SO_REUSEADDR 14
#define SO_SNDBUF 15
#define SO_SNDLOWAT 16
#define SO_SNDTIMEO 17
#define SO_TYPE 18

#define SOMAXCONN 128

#define MSG_CMSG_CLOEXEC 0x01
#define MSG_CMSG_CLOFORK 0x02
#define MSG_CTRUNC 0x08
#define MSG_DONTROUTE 0x04
#define MSG_EOR 0x80
#define MSG_OOB 0x01
#define MSG_NOSIGNAL 0x4000
#define MSG_PEEK 0x02
#define MSG_TRUNC 0x20
#define MSG_WAITALL 0x100

#define AF_INET 2
#define AF_INET6 10
#define AF_UNIX 1
#define AF_UNSPEC 0

#define SHUT_RD 0
#define SHUT_RDWR 2
#define SHUT_WR 1

#define SCM_RIGHTS 0x01

#define CMSG_DATA(cmsg) ((unsigned char *)((struct cmsghdr *)(cmsg) + 1))
#define CMSG_NXTHDR(mhdr, cmsg)                                                \
	(((char *)(cmsg) + CMSG_ALIGN((cmsg)->cmsg_len) + sizeof(struct cmsghdr) > \
	  (char *)((mhdr)->msg_control) + (mhdr)->msg_controllen)                  \
		 ? (struct cmsghdr *)NULL                                              \
		 : (struct cmsghdr *)((char *)(cmsg) + CMSG_ALIGN((cmsg)->cmsg_len)))
#define CMSG_FIRSTHDR(mhdr) \
	((mhdr)->msg_controllen >= sizeof(struct cmsghdr) ? (struct cmsghdr *)((mhdr)->msg_control) : (struct cmsghdr *)NULL)
#define CMSG_ALIGN(len) (((len) + sizeof(long) - 1) & ~(sizeof(long) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN(len) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#define CMSG_LEN(len) (CMSG_ALIGN(len) + sizeof(struct cmsghdr))

int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
int accept4(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len, int flag);
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
int getpeername(int, struct sockaddr *restrict, socklen_t *restrict);
int getsockname(int, struct sockaddr *restrict, socklen_t *restrict);
int getsockopt(int, int, int, void *restrict, socklen_t *restrict);
int listen(int socket, int backlog);
ssize_t recv(int, void *, size_t, int);
ssize_t recvfrom(int, void *restrict, size_t, int, struct sockaddr *restrict, socklen_t *restrict);
ssize_t recvmsg(int, struct msghdr *, int);
ssize_t send(int, const void *, size_t, int);
ssize_t sendmsg(int, const struct msghdr *, int);
ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
int setsockopt(int, int, int, const void *, socklen_t);
int shutdown(int, int);
int sockatmark(int);
int socket(int domain, int type, int protocol);
int socketpair(int, int, int, int[2]);

#endif // _SYS_SOCKET_H
