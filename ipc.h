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
