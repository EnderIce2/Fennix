#ifndef __FENNIX_KERNEL_IPC_SYSCALLS_H__
#define __FENNIX_KERNEL_IPC_SYSCALLS_H__

enum IPCCommand
{
    IPC_NULL,
    IPC_CREATE,
    IPC_READ,
    IPC_WRITE,
    IPC_DELETE,
    IPC_GET,
    IPC_SET,
    IPC_GET_COUNT,
    IPC_GET_SIZE,
    IPC_GET_FLAGS,
    IPC_SET_FLAGS,
    IPC_GET_OWNER,
    IPC_SET_OWNER,
    IPC_GET_GROUP,
    IPC_SET_GROUP,
    IPC_GET_MODE,
    IPC_SET_MODE,
    IPC_GET_NAME,
    IPC_SET_NAME,
    IPC_GET_TYPE,
    IPC_SET_TYPE,
    IPC_GET_ID,
    IPC_SET_ID,
    IPC_GET_INDEX,
    IPC_SET_INDEX,
};

enum IPCType
{
    IPC_TYPE_None,
    IPC_TYPE_MessagePassing,
    IPC_TYPE_Port,
    IPC_TYPE_SharedMemory,
    IPC_TYPE_Pipe,
    IPC_TYPE_Socket
};

enum IPCErrorCode
{
    IPC_E_CODE_Error = -1,
    IPC_E_CODE_Success,
    IPC_E_CODE_NotListening,
    IPC_E_CODE_Timeout,
    IPC_E_CODE_InvalidPort,
    IPC_E_CODE_AlreadyAllocated,
    IPC_E_CODE_NotAllocated,
    IPC_E_CODE_IDInUse,
    IPC_E_CODE_IDNotRegistered,
    IPC_E_CODE_IDNotFound
};

#endif // !__FENNIX_KERNEL_IPC_SYSCALLS_H__
