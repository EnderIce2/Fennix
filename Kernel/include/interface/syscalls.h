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
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall)
						 : "rcx", "r11", "memory");
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
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1)
						 : "rcx", "r11", "memory");
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
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2)
						 : "rcx", "r11", "memory");
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
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
						 : "rcx", "r11", "memory");
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
	register scarg r10 __asm__("r10") = arg4;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
						 : "rcx", "r11", "memory");
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
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
						 : "rcx", "r11", "memory");
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
	register scarg r10 __asm__("r10") = arg4;
	register scarg r8 __asm__("r8") = arg5;
	register scarg r9 __asm__("r9") = arg6;
	__asm__ __volatile__("syscall"
						 : "=a"(ret)
						 : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
						 : "rcx", "r11", "memory");
	return ret;
}

#pragma endregion Syscall Wrappers

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
} mmap_flags_t;

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
} open_flags_t;

typedef enum
{
	__SYS_F_OK = 0,
	__SYS_R_OK = 1,
	__SYS_W_OK = 2,
	__SYS_X_OK = 3
} access_flags_t;

typedef enum
{
	__SYS_GET_GS = 0,
	__SYS_SET_GS = 1,
	__SYS_GET_FS = 2,
	__SYS_SET_FS = 3,
} prctl_options_t;

typedef int __SYS_clockid_t;
typedef unsigned int __SYS_socklen_t;

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
	 * @param offset Offset in the file
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
	 * @param offset Offset in the file
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
	 *              - #O_RDONLY: Open file for reading only.
	 *              - #O_WRONLY: Open file for writing only.
	 *              - #O_RDWR: Open file for reading and writing.
	 *              - #O_APPEND: Append data to the end of file.
	 *              - #O_CREAT: Create file if it does not exist.
	 *              - #O_DSYNC:
	 *              - #O_EXCL:
	 *              - #O_NOCTTY:
	 *              - #O_NONBLOCK:
	 *              - #O_RSYNC:
	 *              - #O_SYNC:
	 *              - #O_TRUNC: Truncate file to zero length.
	 * @param mode Permissions for newly created file (if applicable)
	 *
	 * @return
	 * - File descriptor on success
	 * - #ENOENT if the file does not exist
	 * - #EACCES if permissions are insufficient
	 *
	 * @see #open_flags_t
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
	 * int ioctl(int fd, unsigned long request, ...);
	 * @endcode
	 *
	 * @details Manipulates the underlying parameters of a device.
	 *
	 * @param fd File descriptor referring to the device
	 * @param request Device-specific request code
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
	 *             - #F_OK: Check if the file exists
	 *             - #R_OK: Check if the file is readable
	 *             - #W_OK: Check if the file is writable
	 *             - #X_OK: Check if the file is executable
	 *
	 * @return
	 * - #EOK on success
	 * - #EACCES if access is denied
	 * - #ENOENT if the file does not exist
	 *
	 * @see #access_flags_t
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
	 * int prctl(prctl_options_t option, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);
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
	 *             - #PROT_READ: Readable
	 *             - #PROT_WRITE: Writable
	 *             - #PROT_EXEC: Executable
	 *             - #PROT_NONE: No access
	 * @param flags Mapping options\n
	 *             Supported values:
	 *             - #MAP_SHARED: Share memory with other processes
	 *             - #MAP_PRIVATE: Create a private copy of the file
	 *             - #MAP_FIXED: Use `addr` as the starting address of the mapping
	 *             - #MAP_ANONYMOUS: Create an anonymous mapping
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
	 * @see #mmap_flags_t
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
	 * - #NULL if `t` is NULL
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
	 * - #NULL on error
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

#endif // !__FENNIX_API_SYSCALLS_LIST_H__
