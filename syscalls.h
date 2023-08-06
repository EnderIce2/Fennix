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

#include <stddef.h>

#define LTS_SET_GS 0x1
#define LTS_SET_FS 0x2
#define LTS_GET_FS 0x3
#define LTS_GET_GS 0x4
#define LTS_SET_CPUID 0x5
#define LTS_GET_CPUID 0x6

typedef enum
{
	MAP_PRESENT = 1 << 0,
	MAP_WRITABLE = 1 << 1,
	MAP_USER = 1 << 2,
} MemoryMapFlags;

typedef enum
{
	SYSCALL_SEEK_SET = 0,
	SYSCALL_SEEK_CUR = 1,
	SYSCALL_SEEK_END = 2
} FileSeekMode;

typedef enum
{
	/**
	 * Print a string to the screen using KPrint
	 *
	 * The uptime is prepended to the
	 * string before printing.
	 *
	 * Arguments:
	 * Arg1 - String to print
	 * Arg2 - Length of string
	 */
	KCTL_PRINT = 0,

	/**
	 * Get the page size
	 */
	KCTL_GET_PAGE_SIZE,

	/**
	 * Check whether the current
	 * thread is critical
	 */
	KCTL_IS_CRITICAL,
} KCtl;

/**
 * Enumeration of all the native syscalls
 * available in the kernel
 */
