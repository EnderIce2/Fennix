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

#ifndef _DIRENT_H
#define _DIRENT_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>

	typedef struct dirent
	{
		ino_t d_ino;   /* File serial number. */
		char d_name[]; /* Filename string of entry. */
	} dirent;

	typedef struct posix_dent
	{
		ino_t d_ino;		  /* File serial number. */
		reclen_t d_reclen;	  /* Length of this entry, including trailing padding if necessary. See posix_getdents(). */
		unsigned char d_type; /* File type or unknown-file-type indication. */
		char d_name[];		  /* Filename string of this entry. */
	} posix_dent;

	typedef struct DIR
	{
		int __fd;
	} DIR;

#define DT_BLK
#define DT_CHR
#define DT_DIR
#define DT_FIFO
#define DT_LNK
#define DT_REG
#define DT_SOCK
#define DT_UNKNOWN

#define DT_MQ
#define DT_SEM
#define DT_SHM
#define DT_TMO

	int alphasort(const struct dirent **d1, const struct dirent **d2);
	int closedir(DIR *dirp);
	int dirfd(DIR *dirp);
	DIR *fdopendir(int fd);
	DIR *opendir(const char *dirname);
	ssize_t posix_getdents(int fildes, void *buf, size_t nbyte, int flags);
	struct dirent *readdir(DIR *dirp);
	int readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
	void rewinddir(DIR *);
	int scandir(const char *dir, struct dirent ***namelist, int (*sel)(const struct dirent *), int (*compar)(const struct dirent **, const struct dirent **));
	void seekdir(DIR *, long);
	long telldir(DIR *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_DIRENT_H
