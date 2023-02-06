#ifndef __FENNIX_KERNEL_SYSCALLS_LIST_H__
#define __FENNIX_KERNEL_SYSCALLS_LIST_H__

#include <stddef.h>

/**
 * @enum NativeSyscalls
 * Enumeration of all the native syscalls available in the kernel
 */
enum NativeSyscalls
{
    /** @brief Exit the process.
     * @fn int Exit(int Code)
     * This syscall is used to exit the current process with the provided exit code.
     */
    _Exit = 0,
    /** @brief Print a message to the kernel console
     * @fn int Print(char Char, int Index)
     * This syscall is used to print a message to the kernel console.
     */
    _Print,

    /** @brief Request pages of memory
     * @fn uintptr_t RequestPages(size_t Count)
     * This syscall is used to request a specific number of pages of memory from the kernel.
     */
    _RequestPages,
    /** @brief Free pages of memory
     * @fn int FreePages(uintptr_t Address, size_t Count)
     * This syscall is used to free a specific number of pages of memory that were previously requested.
     */
    _FreePages,
    /** @brief Detach memory address
     * @fn int DetachAddress(uintptr_t Address)
     * This syscall is used to detach a specific memory address from the current process.
     */
    _DetachAddress,

    /** @brief Kernel Control
     * @fn uintptr_t KernelCTL(enum KCtl Command, uint64_t Arg1, uint64_t Arg2, uint64_t Arg3, uint64_t Arg4)
     * This syscall is used to control certain aspects of the kernel or get information about it.
     */
    _KernelCTL,

    /**
     * @brief Creates/Reads/Writes/Deletes an IPC Pipe/Shared Memory/Message Queue/etc.
     * @fn int IPC(enum IPCCommand Command, enum IPCType Type, int ID, int Flags, void *Buffer, size_t Size)
     * This syscall is used to create, read, write or delete an IPC Pipe/Shared Memory/Message Queue/etc.
     */
    _IPC,

    /** @brief Open a file
     * @fn
     * This syscall is used to open a file with the provided path and flags.
     */
    _FileOpen,
    /** @brief Close a file
     * @fn
     * This syscall is used to close a file that was previously opened.
     */
    _FileClose,
    /** @brief Read from a file
     * @fn
     * This syscall is used to read a specific number of bytes from a file at a specific offset.
     */
    _FileRead,
    /** @brief Write to a file
     * @fn
     * This syscall is used to write a specific number of bytes to a file at a specific offset.
     */
    _FileWrite,
    /** @brief Seek in a file
     * @fn
     * This syscall is used to change the current offset in a file.
     */
    _FileSeek,
    /** @brief Get file status
     * @fn
     * This syscall is used to retrieve information about a file such as its size, permissions, etc.
     */
    _FileStatus,

    /** @brief Wait for a process or a thread
     * @fn
     * This syscall is used to wait for a specific process or thread to terminate. It returns the exit code of the process or thread.
     */
    _Wait,
    /** @brief Kill a process or a thread
     * @fn
     * This syscall is used to send a termination signal to a specific process or thread
     */
    _Kill,
    /** @brief Spawn a new process
     * @fn
     * This syscall is used to create a new process with the provided path and arguments.
     */
    _Spawn,
    /** @brief Spawn a new thread
     * @fn
     * This syscall is used to create a new thread within the current process with the provided function and arguments.
     */
    _SpawnThread,
    /** @brief Get thread list of a process
     * @fn
     * This syscall is used to retrieve a list of all the threads within a specific process.
     */
    _GetThreadListOfProcess,
    /** @brief Get current process
     * @fn
     * This syscall is used to retrieve information about the current process.
     */
    _GetCurrentProcess,
    /** @brief Get current thread
     * @fn
     * This syscall is used to retrieve information about the current thread.
     */
    _GetCurrentThread,
    /** @brief Get process by PID
     * @fn
     * This syscall is used to retrieve information about a specific process by its PID.
     */
    _GetProcessByPID,
    /** @brief Get thread by TID
     * @fn
     * This syscall is used to retrieve information about a specific thread by its TID.
     */
    _GetThreadByTID,
    /** @brief Kill a process
     * @fn
     * This syscall is used to send a termination signal to a specific process.
     */
    _KillProcess,
    /** @brief Kill a thread
     * @fn
     * This syscall is used to send a termination signal to a specific thread.
     */
    _KillThread,
    /** @brief Reserved syscall */
    _SysReservedCreateProcess,
    /** @brief Reserved syscall */
    _SysReservedCreateThread,
};

/**
 * @enum SyscallsErrorCodes
 * Enumeration of all the error codes that can be returned by a syscall
 */
enum SyscallsErrorCodes
{
    /**
     * @brief Access denied
     * This error code is returned when the current thread does not have the required permissions to perform the requested operation.
     */
    SYSCALL_ACCESS_DENIED = -0xDEADACC,
    /**
     * @brief Invalid argument
     * This error code is returned when an invalid argument is passed to a syscall.
     */
    SYSCALL_INVALID_ARGUMENT = -0xBADAEE,
    /**
     * @brief Invalid syscall
     * This error code is returned when an invalid syscall number is passed to the syscall handler.
     */
    SYSCALL_INVALID_SYSCALL = -0xBAD55CA,
    /**
     * @brief Internal error
     * This error code is returned when an internal error occurs in the syscall handler.
     */
    SYSCALL_INTERNAL_ERROR = -0xBADBAD5,
    /**
     * @brief Not implemented
     * This error code is returned when a syscall is not implemented.
     */
    SYSCALL_NOT_IMPLEMENTED = -0xBAD5EED,
    /**
     * @brief Generic error
     * This error code is returned when a syscall fails for an unknown reason.
     */
    SYSCALL_ERROR = -1,
    /**
     * @brief Success
     * This error code is returned when a syscall succeeds.
     */
    SYSCALL_OK = 0,
};

static inline long syscall0(long syscall)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall1(long syscall, long arg1)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall2(long syscall, long arg1, long arg2)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall3(long syscall, long arg1, long arg2, long arg3)
{
    unsigned long ret;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall4(long syscall, long arg1, long arg2, long arg3, long arg4)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall5(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
                         : "rcx", "r11", "memory");
    return ret;
}

static inline long syscall6(long syscall, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
    unsigned long ret;
    register long r10 __asm__("r10") = arg4;
    register long r8 __asm__("r8") = arg5;
    register long r9 __asm__("r9") = arg6;
    __asm__ __volatile__("syscall"
                         : "=a"(ret)
                         : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
                         : "rcx", "r11", "memory");
    return ret;
}

#endif // !__FENNIX_KERNEL_SYSCALLS_LIST_H__
