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

#ifndef __FENNIX_KERNEL_LINUX_ERRNO_H__
#define __FENNIX_KERNEL_LINUX_ERRNO_H__

/** Operation not permitted */
#define linux_EPERM 1

/** No such file or directory */
#define linux_ENOENT 2

/** No such process */
#define linux_ESRCH 3

/** Interrupted system call */
#define linux_EINTR 4

/** I/O error */
#define linux_EIO 5

/** No such device or address */
#define linux_ENXIO 6

/** Argument list too long */
#define linux_E2BIG 7

/** Exec format error */
#define linux_ENOEXEC 8

/** Bad file number */
#define linux_EBADF 9

/** No child processes */
#define linux_ECHILD 10

/** Try again */
#define linux_EAGAIN 11

/** Out of memory */
#define linux_ENOMEM 12

/** Permission denied */
#define linux_EACCES 13

/** Bad address */
#define linux_EFAULT 14

/** Block device required */
#define linux_ENOTBLK 15

/** Device or resource busy */
#define linux_EBUSY 16

/** File exists */
#define linux_EEXIST 17

/** Cross-device link */
#define linux_EXDEV 18

/** No such device */
#define linux_ENODEV 19

/** Not a directory */
#define linux_ENOTDIR 20

/** Is a directory */
#define linux_EISDIR 21

/** Invalid argument */
#define linux_EINVAL 22

/** File table overflow */
#define linux_ENFILE 23

/** Too many open files */
#define linux_EMFILE 24

/** Not a typewriter */
#define linux_ENOTTY 25

/** Text file busy */
#define linux_ETXTBSY 26

/** File too large */
#define linux_EFBIG 27

/** No space left on device */
#define linux_ENOSPC 28

/** Illegal seek */
#define linux_ESPIPE 29

/** Read-only file system */
#define linux_EROFS 30

/** Too many links */
#define linux_EMLINK 31

/** Broken pipe */
#define linux_EPIPE 32

/** Math argument out of domain of func */
#define linux_EDOM 33

/** Math result not representable */
#define linux_ERANGE 34

/** Resource deadlock would occur */
#define linux_EDEADLK 35

/** File name too long */
#define linux_ENAMETOOLONG 36

/** No record locks available */
#define linux_ENOLCK 37

/** Function not implemented */
#define linux_ENOSYS 38

/** Directory not empty */
#define linux_ENOTEMPTY 39

/** Too many symbolic links encountered */
#define linux_ELOOP 40

/** No message of desired type */
#define linux_ENOMSG 42

/** Identifier removed */
#define linux_EIDRM 43

/** Channel number out of range */
#define linux_ECHRNG 44

/** Level 2 not synchronized */
#define linux_EL2NSYNC 45

/** Level 3 halted */
#define linux_EL3HLT 46

/** Level 3 reset */
#define linux_EL3RST 47

/** Link number out of range */
#define linux_ELNRNG 48

/** Protocol driver not attached */
#define linux_EUNATCH 49

/** No CSI structure available */
#define linux_ENOCSI 50

/** Level 2 halted */
#define linux_EL2HLT 51

/** Invalid exchange */
#define linux_EBADE 52

/** Invalid request descriptor */
#define linux_EBADR 53

/** Exchange full */
#define linux_EXFULL 54

/** No anode */
#define linux_ENOANO 55

/** Invalid request code */
#define linux_EBADRQC 56

/** Invalid slot */
#define linux_EBADSLT 57

/** Bad font file format */
#define linux_EBFONT 59

/** Device not a stream */
#define linux_ENOSTR 60

/** No data available */
#define linux_ENODATA 61

/** Timer expired */
#define linux_ETIME 62

/** Out of streams resources */
#define linux_ENOSR 63

/** Machine is not on the network */
#define linux_ENONET 64

/** Package not installed */
#define linux_ENOPKG 65

/** Object is remote */
#define linux_EREMOTE 66

/** Link has been severed */
#define linux_ENOLINK 67

/** Advertise error */
#define linux_EADV 68

/** Srmount error */
#define linux_ESRMNT 69

/** Communication error on send */
#define linux_ECOMM 70

/** Protocol error */
#define linux_EPROTO 71

/** Multihop attempted */
#define linux_EMULTIHOP 72

