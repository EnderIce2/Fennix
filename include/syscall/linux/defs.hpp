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

#ifndef __FENNIX_KERNEL_LINUX_DEFS_H__
#define __FENNIX_KERNEL_LINUX_DEFS_H__

#include <types.h>

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

#define ARCH_GET_CPUID 0x1011
#define ARCH_SET_CPUID 0x1012

#define ARCH_GET_XCOMP_SUPP 0x1021
#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define ARCH_GET_XCOMP_GUEST_PERM 0x1024
#define ARCH_REQ_XCOMP_GUEST_PERM 0x1025

#define ARCH_XCOMP_TILECFG 17
#define ARCH_XCOMP_TILEDATA 18

#define ARCH_MAP_VDSO_X32 0x2001
#define ARCH_MAP_VDSO_32 0x2002
#define ARCH_MAP_VDSO_64 0x2003

#define ARCH_GET_UNTAG_MASK 0x4001
#define ARCH_ENABLE_TAGGED_ADDR 0x4002
#define ARCH_GET_MAX_TAG_BITS 0x4003
#define ARCH_FORCE_TAGGED_SVA 0x4004

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP 0x02000000

#define MAP_TYPE 0x0f

#define MAP_FILE 0
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_SHARED_VALIDATE 0x03
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_NORESERVE 0x4000
#define MAP_GROWSDOWN 0x0100
#define MAP_DENYWRITE 0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED 0x2000
#define MAP_POPULATE 0x8000
#define MAP_NONBLOCK 0x10000
#define MAP_STACK 0x20000
#define MAP_HUGETLB 0x40000
#define MAP_SYNC 0x80000
#define MAP_FIXED_NOREPLACE 0x100000

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9
#define CLOCK_SGI_CYCLE 10
#define CLOCK_TAI 11

#define GRND_NONBLOCK 0x1
#define GRND_RANDOM 0x2
#define GRND_INSECURE 0x4

#define RLIMIT_CPU 0
#define RLIMIT_FSIZE 1
#define RLIMIT_DATA 2
#define RLIMIT_STACK 3
#define RLIMIT_CORE 4
#define RLIMIT_RSS 5
#define RLIMIT_NPROC 6
#define RLIMIT_NOFILE 7
#define RLIMIT_MEMLOCK 8
#define RLIMIT_AS 9
#define RLIMIT_LOCKS 10
#define RLIMIT_SIGPENDING 11
#define RLIMIT_MSGQUEUE 12
#define RLIMIT_NICE 13
#define RLIMIT_RTPRIO 14
#define RLIMIT_RTTIME 15
#define RLIMIT_NLIMITS 16

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4

#define F_SETOWN 8
#define F_GETOWN 9
#define F_SETSIG 10
#define F_GETSIG 11

#if __LONG_MAX == 0x7fffffffL
#define F_GETLK 12
#define F_SETLK 13
#define F_SETLKW 14
#else
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#endif

#define F_SETOWN_EX 15
#define F_GETOWN_EX 16
#define F_GETOWNER_UIDS 17

#define F_OFD_GETLK 36
#define F_OFD_SETLK 37
#define F_OFD_SETLKW 38

#define F_DUPFD_CLOEXEC 1030

#define FD_CLOEXEC 1

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

#define AT_FDCWD (-100)
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR 0x200
#define AT_SYMLINK_FOLLOW 0x400
#define AT_EACCESS 0x200
#define AT_NO_AUTOMOUNT 0x800
#define AT_EMPTY_PATH 0x1000
#define AT_STATX_SYNC_TYPE 0x6000
#define AT_STATX_SYNC_AS_STAT 0x0000
#define AT_STATX_FORCE_SYNC 0x2000
#define AT_STATX_DONT_SYNC 0x4000
#define AT_RECURSIVE 0x8000

#define LINUX_REBOOT_MAGIC1 0xfee1dead
#define LINUX_REBOOT_MAGIC2 0x28121969
#define LINUX_REBOOT_MAGIC2A 0x05121996
#define LINUX_REBOOT_MAGIC2B 0x16041998
#define LINUX_REBOOT_MAGIC2C 0x20112000

#define LINUX_REBOOT_CMD_RESTART 0x01234567
#define LINUX_REBOOT_CMD_HALT 0xCDEF0123
#define LINUX_REBOOT_CMD_CAD_ON 0x89ABCDEF
#define LINUX_REBOOT_CMD_CAD_OFF 0x00000000
#define LINUX_REBOOT_CMD_POWER_OFF 0x4321FEDC
#define LINUX_REBOOT_CMD_RESTART2 0xA1B2C3D4
#define LINUX_REBOOT_CMD_SW_SUSPEND 0xD000FCE2
#define LINUX_REBOOT_CMD_KEXEC 0x45584543

