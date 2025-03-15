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

#include <sys/mman.h>
#include <bits/libc.h>
#include <errno.h>

export int mlock(const void *, size_t);
export int mlockall(int);

export void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	return (void *)__check_errno((__iptr)sysdep(MemoryMap)(addr, len, prot, flags, fildes, off), (__iptr)MAP_FAILED);
}

export int mprotect(void *addr, size_t len, int prot)
{
	return __check_errno(sysdep(MemoryProtect)(addr, len, prot), -1);
}

export int msync(void *, size_t, int);
export int munlock(const void *, size_t);
export int munlockall(void);

export int munmap(void *addr, size_t len)
{
	return __check_errno(sysdep(MemoryUnmap)(addr, len), -1);
}

export int posix_madvise(void *, size_t, int);
export int posix_mem_offset(const void *restrict, size_t, off_t *restrict, size_t *restrict, int *restrict);
export int posix_typed_mem_get_info(int, struct posix_typed_mem_info *);
export int posix_typed_mem_open(const char *, int, int);
export int shm_open(const char *, int, mode_t);
export int shm_unlink(const char *);