typedef enum
{
	/**
	 *
	 * Basic syscalls
	 *
	 */

	/**
	 * This syscall is used to exit the current
	 * process with the provided exit code.
	 *
	 * @param Code The exit code
	 * @return This syscall does not return
	 *
	 * @note No permissions are required to call
	 * this syscall
	 */
	sys_Exit = 0,

	/**
	 *
	 * Memory syscalls
	 *
	 */

	/**
	 * This syscall is used to request a specific
	 * number of pages of memory from the kernel.
	 *
	 * @param Count The number of pages to request
	 * @return The address of the first page of
	 * memory that was requested
	 *
	 * @note No permissions are required to call
	 * this syscall
	 */
	sys_RequestPages,

	/**
	 * This syscall is used to free a specific
	 * number of pages of memory that were
	 * previously requested.
	 *
	 * @param Address The address of the first
	 * page of memory to free
	 * @param Count The number of pages to free
	 * @return 0 on success, errno on failure
	 *
	 * @note No permissions are required to call
	 * this syscall
	 */
	sys_FreePages,

	/**
	 * This syscall is used to detach a specific
	 * memory address from the current process.
	 * This means that the address will no longer
	 * be freed when the process exits.
	 *
	 * @param Address The address to detach
	 * @return 0 on success, errno on failure
	 *
	 * @note The process must be trusted by the
	 * kernel to call this syscall
	 */
	sys_DetachAddress,

	/**
	 * This syscall is used to map a specific
	 * memory address to the current process.
	 *
	 * @param VirtualAddress The virtual address
	 * to map
	 * @param PhysicalAddress The physical address
	 * to map
	 * @param Size The size of the memory region
	 * to map
	 * @param Flags The flags to use when mapping
	 * the memory region (see MemoryMapFlags)
	 * @return 0 on success, errno on failure
	 *
	 * @note The process must be trusted by the
	 * kernel to call this syscall
	 */
	sys_MemoryMap,

	/**
	 * This syscall is used to unmap a specific
	 * memory address from the current process.
	 *
	 * @param VirtualAddress The virtual address
	 * to unmap
	 * @param Size The size of the memory region
	 * to unmap
	 * @return 0 on success, errno on failure
	 *
	 * @note The process must be trusted by the
	 * kernel to call this syscall
	 */
	sys_MemoryUnmap,

	/**
	 *
	 * Kernel Control syscalls
	 *
	 */

	/**
	 * Kernel Control
	 *
	 * This syscall is used to control certain
	 * aspects of the kernel or get information
	 * about it.
	 *
	 * @param Command The command to execute
	 * @param Arg1 The first argument
	 * @param Arg2 The second argument
	 * @param Arg3 The third argument
	 * @param Arg4 The fourth argument
	 * @return The result of the command, or
	 * errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_KernelCTL,

	/**
	 *
	 * File syscalls
	 *
	 */

	/**
	 * This syscall is used to open a file with
	 * the provided path and flags.
	 *
	 * @param Path The path to the file to open
	 * @param Flags The flags to use when opening
	 * the file
	 * @param Mode The mode to use when opening
	 * the file
	 * @return The file descriptor of the opened
	 * file, or errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileOpen,

	/**
	 * This syscall is used to close a file
	 * that was previously opened.
	 *
	 * @param FileDescriptor The file descriptor
	 * of the file to close
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileClose,

	/**
	 * This syscall is used to read a specific
	 * number of bytes from a file at a specific
	 * offset.
	 *
	 * @param FileDescriptor The file descriptor
	 * of the file to read from
	 * @param Buffer The buffer to read into
	 * @param Count The number of bytes to read
	 * @return The number of bytes read, or
	 * errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileRead,

	/**
	 * This syscall is used to write a specific
	 * number of bytes to a file at a specific
	 * offset.
	 *
	 * @param FileDescriptor The file descriptor
	 * of the file to write to
	 * @param Buffer The buffer to write from
	 * @param Count The number of bytes to write
	 * @return The number of bytes written, or
	 * errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileWrite,

	/**
	 * This syscall is used to change the current
	 * offset in a file.
	 *
	 * @param FileDescriptor The file descriptor
	 * of the file to seek in
	 * @param Offset The offset to seek to
	 * @param Whence The seek mode
	 * (see FileSeekMode)
	 * @return The new offset, or errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileSeek,

	/**
	 * This syscall is used to retrieve information
	 * about a file such as its size, permissions,
	 * etc.
	 *
	 * @param FileDescriptor The file descriptor
	 * of the file to get information about
	 * @param StatBuffer The buffer to store the
	 * information in
	 * @return 0 on success, errno on failure
	 *
	 * @note No permissions are required to
	 * call this syscall
	 */
	sys_FileStatus,

	/**
	 *
	 * Process syscalls
	 *
	 */

	/**
	 * Creates/Reads/Writes/Deletes an IPC Pipe/Shared Memory/Message Queue/etc.
	 *
	 * @fn int IPC(enum IPCCommand Command, enum IPCType Type, int ID, int Flags, void *Buffer, size_t Size)
	 * This syscall is used to create, read, write or delete an IPC Pipe/Shared Memory/Message Queue/etc.
	 */
	sys_IPC,

	/**
	 * Get/Set the local thread state
	 *
	 * @fn int LocalThreadStorage(int Code, unsigned long Address)
	 * This syscall is used to get or set the local thread state.
	 */
	sys_LocalThreadState,

	/**
	 * Sleep for a specific amount of time
	 *
	 * @fn int Sleep(uint64_t Milliseconds)
	 * This syscall is used to sleep the current thread for a specific amount of time.
	 */
	sys_Sleep,

	/**
	 * Fork the current process
	 *
	 * @fn int Fork()
	 * This syscall is used to create a new process that is a copy of the current process.
	 */
	sys_Fork,

	/**
	 * Wait for a process or a thread
	 *
	 * @fn
	 * This syscall is used to wait for a specific process or thread to terminate. It returns the exit code of the process or thread.
	 */
	sys_Wait,

	/**
	 * Kill a process or a thread
	 *
	 * @fn
	 * This syscall is used to send a termination signal to a specific process or thread
	 */
	sys_Kill,

	/**
	 * Spawn a new process
	 *
	 * @fn
	 * This syscall is used to create a new process with the provided path and arguments.
	 */
	sys_Spawn,

	/**
	 * Spawn a new thread
	 *
	 * @fn int SpawnThread(uint64_t InstructionPointer)
	 * This syscall is used to create a new thread within the current process with the provided function and arguments.
	 */
	sys_SpawnThread,

	/**
	 * Get thread list of a process
	 *
	 * @fn
	 * This syscall is used to retrieve a list of all the threads within a specific process.
	 */
	sys_GetThreadListOfProcess,

	/**
	 * Get current process
	 *
	 * @fn
	 * This syscall is used to retrieve information about the current process.
	 */
	sys_GetCurrentProcess,

	/**
	 * Get current thread
	 *
	 * @fn
	 * This syscall is used to retrieve information about the current thread.
	 */
	sys_GetCurrentThread,

	/**
	 * Get current process ID
	 *
	 * @fn int GetCurrentProcessID()
	 * This syscall is used to retrieve information about the current process.
	 */
	sys_GetCurrentProcessID,

	/**
	 * Get current thread ID
	 *
	 * @fn int GetCurrentThreadID()
	 * This syscall is used to retrieve information about the current thread.
	 */
	sys_GetCurrentThreadID,

	/**
	 * Get process by PID
	 *
	 * @fn
	 * This syscall is used to retrieve information about a specific process by its PID.
	 */
	sys_GetProcessByPID,

	/**
	 * Get thread by TID
	 *
	 * @fn
	 * This syscall is used to retrieve information about a specific thread by its TID.
	 */
	sys_GetThreadByTID,

	/**
	 * Kill a process
	 *
	 * @fn
	 * This syscall is used to send a termination signal to a specific process.
	 */
	sys_KillProcess,

	/**
	 * Kill a thread
	 *
	 * @fn
	 * This syscall is used to send a termination signal to a specific thread.
	 */
	sys_KillThread,

	/**
	 * Reserved syscall */

	sys_SysReservedCreateProcess,

	/**
	 * Reserved syscall */

	sys_SysReservedCreateThread,

	/** Not a real syscall */
	sys_MaxSyscall
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
