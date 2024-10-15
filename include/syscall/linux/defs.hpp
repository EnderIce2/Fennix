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

#define linux_SEEK_SET 0
#define linux_SEEK_CUR 1
#define linux_SEEK_END 2

#define linux_ARCH_SET_GS 0x1001
#define linux_ARCH_SET_FS 0x1002
#define linux_ARCH_GET_FS 0x1003
#define linux_ARCH_GET_GS 0x1004

#define linux_ARCH_GET_CPUID 0x1011
#define linux_ARCH_SET_CPUID 0x1012

#define linux_ARCH_GET_XCOMP_SUPP 0x1021
#define linux_ARCH_GET_XCOMP_PERM 0x1022
#define linux_ARCH_REQ_XCOMP_PERM 0x1023
#define linux_ARCH_GET_XCOMP_GUEST_PERM 0x1024
#define linux_ARCH_REQ_XCOMP_GUEST_PERM 0x1025

#define linux_ARCH_XCOMP_TILECFG 17
#define linux_ARCH_XCOMP_TILEDATA 18

#define linux_ARCH_MAP_VDSO_X32 0x2001
#define linux_ARCH_MAP_VDSO_32 0x2002
#define linux_ARCH_MAP_VDSO_64 0x2003

#define linux_ARCH_GET_UNTAG_MASK 0x4001
#define linux_ARCH_ENABLE_TAGGED_ADDR 0x4002
#define linux_ARCH_GET_MAX_TAG_BITS 0x4003
#define linux_ARCH_FORCE_TAGGED_SVA 0x4004

#define linux_PROT_NONE 0
#define linux_PROT_READ 1
#define linux_PROT_WRITE 2
#define linux_PROT_EXEC 4
#define linux_PROT_GROWSDOWN 0x01000000
#define linux_PROT_GROWSUP 0x02000000

#define linux_MAP_TYPE 0x0f

#define linux_MAP_FILE 0
#define linux_MAP_SHARED 0x01
#define linux_MAP_PRIVATE 0x02
#define linux_MAP_SHARED_VALIDATE 0x03
#define linux_MAP_FIXED 0x10
#define linux_MAP_ANONYMOUS 0x20
#define linux_MAP_NORESERVE 0x4000
#define linux_MAP_GROWSDOWN 0x0100
#define linux_MAP_DENYWRITE 0x0800
#define linux_MAP_EXECUTABLE 0x1000
#define linux_MAP_LOCKED 0x2000
#define linux_MAP_POPULATE 0x8000
#define linux_MAP_NONBLOCK 0x10000
#define linux_MAP_STACK 0x20000
#define linux_MAP_HUGETLB 0x40000
#define linux_MAP_SYNC 0x80000
#define linux_MAP_FIXED_NOREPLACE 0x100000

#define linux_CLOCK_REALTIME 0
#define linux_CLOCK_MONOTONIC 1
#define linux_CLOCK_PROCESS_CPUTIME_ID 2
#define linux_CLOCK_THREAD_CPUTIME_ID 3
#define linux_CLOCK_MONOTONIC_RAW 4
#define linux_CLOCK_REALTIME_COARSE 5
#define linux_CLOCK_MONOTONIC_COARSE 6
#define linux_CLOCK_BOOTTIME 7
#define linux_CLOCK_REALTIME_ALARM 8
#define linux_CLOCK_BOOTTIME_ALARM 9
#define linux_CLOCK_SGI_CYCLE 10
#define linux_CLOCK_TAI 11

#define linux_GRND_NONBLOCK 0x1
#define linux_GRND_RANDOM 0x2
#define linux_GRND_INSECURE 0x4

#define linux_RLIMIT_CPU 0
#define linux_RLIMIT_FSIZE 1
#define linux_RLIMIT_DATA 2
#define linux_RLIMIT_STACK 3
#define linux_RLIMIT_CORE 4
#define linux_RLIMIT_RSS 5
#define linux_RLIMIT_NPROC 6
#define linux_RLIMIT_NOFILE 7
#define linux_RLIMIT_MEMLOCK 8
#define linux_RLIMIT_AS 9
#define linux_RLIMIT_LOCKS 10
#define linux_RLIMIT_SIGPENDING 11
#define linux_RLIMIT_MSGQUEUE 12
#define linux_RLIMIT_NICE 13
#define linux_RLIMIT_RTPRIO 14
#define linux_RLIMIT_RTTIME 15
#define linux_RLIMIT_NLIMITS 16

