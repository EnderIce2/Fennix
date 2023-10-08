/*
	BSD 3-Clause License

	Copyright (c) 2023, EnderIce2
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
		list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __FENNIX_KERNEL_SYSCALLS_LIST_H__
#define __FENNIX_KERNEL_SYSCALLS_LIST_H__

/* mmap */

#define sc_PROT_NONE 0
#define sc_PROT_READ 1
#define sc_PROT_WRITE 2
#define sc_PROT_EXEC 4

#define sc_MAP_SHARED 1
#define sc_MAP_PRIVATE 2
#define sc_MAP_FIXED 4
#define sc_MAP_ANONYMOUS 8

/* lseek */

#define sc_SEEK_SET 0
#define sc_SEEK_CUR 1
#define sc_SEEK_END 2

/**
 * Enumeration of all the native syscalls
 * available in the kernel
 */
typedef enum
{
	/**
	 * This syscall is used to exit the current
	 * process with the provided exit code.
	 *
	 * @fn void exit(int status);
	 * 
	 * @param Code The exit code
	 * @return This syscall does not return
	 *
	 * @note No permissions are required to call
	 * this syscall
	 */
	sc_exit = 0,

	/**
	 * Map pages of memory
	 *
	 * @fn void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
	 */
	sc_mmap,

	/**
	 * Unmap pages of memory
	 *
	 * @fn int munmap(void *addr, size_t len);
	 */
	sc_munmap,

	/**
	 * Change the protection of a page of memory
	 *
	 * @fn int mprotect(void *addr, size_t len, int prot);
	 */
	sc_mprotect,

	/**
	 * Open a file
	 *
	 * @fn int open(const char *path, int oflag, ... );
	 */
	sc_open,

	/**
	 * Close a file descriptor
	 *
	 * @fn int close(int fildes);
	 */
	sc_close,

	/**
	 * Read from a file descriptor
	 *
	 * @fn ssize_t read(int fildes, void *buf, size_t nbyte);
	 */
	sc_read,

	/**
	 * Write to a file descriptor
	 *
	 * @fn ssize_t write(int fildes, const void *buf, size_t nbyte);
	 */
	sc_write,

	/**
	 * Seek to a position in a file descriptor
	 *
	 * @fn off_t lseek(int fildes, off_t offset, int whence);
	 */
	sc_lseek,

	/**
	 * Create a new process
	 * 
	 * @fn pid_t fork(void);
	 */
	sc_fork,

	/** Not a real syscall */
	sc_MaxSyscall
} NativeSyscalls;

#ifndef syscall0
static inline long syscall0(long syscall)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall1
static inline long syscall1(long syscall, long arg1)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall2
static inline long syscall2(long syscall, long arg1, long arg2)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall3
static inline long syscall3(long syscall, long arg1, long arg2, long arg3)
{
	long ret;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall4
static inline long syscall4(long syscall, long arg1, long arg2, long arg3, long arg4)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall5
static inline long syscall5(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	register long r8 __asm__("r8") = arg5;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#ifndef syscall6
static inline long syscall6(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
	long ret;
	register long r10 __asm__("r10") = arg4;
	register long r8 __asm__("r8") = arg5;
	register long r9 __asm__("r9") = arg6;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
						 : "rcx", "r11", "memory");
	return ret;
}
#endif

#endif // !__FENNIX_KERNEL_SYSCALLS_LIST_H__
