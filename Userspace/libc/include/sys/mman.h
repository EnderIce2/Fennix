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

#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x4
#define MAP_ANONYMOUS 0x8
#define MAP_ANON MAP_ANONYMOUS

#define MS_ASYNC 0x01
#define MS_SYNC 0x02
#define MS_INVALIDATE 0x04

#define MCL_CURRENT 0x01
#define MCL_FUTURE 0x02

#define MAP_FAILED ((void *)-1)

	typedef struct posix_typed_mem_info
	{
		/* Maximum length which may be allocated from a typed memory object. */
		size_t posix_tmi_length;
	} posix_typed_mem_info;

	int mlock(const void *, size_t);
	int mlockall(int);
	void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
	int mprotect(void *addr, size_t len, int prot);
	int msync(void *, size_t, int);
	int munlock(const void *, size_t);
	int munlockall(void);
	int munmap(void *addr, size_t len);
	int posix_madvise(void *, size_t, int);
	int posix_mem_offset(const void *restrict, size_t, off_t *restrict, size_t *restrict, int *restrict);
	int posix_typed_mem_get_info(int, struct posix_typed_mem_info *);
	int posix_typed_mem_open(const char *, int, int);
	int shm_open(const char *, int, mode_t);
	int shm_unlink(const char *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_SYS_MMAN_H