#define linux_F_DUPFD 0
#define linux_F_GETFD 1
#define linux_F_SETFD 2
#define linux_F_GETFL 3
#define linux_F_SETFL 4

#define linux_F_SETOWN 8
#define linux_F_GETOWN 9
#define linux_F_SETSIG 10
#define linux_F_GETSIG 11

#if __LONG_MAX == 0x7fffffffL
#define linux_F_GETLK 12
#define linux_F_SETLK 13
#define linux_F_SETLKW 14
#else
#define linux_F_GETLK 5
#define linux_F_SETLK 6
#define linux_F_SETLKW 7
#endif

#define linux_F_SETOWN_EX 15
#define linux_F_GETOWN_EX 16
#define linux_F_GETOWNER_UIDS 17

#define linux_F_OFD_GETLK 36
#define linux_F_OFD_SETLK 37
#define linux_F_OFD_SETLKW 38

#define linux_F_DUPFD_CLOEXEC 1030

#define linux_FD_CLOEXEC 1

#define linux_DT_UNKNOWN 0
#define linux_DT_FIFO 1
#define linux_DT_CHR 2
#define linux_DT_DIR 4
#define linux_DT_BLK 6
#define linux_DT_REG 8
#define linux_DT_LNK 10
#define linux_DT_SOCK 12
#define linux_DT_WHT 14

#define linux_AT_FDCWD (-100)
#define linux_AT_SYMLINK_NOFOLLOW 0x100
#define linux_AT_REMOVEDIR 0x200
#define linux_AT_SYMLINK_FOLLOW 0x400
#define linux_AT_EACCESS 0x200
#define linux_AT_NO_AUTOMOUNT 0x800
#define linux_AT_EMPTY_PATH 0x1000
#define linux_AT_STATX_SYNC_TYPE 0x6000
#define linux_AT_STATX_SYNC_AS_STAT 0x0000
#define linux_AT_STATX_FORCE_SYNC 0x2000
#define linux_AT_STATX_DONT_SYNC 0x4000
#define linux_AT_RECURSIVE 0x8000

#define linux_LINUX_REBOOT_MAGIC1 0xfee1dead
#define linux_LINUX_REBOOT_MAGIC2 0x28121969
#define linux_LINUX_REBOOT_MAGIC2A 0x05121996
#define linux_LINUX_REBOOT_MAGIC2B 0x16041998
#define linux_LINUX_REBOOT_MAGIC2C 0x20112000

#define linux_LINUX_REBOOT_CMD_RESTART 0x01234567
#define linux_LINUX_REBOOT_CMD_HALT 0xCDEF0123
#define linux_LINUX_REBOOT_CMD_CAD_ON 0x89ABCDEF
#define linux_LINUX_REBOOT_CMD_CAD_OFF 0x00000000
#define linux_LINUX_REBOOT_CMD_POWER_OFF 0x4321FEDC
#define linux_LINUX_REBOOT_CMD_RESTART2 0xA1B2C3D4
#define linux_LINUX_REBOOT_CMD_SW_SUSPEND 0xD000FCE2
#define linux_LINUX_REBOOT_CMD_KEXEC 0x45584543

#define linux_SA_IMMUTABLE 0x00800000

#define linux_ITIMER_REAL 0
#define linux_ITIMER_VIRTUAL 1
#define linux_ITIMER_PROF 2

#define linux_RUSAGE_SELF 0
#define linux_RUSAGE_CHILDREN (-1)
#define linux_RUSAGE_THREAD 1

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
	unsigned long d_ino;
	unsigned long d_off;
	unsigned short d_reclen;
	char d_name[];
	/**
	 * Getting d_type is not the same as linux_dirent64:
	 * https://github.com/torvalds/linux/blob/bfa8f18691ed2e978e4dd51190569c434f93e268/fs/readdir.c#L296
	 * "man 2 getdents" also is helpful
	 */
};

struct linux_dirent64
{
	ino64_t d_ino;
	off64_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
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
