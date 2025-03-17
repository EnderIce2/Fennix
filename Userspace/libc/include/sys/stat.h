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

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <time.h>

	struct stat
	{
		dev_t st_dev;			 /* Device ID of device containing file. */
		ino_t st_ino;			 /* File serial number. */
		mode_t st_mode;			 /* Mode of file (see below). */
		nlink_t st_nlink;		 /* Number of hard links to the file. */
		uid_t st_uid;			 /* User ID of file. */
		gid_t st_gid;			 /* Group ID of file. */
		dev_t st_rdev;			 /* Device ID (if file is character or block special). */
		off_t st_size;			 /* For regular files, the file size in bytes. For symbolic links, the length in bytes of the pathname contained in the symbolic link. For a shared memory object, the length in bytes. For a typed memory object, the length in bytes. For other file types, the use of this field is unspecified. */
		struct timespec st_atim; /* Last data access timestamp. */
		struct timespec st_mtim; /* Last data modification timestamp. */
		struct timespec st_ctim; /* Last file status change timestamp. */
		blksize_t st_blksize;	 /* A file system-specific preferred I/O block size for this object. In some file system types, this may vary from file to file. */
		blkcnt_t st_blocks;		 /* Number of blocks allocated for this object. */
	};

#define S_IFMT 0170000
#define S_IFBLK 0060000
#define S_IFCHR 0020000
#define S_IFIFO 0010000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_IRWXU 0700
#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXG 070
#define S_IRGRP 040
#define S_IWGRP 020
#define S_IXGRP 010
#define S_IRWXO 07
#define S_IROTH 04
#define S_IWOTH 02
#define S_IXOTH 01

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#define S_TYPEISMQ(buf) 0
#define S_TYPEISSEM(buf) 0
#define S_TYPEISSHM(buf) 0
#define S_TYPEISTMO(buf) 0

#define UTIME_NOW 0x3fffffff
#define UTIME_OMIT 0x3ffffffe

	int chmod(const char *, mode_t);
	int fchmod(int, mode_t);
	int fchmodat(int, const char *, mode_t, int);
	int fstat(int fildes, struct stat *buf);
	int fstatat(int fd, const char *restrict path, struct stat *restrict buf, int flag);
	int futimens(int, const struct timespec[2]);
	int lstat(const char *restrict path, struct stat *restrict buf);
	int mkdir(const char *path, mode_t mode);
	int mkdirat(int fd, const char *path, mode_t mode);
	int mkfifo(const char *, mode_t);
	int mkfifoat(int, const char *, mode_t);
	int mknod(const char *, mode_t, dev_t);
	int mknodat(int, const char *, mode_t, dev_t);
	int stat(const char *restrict path, struct stat *restrict buf);
	mode_t umask(mode_t);
	int utimensat(int, const char *, const struct timespec[2], int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SYS_STAT_H
