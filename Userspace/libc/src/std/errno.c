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

#include <sys/types.h>
#include <pthread.h>
#include <errno.h>

__iptr __check_errno(__iptr status, __iptr err)
{
	if ((int)status >= EOK)
		return status;
	pthread_self()->CurrentError = (int)status;
	return err;
}

export int *__errno_location(void)
{
	return &pthread_self()->CurrentError;
}

export char *strerror(int errnum)
{
	if (errnum < 0)
		errnum = -errnum;

	if (errnum > __ERRNO_MAX)
		return (char *)"Not a valid error number";

	switch (errnum)
	{
	case EOK:
		return (char *)"No error";
	case E2BIG:
		return (char *)"Argument list too long";
	case EACCES:
		return (char *)"Permission denied";
	case EADDRINUSE:
		return (char *)"Address in use";
	case EADDRNOTAVAIL:
		return (char *)"Address not available";
	case EAFNOSUPPORT:
		return (char *)"Address family not supported";
	case EAGAIN:
		return (char *)"Resource unavailable, try again";
	case EALREADY:
		return (char *)"Connection already in progress";
	case EBADF:
		return (char *)"Bad file descriptor";
	case EBADMSG:
		return (char *)"Bad message";
	case EBUSY:
		return (char *)"Device or resource busy";
	case ECANCELED:
		return (char *)"Operation canceled";
	case ECHILD:
		return (char *)"No child processes";
	case ECONNABORTED:
		return (char *)"Connection aborted";
	case ECONNREFUSED:
		return (char *)"Connection refused";
	case ECONNRESET:
		return (char *)"Connection reset";
	case EDEADLK:
		return (char *)"Resource deadlock would occur";
	case EDESTADDRREQ:
		return (char *)"Destination address required";
	case EDOM:
		return (char *)"Mathematics argument out of domain of function";
	case EDQUOT:
		return (char *)"Reserved";
	case EEXIST:
		return (char *)"File exists";
	case EFAULT:
		return (char *)"Bad address";
	case EFBIG:
		return (char *)"File too large";
	case EHOSTUNREACH:
		return (char *)"Host is unreachable";
	case EIDRM:
		return (char *)"Identifier removed";
	case EILSEQ:
		return (char *)"Illegal byte sequence";
	case EINPROGRESS:
		return (char *)"Operation in progress";
	case EINTR:
		return (char *)"Interrupted function";
	case EINVAL:
		return (char *)"Invalid argument";
	case EIO:
		return (char *)"I/O error";
	case EISCONN:
		return (char *)"Socket is connected";
	case EISDIR:
		return (char *)"Is a directory";
	case ELOOP:
		return (char *)"Too many levels of symbolic links";
	case EMFILE:
		return (char *)"File descriptor value too large";
	case EMLINK:
		return (char *)"Too many links";
	case EMSGSIZE:
		return (char *)"Message too large";
	case EMULTIHOP:
		return (char *)"Reserved";
	case ENAMETOOLONG:
		return (char *)"Filename too long";
	case ENETDOWN:
		return (char *)"Network is down";
	case ENETRESET:
		return (char *)"Connection aborted by network";
	case ENETUNREACH:
		return (char *)"Network unreachable";
	case ENFILE:
		return (char *)"Too many files open in system";
	case ENOBUFS:
		return (char *)"No buffer space available";
	case ENODATA:
		return (char *)"No message available on the STREAM head read queue";
	case ENODEV:
		return (char *)"No such device";
	case ENOENT:
		return (char *)"No such file or directory";
	case ENOEXEC:
		return (char *)"Executable file format error";
	case ENOLCK:
		return (char *)"No locks available";
	case ENOLINK:
		return (char *)"Reserved";
	case ENOMEM:
		return (char *)"Not enough space";
	case ENOMSG:
		return (char *)"No message of the desired type";
	case ENOPROTOOPT:
		return (char *)"Protocol not available";
	case ENOSPC:
		return (char *)"No space left on device";
	case ENOSR:
		return (char *)"No STREAM resources";
	case ENOSTR:
		return (char *)"Not a STREAM";
	case ENOSYS:
		return (char *)"Functionality not supported";
	case ENOTCONN:
		return (char *)"The socket is not connected";
	case ENOTDIR:
		return (char *)"Not a directory or a symbolic link to a directory";
	case ENOTEMPTY:
		return (char *)"Directory not empty";
	case ENOTRECOVERABLE:
		return (char *)"State not recoverable";
	case ENOTSOCK:
		return (char *)"Not a socket";
	case ENOTSUP:
		return (char *)"Not supported";
	case ENOTTY:
		return (char *)"Inappropriate I/O control operation";
	case ENXIO:
		return (char *)"No such device or address";
	case EOPNOTSUPP:
		return (char *)"Operation not supported on socket";
	case EOVERFLOW:
		return (char *)"Value too large to be stored in data type";
	case EOWNERDEAD:
		return (char *)"Previous owner died";
	case EPERM:
		return (char *)"Operation not permitted";
	case EPIPE:
		return (char *)"Broken pipe";
	case EPROTO:
		return (char *)"Protocol error";
	case EPROTONOSUPPORT:
		return (char *)"Protocol not supported";
	case EPROTOTYPE:
		return (char *)"Protocol wrong type for socket";
	case ERANGE:
		return (char *)"Result too large";
	case EROFS:
		return (char *)"Read-only file system";
	case ESPIPE:
		return (char *)"Invalid seek";
	case ESRCH:
		return (char *)"No such process";
	case ESTALE:
		return (char *)"Reserved";
	case ETIME:
		return (char *)"Stream ioctl() timeout";
	case ETIMEDOUT:
		return (char *)"Connection timed out";
	case ETXTBSY:
		return (char *)"Text file busy";
	case EWOULDBLOCK:
		return (char *)"Operation would block";
	case EXDEV:
		return (char *)"Cross-device link";
	default:
		return (char *)"Unknown error";
	}
}
