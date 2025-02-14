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

#include <sys/socket.h>
#include <errno.h>
#include <fennix/syscalls.h>

export int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len)
{
	return __check_errno(call_accept(socket, address, address_len), -1);
}

export int accept4(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len, int flag);

export int bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
	return __check_errno(call_bind(socket, address, address_len), -1);
}

export int connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
	return __check_errno(call_connect(socket, address, address_len), -1);
}

export int getpeername(int, struct sockaddr *restrict, socklen_t *restrict);
export int getsockname(int, struct sockaddr *restrict, socklen_t *restrict);
export int getsockopt(int, int, int, void *restrict, socklen_t *restrict);

export int listen(int socket, int backlog)
{
	return __check_errno(call_listen(socket, backlog), -1);
}

export ssize_t recv(int, void *, size_t, int);
export ssize_t recvfrom(int, void *restrict, size_t, int, struct sockaddr *restrict, socklen_t *restrict);
export ssize_t recvmsg(int, struct msghdr *, int);
export ssize_t send(int, const void *, size_t, int);
export ssize_t sendmsg(int, const struct msghdr *, int);
export ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
export int setsockopt(int, int, int, const void *, socklen_t);
export int shutdown(int, int);
export int sockatmark(int);

export int socket(int domain, int type, int protocol)
{
	return __check_errno(call_socket(domain, type, protocol), -1);
}

export int socketpair(int, int, int, int[2]);
