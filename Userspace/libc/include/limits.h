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

#ifndef _LIMITS_H
#define _LIMITS_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define _POSIX_ARG_MAX 4096
#define _POSIX_CHILD_MAX 25
#define _POSIX_OPEN_MAX 20
#define _POSIX_LINK_MAX 8
#define _POSIX_MAX_CANON 255
#define _POSIX_MAX_INPUT 255
#define _POSIX_NAME_MAX 255
#define _POSIX_PATH_MAX 4096
#define _POSIX_PIPE_BUF 512
#define _POSIX_SYMLOOP_MAX 8
#define _POSIX_HOST_NAME_MAX 255
#define _POSIX_LOGIN_NAME_MAX 9
#define _POSIX_TTY_NAME_MAX 255
#define _POSIX_TZNAME_MAX 6

#define _XOPEN_SHM_MAX 255
#define _XOPEN_IOV_MAX 1024
#define _XOPEN_SSTREAM_MAX 8
#define _XOPEN_NAME_MAX 255
#define _XOPEN_PATH_MAX 4096
#define _XOPEN_SYMLOOP_MAX 8
#define _XOPEN_LOGIN_NAME_MAX 9

#define _POSIX2_BC_BASE_MAX 99
#define _POSIX2_BC_DIM_MAX 2048
#define _POSIX2_BC_SCALE_MAX 99
#define _POSIX2_BC_STRING_MAX 1000
#define _POSIX2_COLL_WEIGHTS_MAX 255
#define _POSIX2_EXPR_NEST_MAX 32
#define _POSIX2_LINE_MAX 2048
#define _POSIX2_RE_DUP_MAX 255

#define CHAR_BIT __CHAR_BIT__
#define SCHAR_MIN (-SCHAR_MAX - 1)
#define SCHAR_MAX __SCHAR_MAX__

#if __SCHAR_MAX__ == __INT_MAX__
#define UCHAR_MAX (SCHAR_MAX * 2U + 1U)
#else
#define UCHAR_MAX (SCHAR_MAX * 2 + 1)
#endif

#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX

#define MB_LEN_MAX 1
#define SHRT_MIN (-SHRT_MAX - 1)
#define SHRT_MAX __SHRT_MAX__

#if __SHRT_MAX__ == __INT_MAX__
#define USHRT_MAX (SHRT_MAX * 2U + 1U)
#else
#define USHRT_MAX (SHRT_MAX * 2 + 1)
#endif

#define INT_MIN (-INT_MAX - 1)
#define INT_MAX __INT_MAX__

#define UINT_MAX (INT_MAX * 2U + 1U)

#define LONG_MIN (-LONG_MAX - 1L)
#define LONG_MAX __LONG_MAX__

#define ULONG_MAX (LONG_MAX * 2UL + 1UL)

#define LLONG_MIN (-__LONG_LONG_MAX__ - 1LL)
#define LLONG_MAX __LONG_LONG_MAX__

#define ULLONG_MAX (LLONG_MAX * 2ULL + 1ULL)

#define ARG_MAX 4096
#define CHILD_MAX 25
#define OPEN_MAX 20
#define LINK_MAX 8
#define MAX_CANON 255
#define MAX_INPUT 255
#define NAME_MAX 255
#define PATH_MAX 4096
#define PIPE_BUF 512
#define SYMLOOP_MAX 8
#define HOST_NAME_MAX 255
#define LOGIN_NAME_MAX 9
#define TTY_NAME_MAX 255
#define TZNAME_MAX 6

#define SSIZE_MAX ((size_t)(-1) / 2)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_LIMITS_H
