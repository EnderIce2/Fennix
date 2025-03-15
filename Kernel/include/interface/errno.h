/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FENNIX_API_ERRNO_H__
#define __FENNIX_API_ERRNO_H__

/**
 * The documentation for these error codes are from:
 *  https://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html
 *
 * Full list:
 *  https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/basedefs/errno.h.html
 */

/**
 * No Error
 */
#define EOK 0

/**
 * Argument list too long. The sum of the number of bytes used by the
 *  new process image's argument list and environment list is greater
 *  than the system-imposed limit of {ARG_MAX} bytes.
 * or:
 * Lack of space in an output buffer.
 * or:
 * Argument is greater than the system-imposed maximum.
 */
#define E2BIG 1

/**
 * Permission denied. An attempt was made to access a file in a way
 *  forbidden by its file access permissions.
 */
#define EACCES 2

/**
 * Address in use. The specified address is in use.
 */
#define EADDRINUSE 3

/**
 * Address not available. The specified address is not available from
 *  the local system.
 */
#define EADDRNOTAVAIL 4

/**
 * Address family not supported. The implementation does not support
 *  the specified address family, or the specified address is not a
 *  valid address for the address family of the specified socket.
 */
#define EAFNOSUPPORT 5

/**
 * Resource temporarily unavailable. This is a temporary condition
 *  and later calls to the same routine may complete normally.
 */
#define EAGAIN 6

/**
 * Connection already in progress. A connection request is already in
 *  progress for the specified socket.
 */
#define EALREADY 7

/**
 * Bad file descriptor. A file descriptor argument is out of range,
 *  refers to no open file, or a read (write) request is made to a
 *  file that is only open for writing (reading).
 */
#define EBADF 8

/**
 * Bad message. During a read(), getmsg(), getpmsg(), or ioctl()
 *  I_RECVFD request to a STREAMS device, a message arrived at the
 *  head of the STREAM that is inappropriate for the function
 *  receiving the message.
 * read()
 * Message waiting to be read on a STREAM is not a data message.
 * getmsg() or getpmsg()
 * A file descriptor was received instead of a control message.
 * ioctl()
 * Control or data information was received instead of a file
 *  descriptor when I_RECVFD was specified.
 */
#define EBADMSG 9

/**
 * Resource busy. An attempt was made to make use of a system
 *  resource that is not currently available, as it is being
 *  used by another process in a manner that would have
 *  conflicted with the request being made by this process.
 */
#define EBUSY 10

/**
 * Operation canceled. The associated asynchronous operation was
 *  canceled before completion.
 */
#define ECANCELED 11

/**
 * No child process. A wait(), waitid(), or waitpid() function was
 *  executed by a process that had no existing or unwaited-for
 *  child process.
 */
#define ECHILD 12

/**
 * Connection aborted. The connection has been aborted.
 */
#define ECONNABORTED 13

/**
 * Connection refused. An attempt to connect to a socket was refused
 *  because there was no process listening or because the queue of
 *  connection requests was full and the underlying protocol does not
 *  support retransmissions.
 */
#define ECONNREFUSED 14

/**
 * Connection reset. The connection was forcibly closed by the peer.
 */
#define ECONNRESET 15

/**
 * Resource deadlock would occur. An attempt was made to lock a system
 *  resource that would have resulted in a deadlock situation.
 */
#define EDEADLK 16

/**
 * Destination address required. No bind address was established.
 */
#define EDESTADDRREQ 17

/**
 * Domain error. An input argument is outside the defined domain of the
 *  mathematical function (defined in the ISO C standard).
 */
#define EDOM 18

/**
 * Reserved.
 */
#define EDQUOT 19

/**
 * File exists. An existing file was mentioned in an inappropriate
 *  context; for example, as a new link name in the link() function.
 */
#define EEXIST 20

/**
 * Bad address. The system detected an invalid address in attempting
 *  to use an argument of a call. The reliable detection of this error
 *  cannot be guaranteed, and when not detected may result in the
 *  generation of a signal, indicating an address violation, which is
 *  sent to the process.
 */
#define EFAULT 21

/**
 * File too large. The size of a file would exceed the maximum file
 *  size of an implementation or offset maximum established in the
 *  corresponding file description.
 */
#define EFBIG 22

/**
 * Host is unreachable. The destination host cannot be reached
 *  (probably because the host is down or a remote router cannot
 *  reach it).
 */
#define EHOSTUNREACH 23

/**
 * Identifier removed. Returned during XSI interprocess communication
 *  if an identifier has been removed from the system.
 */
#define EIDRM 24

/**
 * Illegal byte sequence. A wide-character code has been detected that
 *  does not correspond to a valid character, or a byte sequence does
 *  not form a valid wide-character code (defined in the ISO C standard).
 */
#define EILSEQ 25