/** RFS specific error */
#define linux_EDOTDOT 73

/** Not a data message */
#define linux_EBADMSG 74

/** Value too large for defined data type */
#define linux_EOVERFLOW 75

/** Name not unique on network */
#define linux_ENOTUNIQ 76

/** File descriptor in bad state */
#define linux_EBADFD 77

/** Remote address changed */
#define linux_EREMCHG 78

/** Can not access a needed shared library */
#define linux_ELIBACC 79

/** Accessing a corrupted shared library */
#define linux_ELIBBAD 80

/** .lib section in a.out corrupted */
#define linux_ELIBSCN 81

/** Attempting to link in too many shared libraries */
#define linux_ELIBMAX 82

/** Cannot exec a shared library directly */
#define linux_ELIBEXEC 83

/** Illegal byte sequence */
#define linux_EILSEQ 84

/** Interrupted system call should be restarted */
#define linux_ERESTART 85

/** Streams pipe error */
#define linux_ESTRPIPE 86

/** Too many users */
#define linux_EUSERS 87

/** Socket operation on non-socket */
#define linux_ENOTSOCK 88

/** Destination address required */
#define linux_EDESTADDRREQ 89

/** Message too long */
#define linux_EMSGSIZE 90

/** Protocol wrong type for socket */
#define linux_EPROTOTYPE 91

/** Protocol not available */
#define linux_ENOPROTOOPT 92

/** Protocol not supported */
#define linux_EPROTONOSUPPORT 93

/** Socket type not supported */
#define linux_ESOCKTNOSUPPORT 94

/** Operation not supported on transport endpoint */
#define linux_EOPNOTSUPP 95

/** Protocol family not supported */
#define linux_EPFNOSUPPORT 96

/** Address family not supported by protocol */
#define linux_EAFNOSUPPORT 97

/** Address already in use */
#define linux_EADDRINUSE 98

/** Cannot assign requested address */
#define linux_EADDRNOTAVAIL 99

/** Network is down */
#define linux_ENETDOWN 100

/** Network is unreachable */
#define linux_ENETUNREACH 101

/** Network dropped connection because of reset */
#define linux_ENETRESET 102

/** Software caused connection abort */
#define linux_ECONNABORTED 103

/** Connection reset by peer */
#define linux_ECONNRESET 104

/** No buffer space available */
#define linux_ENOBUFS 105

/** Transport endpoint is already connected */
#define linux_EISCONN 106

/** Transport endpoint is not connected */
#define linux_ENOTCONN 107

/** Cannot send after transport endpoint shutdown */
#define linux_ESHUTDOWN 108

/** Too many references: cannot splice */
#define linux_ETOOMANYREFS 109

/** Connection timed out */
#define linux_ETIMEDOUT 110

/** Connection refused */
#define linux_ECONNREFUSED 111

/** Host is down */
#define linux_EHOSTDOWN 112

/** No route to host */
#define linux_EHOSTUNREACH 113

/** Operation already in progress */
#define linux_EALREADY 114

/** Operation now in progress */
#define linux_EINPROGRESS 115

/** Stale NFS file handle */
#define linux_ESTALE 116

/** Structure needs cleaning */
#define linux_EUCLEAN 117

/** Not a XENIX named type file */
#define linux_ENOTNAM 118

/** No XENIX semaphores available */
#define linux_ENAVAIL 119

/** Is a named type file */
#define linux_EISNAM 120

/** Remote I/O error */
#define linux_EREMOTEIO 121

/** Quota exceeded */
#define linux_EDQUOT 122

/** No medium found */
#define linux_ENOMEDIUM 123

/** Wrong medium type */
#define linux_EMEDIUMTYPE 124

/** Operation Canceled */
#define linux_ECANCELED 125

/** Required key not available */
#define linux_ENOKEY 126

/** Key has expired */
#define linux_EKEYEXPIRED 127

/** Key has been revoked */
#define linux_EKEYREVOKED 128

/** Key was rejected by service */
#define linux_EKEYREJECTED 129

/** Owner died */
#define linux_EOWNERDEAD 130

/** State not recoverable */
#define linux_ENOTRECOVERABLE 131

#endif // !__FENNIX_KERNEL_LINUX_ERRNO_H__
