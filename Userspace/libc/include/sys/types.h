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

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define __iptr __INTPTR_TYPE__
	__iptr __check_errno(__iptr status, __iptr err);

#ifndef restrict
#define restrict __restrict__
#endif // restrict

#ifndef export
#define export __attribute__((__visibility__("default")))
#endif // export

	typedef long blkcnt_t;

	typedef long blksize_t;

	typedef long clock_t;
	typedef int clockid_t;

	typedef unsigned long dev_t;

	typedef unsigned long fsblkcnt_t;
	typedef unsigned long fsfilcnt_t;

	typedef unsigned int gid_t;
	typedef unsigned int id_t;
	typedef unsigned long ino_t;
	typedef unsigned short reclen_t;

	typedef int key_t;

	typedef unsigned int mode_t;
	typedef unsigned int nlink_t;

	typedef long off_t;

	typedef int pid_t;

	typedef struct pthread_attr_t
	{
		char __data;
	} pthread_attr_t;

	typedef struct pthread_cond_t
	{
		char __data;
	} pthread_cond_t;

	typedef struct pthread_condattr_t
	{
		char __data;
	} pthread_condattr_t;

	typedef unsigned int pthread_key_t;

	typedef struct pthread_mutex_t
	{
		short locked;
		char __data;
	} pthread_mutex_t;

	typedef struct pthread_mutexattr_t
	{
		char __data;
	} pthread_mutexattr_t;

	typedef struct pthread_once_t
	{
		int __initialized;
	} pthread_once_t;

	typedef struct pthread_rwlock_t
	{
		char __data;
	} pthread_rwlock_t;

	typedef struct pthread_rwlockattr_t
	{
		char __data;
	} pthread_rwlockattr_t;

	typedef struct pthread_barrier_t
	{
		char __data;
	} pthread_barrier_t;

	typedef __SIZE_TYPE__ size_t;
	typedef long ssize_t;

	typedef long suseconds_t;
	typedef long time_t;

	typedef int timer_t;

	typedef unsigned int uid_t;
	typedef unsigned int useconds_t;

	typedef struct __pthread
	{
		struct __pthread *Self;
		/* For __tls_get_addr */
		__UINTPTR_TYPE__ *Storage;

		int CurrentError;
	} __pthread;

#ifdef __cplusplus
	typedef unsigned long pthread_t;
#else
typedef struct __pthread *pthread_t;
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SYS_TYPES_H