/**
 * Operation in progress. This code is used to indicate that an
 *  asynchronous operation has not yet completed.
 * or:
 * O_NONBLOCK is set for the socket file descriptor and the connection
 *  cannot be immediately established.
 */
#define EINPROGRESS 26

/**
 * Interrupted function call. An asynchronous signal was caught by the
 *  process during the execution of an interruptible function. If the
 *  signal handler performs a normal return, the interrupted function
 *  call may return this condition (see the Base Definitions volume
 *  of POSIX.1-2017, <signal.h>).
 */
#define EINTR 27

/**
 * Invalid argument. Some invalid argument was supplied; for example,
 *  specifying an undefined signal in a signal() function or a
 *  kill() function.
 */
#define EINVAL 28

/**
 * Input/output error. Some physical input or output error has occurred.
 *  This error may be reported on a subsequent operation on the same
 *  file descriptor. Any other error-causing operation on the same file
 *  descriptor may cause the [EIO] error indication to be lost.
 */
#define EIO 29

/**
 * Socket is connected. The specified socket is already connected.
 */
#define EISCONN 30

/**
 * Is a directory. An attempt was made to open a directory with write
 *  mode specified.
 */
#define EISDIR 31

/**
 * Symbolic link loop. A loop exists in symbolic links encountered
 *  during pathname resolution. This error may also be returned if
 *  more than {SYMLOOP_MAX} symbolic links are encountered during
 *  pathname resolution.
 */
#define ELOOP 32

/**
 * File descriptor value too large or too many open streams. An
 *  attempt was made to open a file descriptor with a value greater
 *  than or equal to {OPEN_MAX}, or an attempt was made to open more
 *  than the maximum number of streams allowed in the process.
 */
#define EMFILE 33

/**
 * Too many links. An attempt was made to have the link count of a
 *  single file exceed {LINK_MAX}.
 */
#define EMLINK 34

/**
 * Message too large. A message sent on a transport provider was
 *  larger than an internal message buffer or some other network limit.
 * or:
 * Inappropriate message buffer length.
 */
#define EMSGSIZE 35

/**
 * Reserved.
 */
#define EMULTIHOP 36

/**
 * Filename too long. The length of a pathname exceeds {PATH_MAX} and
 *  the implementation considers this to be an error, or a pathname
 *  component is longer than {NAME_MAX}. This error may also occur
 *  when pathname substitution, as a result of encountering a
 *  symbolic link during pathname resolution, results in a pathname
 *  string the size of which exceeds {PATH_MAX}.
 */
#define ENAMETOOLONG 37

/**
 * Network is down. The local network interface used to reach the
 *  destination is down.
 */
#define ENETDOWN 38

/**
 * The connection was aborted by the network.
 */
#define ENETRESET 39

/**
 * Network unreachable. No route to the network is present.
 */
#define ENETUNREACH 40

/**
 * Too many files open in system. Too many files are currently open
 *  in the system. The system has reached its predefined limit for
 *  simultaneously open files and temporarily cannot accept requests
 *  to open another one.
 */
#define ENFILE 41

/**
 * No buffer space available. Insufficient buffer resources were
 *  available in the system to perform the socket operation.
 */
#define ENOBUFS 42

/**
 * No message available. No message is available on the STREAM head
 *  read queue.
 */
#define ENODATA 43

/**
 * No such device. An attempt was made to apply an inappropriate
 *  function to a device; for example, trying to read a write-only
 *  device such as a printer.
 */
#define ENODEV 44

/**
 * No such file or directory. A component of a specified pathname
 *  does not exist, or the pathname is an empty string.
 */
#define ENOENT 45

/**
 * Executable file format error. A request is made to execute a file
 *  that, although it has appropriate privileges, is not in the
 *  format required by the implementation for executable files.
 */
#define ENOEXEC 46

/**
 * No locks available. A system-imposed limit on the number of
 *  simultaneous file and record locks has been reached and no more
 *  are currently available.
 */
#define ENOLCK 47

/**
 * Reserved.
 */
#define ENOLINK 48

/**
 * Not enough space. The new process image requires more memory than
 *  is allowed by the hardware or system-imposed memory management
 *  constraints.
 */
#define ENOMEM 49

/**
 * No message of the desired type. The message queue does not contain
 *  a message of the required type during XSI interprocess communication.
 */
#define ENOMSG 50

/**
 * Protocol not available. The protocol option specified to
 *  setsockopt() is not supported by the implementation.
 */
#define ENOPROTOOPT 51

/**
 * No space left on a device. During the write() function on a
 *  regular file or when extending a directory, there is no free
 *  space left on the device.
 */
#define ENOSPC 52

/**
 * No STREAM resources. Insufficient STREAMS memory resources are
 *  available to perform a STREAMS-related function. This is a
 *  temporary condition; it may be recovered from if other
 *  processes release resources.
 */
#define ENOSR 53

/**
 * Not a STREAM. A STREAM function was attempted on a file descriptor
 *  that was not associated with a STREAMS device.
 */