#define SA_IMMUTABLE 0x00800000

#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

#define RUSAGE_SELF 0
#define RUSAGE_CHILDREN (-1)
#define RUSAGE_THREAD 1

typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;
typedef long __kernel_old_time_t;
typedef long __kernel_suseconds_t;
typedef unsigned long timeu64_t;
typedef int clockid_t;
typedef long time64_t;

struct f_owner_ex
{
	int type;
	pid_t pid;
};

struct iovec
{
	void *iov_base;
	size_t iov_len;
};

struct timeval
{
	__kernel_old_time_t tv_sec;
	__kernel_suseconds_t tv_usec;
};

struct timespec64
{
	time64_t tv_sec;
	long tv_nsec;
};

struct itimerspec64
{
	struct timespec64 it_interval;
	struct timespec64 it_value;
};

struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
	long ru_maxrss;
	long ru_ixrss;
	long ru_idrss;
	long ru_isrss;
	long ru_minflt;
	long ru_majflt;
	long ru_nswap;
	long ru_inblock;
	long ru_oublock;
	long ru_msgsnd;
	long ru_msgrcv;
	long ru_nsignals;
	long ru_nvcsw;
	long ru_nivcsw;
};

struct linux_dirent
{
	unsigned long d_ino;	 /* Inode number */
	unsigned long d_off;	 /* Offset to next linux_dirent */
	unsigned short d_reclen; /* Length of this linux_dirent */
	char d_name[];			 /* Filename (null-terminated) */
};

struct linux_dirent64
{
	ino64_t d_ino;			 /* 64-bit inode number */
	off64_t d_off;			 /* 64-bit offset to next structure */
	unsigned short d_reclen; /* Size of this dirent */
	unsigned char d_type;	 /* File type */
	char d_name[];			 /* Filename (null-terminated) */
};

struct linux_kstat
{
#if defined(a64)
	__kernel_ulong_t st_dev;
	__kernel_ulong_t st_ino;
	__kernel_ulong_t st_nlink;
	unsigned int st_mode;
	unsigned int st_uid;
	unsigned int st_gid;
	unsigned int __pad0;
	__kernel_ulong_t st_rdev;
	__kernel_long_t st_size;
	__kernel_long_t st_blksize;
	__kernel_long_t st_blocks;
	__kernel_ulong_t st_atime;
	__kernel_ulong_t st_atime_nsec;
	__kernel_ulong_t st_mtime;
	__kernel_ulong_t st_mtime_nsec;
	__kernel_ulong_t st_ctime;
	__kernel_ulong_t st_ctime_nsec;
#undef __unused
	__kernel_long_t __unused[3];
#elif defined(a32)
	unsigned long st_dev;
	unsigned long st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned long st_rdev;
	unsigned long st_size;
	unsigned long st_blksize;
	unsigned long st_blocks;
	unsigned long st_atime;
	unsigned long st_atime_nsec;
	unsigned long st_mtime;
	unsigned long st_mtime_nsec;
	unsigned long st_ctime;
	unsigned long st_ctime_nsec;
	unsigned long __unused4;
	unsigned long __unused5;
#else
#error "Unsupported architecture"
#endif
};

struct linux_kstat64
{
	unsigned long long st_dev;
	unsigned char __pad0[4];
	unsigned long __st_ino;
	unsigned int st_mode;
	unsigned int st_nlink;
	unsigned long st_uid;
	unsigned long st_gid;
	unsigned long long st_rdev;
	unsigned char __pad3[4];
	long long st_size;
	unsigned long st_blksize;
	unsigned long long st_blocks;
	unsigned long st_atime;
	unsigned long st_atime_nsec;
	unsigned long st_mtime;
	unsigned int st_mtime_nsec;
	unsigned long st_ctime;
	unsigned long st_ctime_nsec;
	unsigned long long st_ino;
};

struct __old_kernel_stat
{
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
#ifdef __i386__
	unsigned long st_size;
	unsigned long st_atime;
	unsigned long st_mtime;
	unsigned long st_ctime;
#else
	unsigned int st_size;
	unsigned int st_atime;
	unsigned int st_mtime;
	unsigned int st_ctime;
#endif
};

#endif // !__FENNIX_KERNEL_LINUX_DEFS_H__
