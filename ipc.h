#ifndef __FENNIX_KERNEL_IPC_SYSCALLS_H__
#define __FENNIX_KERNEL_IPC_SYSCALLS_H__

enum IPCCommand
{
    IPC_NULL,

    /**
     * @brief Create a new IPC
     * Creating a new IPC will return a new ID for the IPC.
     *
     * @return The ID of the new IPC.
     */
    IPC_CREATE,

    /**
     * @brief Read from IPC
     * This will read from an IPC.
     */
    IPC_READ,

    /**
     * @brief Write to IPC
     * This will write to an IPC.
     * @see Flags is used as process ID.
     */
    IPC_WRITE,

    IPC_DELETE,

    /**
     * @brief Wait for an IPC to be ready
     * This will wait for an IPC to be ready to read/write.
     * If it's message passing, it will wait for a message to be received.
     */
    IPC_WAIT,

    /**
     * @brief Listen to a IPC
     * @see Flags is used as a boolean
     */
    IPC_LISTEN,
};

/* This must be a clone of IPCType inside ipc.hpp */
enum IPCType
{
    IPC_TYPE_None,

    /**
     * @brief Message Passing
     * Message passing is a way to send messages between processes.
     *
     */
    IPC_TYPE_MessagePassing,
    IPC_TYPE_Port,
    IPC_TYPE_SharedMemory,
    IPC_TYPE_Pipe,
    IPC_TYPE_Socket
};

/* This must be a clone of IPCErrorCode inside ipc.hpp */
enum IPCErrorCode
{
    IPC_E_CODE_Error = -1,
    IPC_E_CODE_Success,
    IPC_E_CODE_NotListening,
    IPC_E_CODE_Timeout,
    IPC_E_CODE_InvalidCommand,
    IPC_E_CODE_AlreadyAllocated,
    IPC_E_CODE_NotAllocated,
    IPC_E_CODE_IDInUse,
    IPC_E_CODE_IDNotRegistered,
    IPC_E_CODE_IDNotFound,
};

#endif // !__FENNIX_KERNEL_IPC_SYSCALLS_H__