#define ENOSTR 54

/**
 * Functionality not supported. An attempt was made to use optional
 *  functionality that is not supported in this implementation.
 */
#define ENOSYS 55

/**
 * Socket not connected. The socket is not connected.
 */
#define ENOTCONN 56

/**
 * Not a directory. A component of the specified pathname exists, but
 *  it is not a directory, when a directory was expected; or an
 *  attempt was made to create a non-directory file, and the specified
 *  pathname contains at least one non- \<slash\> character and ends
 *  with one or more trailing \<slash\> characters.
 */
#define ENOTDIR 57

/**
 * Directory not empty. A directory other than an empty directory
 *  was supplied when an empty directory was expected.
 */
#define ENOTEMPTY 58

/**
 * State not recoverable. The state protected by a robust mutex
 *  is not recoverable.
 */
#define ENOTRECOVERABLE 59

/**
 * Not a socket. The file descriptor does not refer to a socket.
 */
#define ENOTSOCK 60

/**
 * Not supported. The implementation does not support the requested
 *  feature or value.
 */
#define ENOTSUP 61

/**
 * Inappropriate I/O control operation. A control function has been
 *  attempted for a file or special file for which the operation
 *  is inappropriate.
 */
#define ENOTTY 62

/**
 * No such device or address. Input or output on a special file
 *  refers to a device that does not exist, or makes a request
 *  beyond the capabilities of the device. It may also occur when,
 *  for example, a tape drive is not on-line.
 */
#define ENXIO 63

/**
 * Operation not supported on socket. The type of socket (address
 *  family or protocol) does not support the requested operation.
 */
#define EOPNOTSUPP 64

/**
 * Value too large to be stored in data type. An operation was
 *  attempted which would generate a value that is outside the
 *  range of values that can be represented in the relevant data
 *  type or that are allowed for a given data item.
 */
#define EOVERFLOW 65

/**
 * Previous owner died. The owner of a robust mutex terminated
 *  while holding the mutex lock.
 */
#define EOWNERDEAD 66

/**
 * Operation not permitted. An attempt was made to perform an
 *  operation limited to processes with appropriate privileges or
 *  to the owner of a file or other resource.
 */
#define EPERM 67

/**
 * Broken pipe. A write was attempted on a socket, pipe, or FIFO
 *  for which there is no process to read the data.
 */
#define EPIPE 68

/**
 * Protocol error. Some protocol error occurred. This error is
 *  device-specific, but is generally not related to a
 *  hardware failure.
 */
#define EPROTO 69

/**
 * Protocol not supported. The protocol is not supported by the
 *  address family, or the protocol is not supported by
 *  the implementation.
 */
#define EPROTONOSUPPORT 70

/**
 * Protocol wrong type for socket. The socket type is not
 *  supported by the protocol.
 */
#define EPROTOTYPE 71

/**
 * Result too large or too small. The result of the function
 *  is too large (overflow) or too small (underflow) to be
 *  represented in the available space.
 */
#define ERANGE 72

/**
 * Read-only file system. An attempt was made to modify a file
 *  or directory on a file system that is read-only.
 */
#define EROFS 73

/**
 * Invalid seek. An attempt was made to access the file offset
 *  associated with a pipe or FIFO.
 */
#define ESPIPE 74

/**
 * No such process. No process can be found corresponding to that
 *  specified by the given process ID.
 */
#define ESRCH 75

/**
 * Reserved.
 */
#define ESTALE 76

/**
 * STREAM ioctl() timeout. The timer set for a STREAMS ioctl() call
 *  has expired. The cause of this error is device-specific and could
 *  indicate either a hardware or software failure, or a timeout
 *  value that is too short for the specific operation. The status
 *  of the ioctl() operation is unspecified.
 */
#define ETIME 77

/**
 * Connection timed out. The connection to a remote machine has
 *  timed out.
 * If the connection timed out during execution of the function that
 *  reported this error (as opposed to timing out prior to the
 *  function being called), it is unspecified whether the function
 *  has completed some or all of the documented behavior associated
 *  with a successful completion of the function.
 * or:
 * Operation timed out. The time limit associated with the operation
 *  was exceeded before the operation completed.
 */
#define ETIMEDOUT 78

/**
 * Text file busy. An attempt was made to execute a pure-procedure
 *  program that is currently open for writing, or an attempt has
 *  been made to open for writing a pure-procedure program that
 *  is being executed.
 */
#define ETXTBSY 79

/**
 * Operation would block. An operation on a socket marked as
 *  non-blocking has encountered a situation such as no data available
 *  that otherwise would have caused the function to suspend execution.
 */
#define EWOULDBLOCK 80

/**
 * Improper link. A link to a file on another file system was attempted.
 */
#define EXDEV 81

#ifdef __kernel__
#define __ERRNO_MAX 82
#endif

#endif // !__FENNIX_API_ERRNO_H__
