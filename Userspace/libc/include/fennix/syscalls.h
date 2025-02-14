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

#ifndef __FENNIX_API_SYSCALLS_LIST_H__
#define __FENNIX_API_SYSCALLS_LIST_H__

#pragma region Syscall Wrappers

#define scarg __UINTPTR_TYPE__

/**
 * @brief Syscall wrapper with 0 arguments
 *
 * @details This wrapper is used to call syscalls with 0 arguments
 *
 * @param syscall #syscalls_t
 * @return The return value of the syscall
 */
static inline scarg syscall0(scarg syscall)
{
	scarg ret;
#if defined(__amd64__)
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0");
	__asm__ __volatile__("svc 0"
						 : "=r"(x0)
						 : "r"(x8)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 1 argument
 *
 * @details This wrapper is used to call syscalls with 1 argument
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @return The return value of the syscall
 */
static inline scarg syscall1(scarg syscall, scarg arg1)
{
	scarg ret;
#if defined(__amd64__)
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 2 arguments
 *
 * @details This wrapper is used to call syscalls with 2 arguments
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @param arg2 Argument 2
 * @return The return value of the syscall
 */
static inline scarg syscall2(scarg syscall, scarg arg1, scarg arg2)
{
	scarg ret;
#if defined(__amd64__)
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	register long x1 __asm__("x1") = arg2;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0), "r"(x1)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 3 arguments
 *
 * @details This wrapper is used to call syscalls with 3 arguments
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @param arg2 Argument 2
 * @param arg3 Argument 3
 * @return The return value of the syscall
 */
static inline scarg syscall3(scarg syscall, scarg arg1, scarg arg2, scarg arg3)
{
	scarg ret;
#if defined(__amd64__)
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	register long x1 __asm__("x1") = arg2;
	register long x2 __asm__("x2") = arg3;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0), "r"(x1), "r"(x2)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 4 arguments
 *
 * @details This wrapper is used to call syscalls with 4 arguments
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @param arg2 Argument 2
 * @param arg3 Argument 3
 * @param arg4 Argument 4
 * @return The return value of the syscall
 */
static inline scarg syscall4(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4)
{
	scarg ret;
#if defined(__amd64__)
	register scarg r10 __asm__("r10") = arg4;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	register long x1 __asm__("x1") = arg2;
	register long x2 __asm__("x2") = arg3;
	register long x3 __asm__("x3") = arg4;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 5 arguments
 *
 * @details This wrapper is used to call syscalls with 5 arguments
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @param arg2 Argument 2
 * @param arg3 Argument 3
 * @param arg4 Argument 4
 * @param arg5 Argument 5
 * @return The return value of the syscall
 */
static inline scarg syscall5(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4, scarg arg5)
{
	scarg ret;
#if defined(__amd64__)
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	register long x1 __asm__("x1") = arg2;
	register long x2 __asm__("x2") = arg3;
	register long x3 __asm__("x3") = arg4;
	register long x4 __asm__("x4") = arg5;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

/**
 * @brief Syscall wrapper with 6 arguments
 *
 * @details This wrapper is used to call syscalls with 6 arguments
 *
 * @param syscall #syscalls_t
 * @param arg1 Argument 1
 * @param arg2 Argument 2
 * @param arg3 Argument 3
 * @param arg4 Argument 4
 * @param arg5 Argument 5
 * @param arg6 Argument 6
 * @return The return value of the syscall
 */
static inline scarg syscall6(scarg syscall, scarg arg1, scarg arg2, scarg arg3, scarg arg4, scarg arg5, scarg arg6)
{
	scarg ret;
#if defined(__amd64__)
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	register scarg r9 __asm__("r9") = arg6;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
						 : "rcx", "r11", "memory");
#elif defined(__i386__)
#warning "i386 syscall wrapper not implemented"
#elif defined(__arm__)
#warning "arm syscall wrapper not implemented"
#elif defined(__aarch64__)
	register long x8 __asm__("x8") = syscall;
	register long x0 __asm__("x0") = arg1;
	register long x1 __asm__("x1") = arg2;
	register long x2 __asm__("x2") = arg3;
	register long x3 __asm__("x3") = arg4;
	register long x4 __asm__("x4") = arg5;
	register long x5 __asm__("x5") = arg6;
	__asm__ __volatile__("svc 0"
						 : "=r"(ret)
						 : "r"(x8), "0"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
						 : "memory", "cc");
#else
#error "Unsupported architecture"
#endif
	return ret;
}

#pragma endregion Syscall Wrappers

/**
 * @brief NULL pointer
 *
 * This is a pointer to address 0, which is reserved and cannot be dereferenced.
 *
 * @note This macro is defined only for this documentation.
 */
#define __SYS_NULL ((void *)0)

typedef enum
{
	__SYS_PROT_READ = 0x1,
	__SYS_PROT_WRITE = 0x2,
	__SYS_PROT_EXEC = 0x4,
	__SYS_PROT_NONE = 0x0,

	__SYS_MAP_SHARED = 0x1,
	__SYS_MAP_PRIVATE = 0x2,
	__SYS_MAP_FIXED = 0x4,
	__SYS_MAP_ANONYMOUS = 0x8,
	__SYS_MAP_ANON = __SYS_MAP_ANONYMOUS
} syscall_mmap_flags_t;

typedef enum
{
	__SYS_O_RDONLY = 0x1,
	__SYS_O_WRONLY = 0x2,
	__SYS_O_RDWR = 0x3,
	__SYS_O_APPEND = 0x4,
	__SYS_O_CREAT = 0x8,
	__SYS_O_DSYNC = 0x10,
	__SYS_O_EXCL = 0x20,
	__SYS_O_NOCTTY = 0x40,
	__SYS_O_NONBLOCK = 0x80,
	__SYS_O_RSYNC = 0x100,
	__SYS_O_SYNC = 0x200,
	__SYS_O_TRUNC = 0x400
} syscall_open_flags_t;

typedef enum
{
	__SYS_F_OK = 0,
	__SYS_R_OK = 1,
	__SYS_W_OK = 2,
	__SYS_X_OK = 3
} syscall_access_flags_t;

typedef enum
{
	__SYS_GET_GS = 0,
	__SYS_SET_GS = 1,
	__SYS_GET_FS = 2,
	__SYS_SET_FS = 3,
} syscall_prctl_options_t;
#ifdef __kernel__
typedef syscall_prctl_options_t prctl_options_t;
#endif

typedef enum
{
	__SYS_SEEK_SET = 0,
	__SYS_SEEK_CUR = 1,
	__SYS_SEEK_END = 2
} syscall_seek_whence_t;

typedef enum
{
	__SYS_SIGNULL = 0,
	/* Process abort signal. */
	__SYS_SIGABRT = 1,
	/* Alarm clock. */
	__SYS_SIGALRM = 2,
	/* Access to an undefined portion of a memory object. */
	__SYS_SIGBUS = 3,
	/* Child process terminated, stopped, or continued. */
	__SYS_SIGCHLD = 4,
	/* Continue executing, if stopped. */
	__SYS_SIGCONT = 5,
	/* Erroneous arithmetic operation. */
	__SYS_SIGFPE = 6,
	/* Hangup. */
	__SYS_SIGHUP = 7,
	/* Illegal instruction. */
	__SYS_SIGILL = 8,
	/* Terminal interrupt signal. */
	__SYS_SIGINT = 9,
	/* Kill (cannot be caught or ignored). */
	__SYS_SIGKILL = 10,
	/* Write on a pipe with no one to read it. */
	__SYS_SIGPIPE = 11,
	/* Terminal quit signal. */
	__SYS_SIGQUIT = 12,
	/* Invalid memory reference. */
	__SYS_SIGSEGV = 13,
	/* Stop executing (cannot be caught or ignored). */
	__SYS_SIGSTOP = 14,
	/* Termination signal. */
	__SYS_SIGTERM = 15,
	/* Terminal stop signal. */
	__SYS_SIGTSTP = 16,
	/* Background process attempting read. */
	__SYS_SIGTTIN = 17,
	/* Background process attempting write. */
	__SYS_SIGTTOU = 18,
	/* User-defined signal 1. */
	__SYS_SIGUSR1 = 19,
	/* User-defined signal 2. */
	__SYS_SIGUSR2 = 20,
	/* Pollable event. */
	__SYS_SIGPOLL = 21,
	/* Profiling timer expired. */
	__SYS_SIGPROF = 22,
	/* Bad system call. */
	__SYS_SIGSYS = 23,
	/* Trace/breakpoint trap. */
	__SYS_SIGTRAP = 24,
	/* High bandwidth data is available at a socket. */
	__SYS_SIGURG = 25,
	/* Virtual timer expired. */
	__SYS_SIGVTALRM = 26,
	/* CPU time limit exceeded. */
	__SYS_SIGXCPU = 27,
	/* File size limit exceeded. */
	__SYS_SIGXFSZ = 28,

	/**
	 * Reserved
	 * These are just to match Linux's signal numbers.
	 */
	__SYS_SIGCOMP1 = 29,
	__SYS_SIGCOMP2 = 30,
	__SYS_SIGCOMP3 = 31,

	/* Real-time signals. */
	__SYS_SIGRTMIN = 32,
	__SYS_SIGRT_1 = 33,
	__SYS_SIGRT_2 = 34,
	__SYS_SIGRT_3 = 35,
	__SYS_SIGRT_4 = 36,
	__SYS_SIGRT_5 = 37,
	__SYS_SIGRT_6 = 38,
	__SYS_SIGRT_7 = 39,
	__SYS_SIGRT_8 = 40,
	__SYS_SIGRT_9 = 41,
	__SYS_SIGRT_10 = 42,
	__SYS_SIGRT_11 = 43,
	__SYS_SIGRT_12 = 44,
	__SYS_SIGRT_13 = 45,
	__SYS_SIGRT_14 = 46,
	__SYS_SIGRT_15 = 47,
	__SYS_SIGRT_16 = 48,
	__SYS_SIGRT_17 = 49,
	__SYS_SIGRT_18 = 50,
	__SYS_SIGRT_19 = 51,
	__SYS_SIGRT_20 = 52,
	__SYS_SIGRT_21 = 53,
	__SYS_SIGRT_22 = 54,
	__SYS_SIGRT_23 = 55,
	__SYS_SIGRT_24 = 56,
	__SYS_SIGRT_25 = 57,
	__SYS_SIGRT_26 = 58,
	__SYS_SIGRT_27 = 59,
	__SYS_SIGRT_28 = 60,
	__SYS_SIGRT_29 = 61,
	__SYS_SIGRT_30 = 62,
	__SYS_SIGRT_31 = 63,
	__SYS_SIGRTMAX = 64,

	/* Maximum signal number. */
	__SYS_SIGNAL_MAX = __SYS_SIGRTMAX
} syscall_signal_t;
#ifdef __kernel__
typedef syscall_signal_t signal_t;
#endif

typedef enum
{
	/** Terminate the process. */
	__SYS_SIG_TERM = 0,
	/** Ignore the signal. */
	__SYS_SIG_IGN = 1,
	/** Continue the process. */
	__SYS_SIG_CONT = 2,
	/** Stop the process. */
	__SYS_SIG_STOP = 3,
	/** Dump core. */
	__SYS_SIG_CORE = 4
} syscall_signal_disposition_t;
#ifdef __kernel__
typedef syscall_signal_disposition_t signal_disposition_t;
#endif

typedef enum
{
	__SYS_SIG_BLOCK = 0,
	__SYS_SIG_UNBLOCK = 1,
	__SYS_SIG_SETMASK = 2
} syscall_signal_actions_t;

typedef enum
{
	__SYS_SA_NOCLDSTOP = 1,
	__SYS_SA_ONSTACK = 0x08000000,
	__SYS_SA_RESETHAND = 0x80000000,
	__SYS_SA_RESTART = 0x10000000,
	__SYS_SA_SIGINFO = 4,
	__SYS_SA_NOCLDWAIT = 2,
	__SYS_SA_NODEFER = 0x40000000,
} syscall_signal_action_flags_t;

typedef enum
{
	__SYS_SIG_ERR = -1,
	__SYS_SIG_DFL = 0,
	___SYS_SIG_IGN = 1
} syscall_signal_action_disposition_t;

typedef enum
{
	__SYS_CLOCK_MONOTONIC = 1,
	__SYS_CLOCK_PROCESS_CPUTIME_ID = 2,
	__SYS_CLOCK_REALTIME = 3,
	__SYS_CLOCK_THREAD_CPUTIME_ID = 4,
} syscall_clockid_t;

#ifndef __cplusplus
_Static_assert((int)__SYS_SIG_IGN == (int)___SYS_SIG_IGN, "SIG_IGN values do not match");
#else
static_assert((int)__SYS_SIG_IGN == (int)___SYS_SIG_IGN, "SIG_IGN values do not match");
#endif

typedef int __SYS_clockid_t;
typedef unsigned int __SYS_socklen_t;

typedef struct FramebufferScreenInfo
{
	__UINT32_TYPE__ Width;
	__UINT32_TYPE__ Height;
	__UINT32_TYPE__ Pitch;
	__UINT32_TYPE__ Bpp;
	__UINT32_TYPE__ Size;
} FramebufferScreenInfo;

#define FBIOGET_SCREEN_INFO 0xf0

/**
 * @brief List of syscalls
 *
 * @details This list contains all the syscalls of the Fennix Kernel API.
 *
 */
typedef enum
{
	/* Initialization */

	/**
	 * @brief Set syscall version
	 *
	 * @code
	 * int api_version(int version);
	 * @endcode
	 *
	 * @details This syscall is used to set the version of the list.
	 * To prevent applications from breaking on major changes, this should
	 * be called at the very beginning of the program.
	 *
	 * @param version The version of the syscall list of which the program
	 *                was compiled with
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if the requested version is invalid
	 *
	 * @note If this syscall is not used, the latest version will be used.
	 */
	SYS_API_VERSION = 0,

	/* I/O */

	/**
	 * @brief Read from a file descriptor
	 *
	 * @code
	 * ssize_t sys_read(int fildes, void *buf, size_t nbyte);
	 * @endcode
	 *
	 * @details Reads up to `count` bytes from the file descriptor `fd` into
	 * the buffer starting at `buf`.
	 *
	 * @param fd File descriptor to read from
	 * @param buf Buffer where data will be stored
	 * @param count Maximum number of bytes to read
	 *
	 * @return
	 * - Number of bytes read on success
	 * - 0 if the end of file is reached
	 * - #EFAULT if the buffer is outside accessible address space
	 * - #EBADF if `fd` is not a valid file descriptor
	 */
	SYS_READ = 100,
	/**
	 * @brief Read from a file descriptor
	 *
	 * @code
	 * ssize_t sys_pread(int fildes, void *buf, size_t nbyte, off_t offset);
	 * @endcode
	 *
	 * @details Reads up to `count` bytes from the file descriptor `fd` into
	 * the buffer starting at `buf`.
	 *
	 * @param fd File descriptor to read from
	 * @param buf Buffer where data will be stored
	 * @param count Maximum number of bytes to read
	 * @param offset Offset in the file
	 *
	 * @return
	 * - Number of bytes read on success
	 * - 0 if the end of file is reached
	 * - #EFAULT if the buffer is outside accessible address space
	 * - #EBADF if `fd` is not a valid file descriptor
	 */
	SYS_PREAD,
	/**
	 * @brief Write to a file descriptor
	 *
	 * @code
	 * ssize_t sys_write(int fildes, const void *buf, size_t nbyte);
	 * @endcode
	 *
	 * @details Writes up to `count` bytes from the buffer starting at `buf`
	 * to the file descriptor `fd`.
	 *
	 * @param fd File descriptor to write to
	 * @param buf Buffer containing data to write
	 * @param count Number of bytes to write
	 *
	 * @return
	 * - Number of bytes written on success
	 * - #EFAULT if the buffer is outside accessible address space
	 * - #EBADF if `fd` is not a valid file descriptor
	 * - #EPIPE if writing to a pipe with no reader
	 */
	SYS_WRITE,
	/**
	 * @brief Write to a file descriptor
	 *
	 * @code
	 * ssize_t sys_pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
	 * @endcode
	 *
	 * @details Writes up to `count` bytes from the buffer starting at `buf`
	 * to the file descriptor `fd`.
	 *
	 * @param fd File descriptor to write to
	 * @param buf Buffer containing data to write
	 * @param count Number of bytes to write
	 * @param offset Offset in the file
	 *
	 * @return
	 * - Number of bytes written on success
	 * - #EFAULT if the buffer is outside accessible address space
	 * - #EBADF if `fd` is not a valid file descriptor
	 */
	SYS_PWRITE,
	/**
	 * @brief Open a file
	 *
	 * @code
	 * int open(const char *pathname, int flags, mode_t mode);
	 * @endcode
	 *
	 * @details Opens the file specified by `pathname`.
	 *
	 * @param pathname Path to the file
	 * @param flags Flags for file access mode\n
	 *              Supported values:
	 *              - #__SYS_O_RDONLY: Open file for reading only.
	 *              - #__SYS_O_WRONLY: Open file for writing only.
	 *              - #__SYS_O_RDWR: Open file for reading and writing.
	 *              - #__SYS_O_APPEND: Append data to the end of file.
	 *              - #__SYS_O_CREAT: Create file if it does not exist.
	 *              - #__SYS_O_DSYNC:
	 *              - #__SYS_O_EXCL:
	 *              - #__SYS_O_NOCTTY:
	 *              - #__SYS_O_NONBLOCK:
	 *              - #__SYS_O_RSYNC:
	 *              - #__SYS_O_SYNC:
	 *              - #__SYS_O_TRUNC: Truncate file to zero length.
	 * @param mode Permissions for newly created file (if applicable)
	 *
	 * @return
	 * - File descriptor on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permissions are insufficient
	 *
	 * @see #syscall_open_flags_t
	 */
	SYS_OPEN,
	/**
	 * @brief Close a file descriptor
	 *
	 * @code
	 * int close(int fd);
	 * @endcode
	 *
	 * @details Closes the file descriptor `fd`, releasing its resources.
	 *
	 * @param fd File descriptor to close
	 *
	 * @return
	 * - #EOK on success
	 * - #EBADF if `fd` is not a valid file descriptor
	 */
	SYS_CLOSE,
	/**
	 * @brief Control a device
	 *
	 * @code
	 * int ioctl(int fd, unsigned long request, void *argp);
	 * @endcode
	 *
	 * @details Manipulates the underlying parameters of a device.
	 *
	 * @param fd File descriptor referring to the device
	 * @param request Device-specific request code
	 * @param argp Argument for the request
	 *
	 * @return
	 * - #EOK on success
	 * - #EBADF if `fd` is not valid
	 * - #EINVAL if the request is invalid
	 */
	SYS_IOCTL,

	/* File Status */

	/**
	 * @brief Retrieve file status
	 *
	 * @code
	 * int stat(const char *pathname, struct stat *statbuf);
	 * @endcode
	 *
	 * @details Gets the status of the file specified by `pathname`.
	 *
	 * @param pathname Path to the file
	 * @param statbuf Buffer to store file status
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permissions are insufficient
	 */
	SYS_STAT = 200,
	/**
	 * @brief Retrieve file status for an open file descriptor
	 *
	 * @code
	 * int fstat(int fd, struct stat *statbuf);
	 * @endcode
	 *
	 * @details Gets the status of the file referred to by `fd`.
	 *
	 * @param fd File descriptor
	 * @param statbuf Buffer to store file status
	 *
	 * @return
	 * - #EOK on success
	 * - #EBADF if `fd` is not a valid file descriptor
	 * - #EFAULT if `statbuf` is outside accessible address space
	 */
	SYS_FSTAT,
	/**
	 * @brief Retrieve file status with symbolic link resolution
	 *
	 * @code
	 * int lstat(const char *pathname, struct stat *statbuf);
	 * @endcode
	 *
	 * @details Gets the status of the file specified by `pathname`,
	 * but does not follow symbolic links.
	 *
	 * @param pathname Path to the file
	 * @param statbuf Buffer to store file status
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permissions are insufficient
	 */
	SYS_LSTAT,
	/**
	 * @brief Check a file's accessibility
	 *
	 * @code
	 * int access(const char *pathname, int mode);
	 * @endcode
	 *
	 * @details Checks if the calling process can access the file specified
	 * by `pathname` according to the specified `mode`.
	 *
	 * @param pathname Path to the file
	 * @param mode Accessibility check mode\n
	 *             Supported values:
	 *             - #__SYS_F_OK: Check if the file exists
	 *             - #__SYS_R_OK: Check if the file is readable
	 *             - #__SYS_W_OK: Check if the file is writable
	 *             - #__SYS_X_OK: Check if the file is executable
	 *
	 * @return
	 * - #EOK on success
	 * - #EACCES if access is denied
	 * - #ENOENT if the file does not exist
	 *
	 * @see #syscall_access_flags_t
	 */
	SYS_ACCESS,
	/**
	 * @brief Change the size of a file
	 *
	 * @code
	 * int truncate(const char *pathname, off_t length);
	 * @endcode
	 *
	 * @details Sets the size of the file specified by `pathname` to `length`.
	 * If the file is shorter, it is extended and the extended part is zero-filled.
	 *
	 * @param pathname Path to the file
	 * @param length Desired file length
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if `length` is invalid
	 * - #EACCES if permissions are insufficient
	 */
	SYS_TRUNCATE,
	/**
	 * @brief Change the size of a file referred by a file descriptor
	 *
	 * @code
	 * int ftruncate(int fd, off_t length);
	 * @endcode
	 *
	 * @details Sets the size of the file referred to by `fd` to `length`.
	 *
	 * @param fd File descriptor
	 * @param length Desired file length
	 *
	 * @return
	 * - #EOK on success
	 * - #EBADF if `fd` is not valid
	 * - #EINVAL if `length` is invalid
	 */
	SYS_FTRUNCATE,
	/**
	 * @brief Get the current file offset
	 *
	 * @code
	 * off_t tell(int fd);
	 * @endcode
	 *
	 * @details Returns the current file offset for the file referred to by `fd`.
	 *
	 * @param fd File descriptor
	 *
	 * @return
	 * - Current file offset on success
	 * - #EBADF if `fd` is not a valid file descriptor
	 */
	SYS_TELL,
	/**
	 * @brief Set the file offset
	 *
	 * @code
	 * off_t seek(int fd, off_t offset, int whence);
	 * @endcode
	 *
	 * @details Sets the file offset for the file referred to by `fd` to the
	 * specified `offset` according to the directive `whence`.
	 *
	 * @param fd File descriptor
	 * @param offset Offset to set
	 * @param whence Directive for setting the offset\n
	 *               Supported values:
	 *               - #__SYS_SEEK_SET: Set the offset to `offset` bytes
	 *               - #__SYS_SEEK_CUR: Set the offset to the current offset plus `offset`
	 *               - #__SYS_SEEK_END: Set the offset to the size of the file plus `offset`
	 *
	 * @return
	 * - New file offset on success
	 * - #EBADF if `fd` is not a valid file descriptor
	 * - #EINVAL if `whence` is invalid
	 */
	SYS_SEEK,

	/* Process Control */

	/**
	 * @brief Terminate the calling process
	 *
	 * @code
	 * void exit(int status);
	 * @endcode
	 *
	 * @details Terminates the calling process with the specified `status`.
	 * The status code is made available to the parent process.
	 *
	 * @param status Exit status code
	 *
	 * @return This function does not return.
	 */
	SYS_EXIT = 300,
	/**
	 * @brief Create a child process
	 *
	 * @code
	 * pid_t fork(void);
	 * @endcode
	 *
	 * @details Creates a new process by duplicating the calling process.
	 * The child process has its own copy of the parent's address space.
	 *
	 * @return
	 * - 0 to the child process
	 * - PID of the child to the parent process
	 * - #ENOMEM if memory is insufficient
	 */
	SYS_FORK,
	/**
	 * @brief Execute a program
	 *
	 * @code
	 * int execve(const char *pathname, char *const argv[], char *const envp[]);
	 * @endcode
	 *
	 * @details Replaces the current process image with a new process image
	 * specified by `pathname`.
	 *
	 * @param pathname Path to the executable file
	 * @param argv Argument vector
	 * @param envp Environment variables
	 *
	 * @return
	 * - Does not return on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permissions are insufficient
	 */
	SYS_EXECVE,
	/**
	 * @brief Get the process ID of the calling process
	 *
	 * @code
	 * pid_t getpid(void);
	 * @endcode
	 *
	 * @details Returns the process ID of the calling process.
	 *
	 * @return
	 * - Process ID on success
	 */
	SYS_GETPID,
	/**
	 * @brief Get the parent process ID
	 *
	 * @code
	 * pid_t getppid(void);
	 * @endcode
	 *
	 * @details Returns the parent process ID of the calling process.
	 *
	 * @return
	 * - Parent process ID on success
	 */
	SYS_GETPPID,
	/**
	 * @brief Wait for a child process to change state
	 *
	 * @code
	 * pid_t waitpid(pid_t pid, int *wstatus, int options);
	 * @endcode
	 *
	 * @details Waits for the child process specified by `pid` to change state.
	 *
	 * @param pid Process ID to wait for
	 * @param wstatus Pointer to store the status information
	 * @param options Options for waiting behavior
	 *
	 * @return
	 * - Process ID of the child on success
	 * - #ECHILD if no child processes exist
	 */
	SYS_WAITPID,
	/**
	 * @brief Send a signal to a process
	 *
	 * @code
	 * int kill(pid_t pid, int sig);
	 * @endcode
	 *
	 * @details Sends the signal `sig` to the process specified by `pid`.
	 *
	 * @param pid Process ID
	 * @param sig Signal to send
	 *
	 * @return
	 * - #EOK on success
	 * - #ESRCH if the process does not exist
	 * - #EINVAL if `sig` is invalid
	 */
	SYS_KILL,
	/**
	 * @brief Process/Thread Control
	 *
	 * @code
	 * int prctl(syscall_prctl_options_t option, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);
	 * @endcode
	 *
	 * @details Perform various operations on a process or thread.
	 *
	 * @param option Operation to perform
	 * @param arg1 Argument 1
	 * @param arg2 Argument 2
	 * @param arg3 Argument 3
	 * @param arg4 Argument 4
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if the operation is invalid
	 * - #EFAULT if one of the arguments is invalid
	 */
	SYS_PRCTL,

	/* Memory */

	/**
	 * @brief Set the program break
	 *
	 * @code
	 * int brk(void *end_data);
	 * @endcode
	 *
	 * @details Increases or decreases the program’s data space, ending at `end_data`.
	 *
	 * @param end_data New program break location
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOMEM if memory allocation fails
	 */
	SYS_BRK = 400,
	/**
	 * @brief Map files or devices into memory
	 *
	 * @code
	 * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
	 * @endcode
	 *
	 * @details Maps a file or device into memory. This can be used for memory-mapped I/O or
	 * for sharing memory between processes.
	 *
	 * @param addr Desired starting address of the mapping (NULL for automatic allocation)
	 * @param length Length of the mapping
	 * @param prot Desired memory protection\n
	 *             Supported values:
	 *             - #__SYS_PROT_READ: Readable
	 *             - #__SYS_PROT_WRITE: Writable
	 *             - #__SYS_PROT_EXEC: Executable
	 *             - #__SYS_PROT_NONE: No access
	 * @param flags Mapping options\n
	 *             Supported values:
	 *             - #__SYS_MAP_SHARED: Share memory with other processes
	 *             - #__SYS_MAP_PRIVATE: Create a private copy of the file
	 *             - #__SYS_MAP_FIXED: Use `addr` as the starting address of the mapping
	 *             - #__SYS_MAP_ANONYMOUS: Create an anonymous mapping
	 * @param fd File descriptor for the file to map
	 * @param offset Offset in the file to start the mapping
	 *
	 * @return There are several possible return values:
	 *         - Pointer to mapped area on success
	 *         - #EACCES
	 *         - #EAGAIN
	 *         - #EBADF
	 *         - #EINVAL
	 *         - #EMFILE
	 *         - #ENODEV
	 *         - #ENOMEM
	 *         - #ENOTSUP
	 *         - #ENXIO
	 *         - #EOVERFLOW
	 *
	 * @see #syscall_mmap_flags_t
	 */
	SYS_MMAP,
	/**
	 * @brief Unmap a mapped memory region
	 *
	 * @code
	 * int munmap(void *addr, size_t length);
	 * @endcode
	 *
	 * @details Unmaps a previously mapped memory region, making the memory available for reuse.
	 *
	 * @param addr Start address of the memory region
	 * @param length Length of the memory region to unmap
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if the address or length is invalid
	 * - #EFAULT if the memory region is not currently mapped
	 */
	SYS_MUNMAP,
	/**
	 * @brief Change memory protection
	 *
	 * @code
	 * int mprotect(void *addr, size_t length, int prot);
	 * @endcode
	 *
	 * @details Sets the protection on the memory region starting at `addr` for `length`.
	 *
	 * @param addr Start address of the memory region
	 * @param length Length of the memory region
	 * @param prot Desired memory protection (e.g., PROT_READ, PROT_WRITE)
	 *
	 * @return
	 * - #EOK on success
	 * - #EACCES if protection cannot be set
	 */
	SYS_MPROTECT,
	/**
	 * @brief Provide advice about memory usage
	 *
	 * @code
	 * int madvise(void *addr, size_t length, int advice);
	 * @endcode
	 *
	 * @details Provides advice to the kernel about the expected behavior of the memory region
	 * starting at `addr` for `length`, such as whether it will be accessed randomly or sequentially.
	 *
	 * @param addr Start address of the memory region
	 * @param length Length of the memory region
	 * @param advice Desired advice (e.g., MADV_DONTNEED, MADV_SEQUENTIAL)
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if the parameters are invalid
	 */
	SYS_MADVISE,

	/* Communication */

	/**
	 * @brief Create a pipe
	 *
	 * @code
	 * int pipe(int pipefd[2]);
	 * @endcode
	 *
	 * @details Creates a pipe, returning two file descriptors in `pipefd`. One is for reading,
	 * and the other is for writing.
	 *
	 * @param pipefd Array to store the two file descriptors
	 *
	 * @return
	 * - #EOK on success
	 * - #EMFILE if the process has too many open file descriptors
	 */
	SYS_PIPE = 500,
	/**
	 * @brief Duplicate a file descriptor
	 *
	 * @code
	 * int dup(int oldfd);
	 * @endcode
	 *
	 * @details Duplicates the file descriptor `oldfd`, returning the new file descriptor.
	 *
	 * @param oldfd File descriptor to duplicate
	 *
	 * @return
	 * - New file descriptor on success
	 * - #EBADF if `oldfd` is invalid
	 */
	SYS_DUP,
	/**
	 * @brief Duplicate a file descriptor to a specific value
	 *
	 * @code
	 * int dup2(int oldfd, int newfd);
	 * @endcode
	 *
	 * @details Duplicates `oldfd` to `newfd`. If `newfd` is already open, it will be closed first.
	 *
	 * @param oldfd File descriptor to duplicate
	 * @param newfd File descriptor to duplicate `oldfd` to
	 *
	 * @return
	 * - `newfd` on success
	 * - #EBADF if `oldfd` is invalid
	 * - #EINVAL if `newfd` is invalid
	 */
	SYS_DUP2,
	/**
	 * @brief Create an endpoint for communication
	 *
	 * @code
	 * int socket(int domain, int type, int protocol);
	 * @endcode
	 *
	 * @details Creates an endpoint for communication, returning a socket file descriptor.
	 *
	 * @param domain Communication domain (e.g., AF_INET for IPv4)
	 * @param type Type of socket (e.g., SOCK_STREAM for TCP)
	 * @param protocol Protocol to use (e.g., IPPROTO_TCP)
	 *
	 * @return
	 * - Socket file descriptor on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_SOCKET,
	/**
	 * @brief Bind a socket to a local address
	 *
	 * @code
	 * int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	 * @endcode
	 *
	 * @details Binds a socket to a local address so it can listen for incoming connections.
	 *
	 * @param sockfd Socket file descriptor
	 * @param addr Address to bind to
	 * @param addrlen Length of the address
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if the socket is invalid
	 */
	SYS_BIND,
	/**
	 * @brief Connect to a remote address
	 *
	 * @code
	 * int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	 * @endcode
	 *
	 * @details Connects a socket to a remote address.
	 *
	 * @param sockfd Socket file descriptor
	 * @param addr Remote address to connect to
	 * @param addrlen Length of the address
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_CONNECT,
	/**
	 * @brief Listen for incoming connections on a socket
	 *
	 * @code
	 * int listen(int sockfd, int backlog);
	 * @endcode
	 *
	 * @details Sets a socket to listen for incoming connections, specifying the backlog queue size.
	 *
	 * @param sockfd Socket file descriptor
	 * @param backlog Number of pending connections to allow
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_LISTEN,
	/**
	 * @brief Accept an incoming connection on a socket
	 *
	 * @code
	 * int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	 * @endcode
	 *
	 * @details Accepts an incoming connection on a listening socket, creating a new socket for communication.
	 *
	 * @param sockfd Socket file descriptor
	 * @param addr Client address
	 * @param addrlen Length of the address
	 *
	 * @return
	 * - New socket file descriptor on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_ACCEPT,
	/**
	 * @brief Send data on a socket
	 *
	 * @code
	 * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
	 * @endcode
	 *
	 * @details Sends data through a socket.
	 *
	 * @param sockfd Socket file descriptor
	 * @param buf Data to send
	 * @param len Length of the data
	 * @param flags Flags for the send operation
	 *
	 * @return
	 * - Number of bytes sent on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_SEND,
	/**
	 * @brief Receive data on a socket
	 *
	 * @code
	 * ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	 * @endcode
	 *
	 * @details Receives data from a socket.
	 *
	 * @param sockfd Socket file descriptor
	 * @param buf Buffer to store received data
	 * @param len Maximum number of bytes to receive
	 * @param flags Flags for the receive operation
	 *
	 * @return
	 * - Number of bytes received on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_RECV,
	/**
	 * @brief Shut down part of a full-duplex connection
	 *
	 * @code
	 * int shutdown(int sockfd, int how);
	 * @endcode
	 *
	 * @details Shuts down part of a full-duplex connection on a socket.
	 *
	 * @param sockfd Socket file descriptor
	 * @param how Determines which operations to shut down (e.g., SHUT_RD, SHUT_WR)
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_SHUTDOWN,

	/* Time */

	/**
	 * @brief Get the current time
	 *
	 * @code
	 * time_t time(time_t *t);
	 * @endcode
	 *
	 * @details Retrieves the current calendar time as the number of seconds since the epoch.
	 *
	 * @param t Pointer to store the time (optional)
	 *
	 * @return
	 * - Current time in seconds on success
	 * - #__SYS_NULL if `t` is NULL
	 */
	SYS_TIME = 600,
	/**
	 * @brief Get the current time of a specific clock
	 *
	 * @code
	 * int clock_gettime(clockid_t clockid, struct timespec *tp);
	 * @endcode
	 *
	 * @details Retrieves the current time for the specified clock (`CLOCK_REALTIME`, `CLOCK_MONOTONIC`, etc.).
	 *
	 * @param clockid Clock ID to query
	 * @param tp Pointer to store the time
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_CLOCK_GETTIME,
	/**
	 * @brief Set the current time of a specific clock
	 *
	 * @code
	 * int clock_settime(clockid_t clockid, const struct timespec *tp);
	 * @endcode
	 *
	 * @details Sets the time for the specified clock (`CLOCK_REALTIME`, `CLOCK_MONOTONIC`, etc.).
	 *
	 * @param clockid Clock ID to set
	 * @param tp Pointer to the time value
	 *
	 * @return
	 * - #EOK on success
	 * - #EINVAL if parameters are invalid
	 */
	SYS_CLOCK_SETTIME,
	/**
	 * @brief Sleep for a specified time
	 *
	 * @code
	 * int nanosleep(const struct timespec *req, struct timespec *rem);
	 * @endcode
	 *
	 * @details Suspends the execution of the calling thread for the specified time duration.
	 *
	 * @param req Pointer to `timespec` specifying the time to sleep
	 * @param rem Pointer to store remaining time if interrupted
	 *
	 * @return
	 * - #EOK on success
	 * - #EINTR if interrupted by a signal
	 */
	SYS_NANOSLEEP,

	/* Miscellaneous */

	/**
	 * @brief Get the current working directory
	 *
	 * @code
	 * char *getcwd(char *buf, size_t size);
	 * @endcode
	 *
	 * @details Retrieves the current working directory.
	 *
	 * @param buf Buffer to store the directory path
	 * @param size Size of the buffer
	 *
	 * @return
	 * - Pointer to `buf` on success
	 * - #__SYS_NULL on error
	 */
	SYS_GETCWD = 700,
	/**
	 * @brief Change the current working directory
	 *
	 * @code
	 * int chdir(const char *path);
	 * @endcode
	 *
	 * @details Changes the current working directory to the specified `path`.
	 *
	 * @param path New directory path
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOENT if the directory does not exist
	 * - #EACCES if permission is denied
	 */
	SYS_CHDIR,
	/**
	 * @brief Create a new directory
	 *
	 * @code
	 * int mkdir(const char *path, mode_t mode);
	 * @endcode
	 *
	 * @details Creates a new directory at `path` with the specified permissions.
	 *
	 * @param path Path to the new directory
	 * @param mode Directory permissions
	 *
	 * @return
	 * - #EOK on success
	 * - #EEXIST if the directory already exists
	 * - #EACCES if permission is denied
	 */
	SYS_MKDIR,
	/**
	 * @brief Remove an empty directory
	 *
	 * @code
	 * int rmdir(const char *path);
	 * @endcode
	 *
	 * @details Removes the empty directory specified by `path`.
	 *
	 * @param path Path to the directory
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOTEMPTY if the directory is not empty
	 */
	SYS_RMDIR,
	/**
	 * @brief Remove a file
	 *
	 * @code
	 * int unlink(const char *pathname);
	 * @endcode
	 *
	 * @details Removes the file specified by `pathname`.
	 *
	 * @param pathname Path to the file
	 *
	 * @return
	 * - #EOK on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permission is denied
	 */
	SYS_UNLINK,
	/**
	 * @brief Rename a file or directory
	 *
	 * @code
	 * int rename(const char *oldpath, const char *newpath);
	 * @endcode
	 *
	 * @details Renames a file or directory from `oldpath` to `newpath`.
	 *
	 * @param oldpath Current name of the file or directory
	 * @param newpath New name of the file or directory
	 *
	 * @return
	 * - #EOK on success
	 * - #EEXIST if the target exists
	 * - #EACCES if permission is denied
	 */
	SYS_RENAME,

	/**
	 * @brief Max number of syscalls
	 *
	 * @details This is used to determine the size of the `syscalls_t` array.
	 *
	 * @code
	 * syscalls_t syscalls[SYS_MAX];
	 * @endcode
	 *
	 * @note This must be the last element in the list
	 */
	SYS_MAX
} syscalls_t;

/* Initialization */

/** @copydoc SYS_API_VERSION */
#define call_api_version(version) syscall1(SYS_API_VERSION, (scarg)version)

/* I/O */

/** @copydoc SYS_READ */
#define call_read(fd, buf, count) syscall3(SYS_READ, (scarg)fd, (scarg)buf, (scarg)count)

/** @copydoc SYS_PREAD */
#define call_pread(fd, buf, count, offset) syscall4(SYS_PREAD, (scarg)fd, (scarg)buf, (scarg)count, (scarg)offset)

/** @copydoc SYS_WRITE */
#define call_write(fd, buf, count) syscall3(SYS_WRITE, (scarg)fd, (scarg)buf, (scarg)count)

/** @copydoc SYS_PWRITE */
#define call_pwrite(fd, buf, count, offset) syscall4(SYS_PWRITE, (scarg)fd, (scarg)buf, (scarg)count, (scarg)offset)

/** @copydoc SYS_OPEN */
#define call_open(pathname, flags, mode) syscall3(SYS_OPEN, (scarg)pathname, (scarg)flags, (scarg)mode)

/** @copydoc SYS_CLOSE */
#define call_close(fd) syscall1(SYS_CLOSE, fd)

/** @copydoc SYS_IOCTL */
#define call_ioctl(fd, request, argp) syscall3(SYS_IOCTL, (scarg)fd, (scarg)request, (scarg)argp)

/* File Status */

/** @copydoc SYS_STAT */
#define call_stat(pathname, statbuf) syscall2(SYS_STAT, (scarg)pathname, (scarg)statbuf)

/** @copydoc SYS_FSTAT */
#define call_fstat(fd, statbuf) syscall2(SYS_FSTAT, (scarg)fd, (scarg)statbuf)

/** @copydoc SYS_LSTAT */
#define call_lstat(pathname, statbuf) syscall2(SYS_LSTAT, (scarg)pathname, (scarg)statbuf)

/** @copydoc SYS_ACCESS */
#define call_access(pathname, mode) syscall2(SYS_ACCESS, (scarg)pathname, (scarg)mode)

/** @copydoc SYS_TRUNCATE */
#define call_truncate(pathname, length) syscall2(SYS_TRUNCATE, (scarg)pathname, (scarg)length)

/** @copydoc SYS_FTRUNCATE */
#define call_ftruncate(fd, length) syscall2(SYS_FTRUNCATE, (scarg)fd, (scarg)length)

/** @copydoc SYS_TELL */
#define call_tell(fd) syscall1(SYS_TELL, (scarg)fd)

/** @copydoc SYS_SEEK */
#define call_seek(fd, offset, whence) syscall3(SYS_SEEK, (scarg)fd, (scarg)offset, (scarg)whence)

/* Process Control */

/** @copydoc SYS_EXIT */
#define call_exit(status) syscall1(SYS_EXIT, (scarg)status)

/** @copydoc SYS_FORK */
#define call_fork() syscall0(SYS_FORK)

/** @copydoc SYS_EXECVE */
#define call_execve(pathname, argv, envp) syscall3(SYS_EXECVE, (scarg)pathname, (scarg)argv, (scarg)envp)

/** @copydoc SYS_GETPID */
#define call_getpid() syscall0(SYS_GETPID)

/** @copydoc SYS_GETPPID */
#define call_getppid() syscall0(SYS_GETPPID)

/** @copydoc SYS_WAITPID */
#define call_waitpid(pid, wstatus, options) syscall3(SYS_WAITPID, (scarg)pid, (scarg)wstatus, (scarg)options)

/** @copydoc SYS_KILL */
#define call_kill(pid, sig) syscall2(SYS_KILL, (scarg)pid, (scarg)sig)

/** @copydoc SYS_PRCTL */
#define call_prctl(option, arg1, arg2, arg3, arg4) syscall5(SYS_PRCTL, (scarg)option, (scarg)arg1, (scarg)arg2, (scarg)arg3, (scarg)arg4)

/* Memory */

/** @copydoc SYS_BRK */
#define call_brk(end_data) syscall1(SYS_BRK, (scarg)end_data)

/** @copydoc SYS_MMAP */
#define call_mmap(addr, length, prot, flags, fd, offset) syscall6(SYS_MMAP, (scarg)addr, (scarg)length, (scarg)prot, (scarg)flags, (scarg)fd, (scarg)offset)

/** @copydoc SYS_MUNMAP */
#define call_munmap(addr, length) syscall2(SYS_MUNMAP, (scarg)addr, (scarg)length)

/** @copydoc SYS_MPROTECT */
#define call_mprotect(addr, length, prot) syscall3(SYS_MPROTECT, (scarg)addr, (scarg)length, (scarg)prot)

/** @copydoc SYS_MADVISE */
#define call_madvise(addr, length, advice) syscall3(SYS_MADVISE, (scarg)addr, (scarg)length, (scarg)advice)

/* Communication */

/** @copydoc SYS_PIPE */
#define call_pipe(pipefd) syscall1(SYS_PIPE, (scarg)pipefd)

/** @copydoc SYS_DUP */
#define call_dup(oldfd) syscall1(SYS_DUP, (scarg)oldfd)

/** @copydoc SYS_DUP2 */
#define call_dup2(oldfd, newfd) syscall2(SYS_DUP2, (scarg)oldfd, (scarg)newfd)

/** @copydoc SYS_SOCKET */
#define call_socket(domain, type, protocol) syscall3(SYS_SOCKET, (scarg)domain, (scarg)type, (scarg)protocol)

/** @copydoc SYS_BIND */
#define call_bind(sockfd, addr, addrlen) syscall3(SYS_BIND, (scarg)sockfd, (scarg)addr, (scarg)addrlen)

/** @copydoc SYS_CONNECT */
#define call_connect(sockfd, addr, addrlen) syscall3(SYS_CONNECT, (scarg)sockfd, (scarg)addr, (scarg)addrlen)

/** @copydoc SYS_LISTEN */
#define call_listen(sockfd, backlog) syscall2(SYS_LISTEN, (scarg)sockfd, (scarg)backlog)

/** @copydoc SYS_ACCEPT */
#define call_accept(sockfd, addr, addrlen) syscall3(SYS_ACCEPT, (scarg)sockfd, (scarg)addr, (scarg)addrlen)

/** @copydoc SYS_SEND */
#define call_send(sockfd, buf, len, flags) syscall4(SYS_SEND, (scarg)sockfd, (scarg)buf, (scarg)len, (scarg)flags)

/** @copydoc SYS_RECV */
#define call_recv(sockfd, buf, len, flags) syscall4(SYS_RECV, (scarg)sockfd, (scarg)buf, (scarg)len, (scarg)flags)

/** @copydoc SYS_SHUTDOWN */
#define call_shutdown(sockfd, how) syscall2(SYS_SHUTDOWN, (scarg)sockfd, (scarg)how)

/* Time */

/** @copydoc SYS_TIME */
#define call_time(t) syscall1(SYS_TIME, t)

/** @copydoc SYS_CLOCK_GETTIME */
#define call_clock_gettime(clockid, tp) syscall2(SYS_CLOCK_GETTIME, (scarg)clockid, (scarg)tp)

/** @copydoc SYS_CLOCK_SETTIME */
#define call_clock_settime(clockid, tp) syscall2(SYS_CLOCK_SETTIME, (scarg)clockid, (scarg)tp)

/** @copydoc SYS_NANOSLEEP */
#define call_nanosleep(req, rem) syscall2(SYS_NANOSLEEP, (scarg)req, (scarg)rem)

/* Miscellaneous */

/** @copydoc SYS_GETCWD */
#define call_getcwd(buf, size) syscall2(SYS_GETCWD, (scarg)buf, (scarg)size)

/** @copydoc SYS_CHDIR */
#define call_chdir(path) syscall1(SYS_CHDIR, (scarg)path)

/** @copydoc SYS_MKDIR */
#define call_mkdir(path, mode) syscall2(SYS_MKDIR, (scarg)path, (scarg)mode)

/** @copydoc SYS_RMDIR */
#define call_rmdir(path) syscall1(SYS_RMDIR, (scarg)path)

/** @copydoc SYS_UNLINK */
#define call_unlink(pathname) syscall1(SYS_UNLINK, (scarg)pathname)

/** @copydoc SYS_RENAME */
#define call_rename(oldpath, newpath) syscall2(SYS_RENAME, (scarg)oldpath, (scarg)newpath)

#endif // !__FENNIX_API_SYSCALLS_LIST_H__
