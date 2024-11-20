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

#ifndef __FENNIX_DRIVER_API_H__
#define __FENNIX_DRIVER_API_H__

/**
 * @brief The driver API is a set of functions that the kernel provides to the drivers.
 *
 * - The driver is responsible for the memory management.
 * - The kernel will NOT free any memory allocated by the driver. On @see StopReason the driver must free all the memory it allocated and disable the hardware it uses.
 * - The driver image will be freed after the driver is unloaded.
 * - The kernel will unbind the interrupt handlers and the process handlers.
 * - Kernel API will be freed after the driver is unloaded.
 *
 */

enum DriverReturnCode
{
    ERROR,
    OK,
    NOT_IMPLEMENTED,
    NOT_FOUND,
    NOT_READY,
    NOT_AVAILABLE,
    NOT_AUTHORIZED,
    NOT_VALID,
    NOT_ACCEPTED,
    INVALID_PCI_BAR,
    INVALID_KERNEL_API,
    INVALID_MEMORY_ALLOCATION,
    INVALID_DATA,
    DEVICE_NOT_SUPPORTED,
    SYSTEM_NOT_SUPPORTED,
    KERNEL_API_VERSION_NOT_SUPPORTED,
};

enum DriverBindType
{
    BIND_NULL,
    BIND_INTERRUPT,
    BIND_PROCESS,
    BIND_PCI,
    BIND_INPUT
};

struct KernelAPI
{
    struct KAPIVersion
    {
        int Major;
        int Minor;
        int Patch;
    } Version;

    struct KAPIInfo
    {
        unsigned long Offset;
        unsigned long DriverUID;
        char KernelDebug;
    } Info;

    struct KAPIMemory
    {
        unsigned long PageSize;
        void *(*RequestPage)(unsigned long Size);
        void (*FreePage)(void *Page, unsigned long Size);
        void (*Map)(void *VirtualAddress, void *PhysicalAddress, unsigned long Flags);
        void (*Unmap)(void *VirtualAddress);
    } Memory;

    struct KAPIPCI
    {
        char *(*GetDeviceName)(unsigned int VendorID, unsigned int DeviceID);
    } PCI;

    struct KAPIUtilities
    {
        void (*DebugPrint)(char *String, unsigned long DriverUID);
        void (*DisplayPrint)(char *Value);
        void *(*memcpy)(void *Destination, void *Source, unsigned long Size);
        void *(*memset)(void *Destination, int Value, unsigned long Size);
        void (*Sleep)(unsigned long Milliseconds);
        int (*sprintf)(char *Buffer, const char *Format, ...);
    } Util;

    struct KAPIDriverTalk
    {
        /** @brief Connects to the network manager */
        struct
        {
            void (*SendPacket)(unsigned int DriverID, unsigned char *Data, unsigned short Size);
            void (*ReceivePacket)(unsigned int DriverID, unsigned char *Data, unsigned short Size);
        } Network;

        /** @brief Connects to the disk manager */
        struct
        {
            struct
            {
                void (*ReadSector)(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port);
                void (*WriteSector)(unsigned int DriverID, unsigned long Sector, unsigned char *Data, unsigned int SectorCount, unsigned char Port);
            } AHCI;
        } Disk;
    } Command;

    struct KAPIDisplay
    {
        unsigned int (*GetWidth)(void);
        unsigned int (*GetHeight)(void);
        /* TODO: Add more */
    } Display;
} __attribute__((packed));

enum CallbackReason
{
    /**
     * @brief This is used to detect memory corruption, not used.
     */
    UnknownReason,

    /**
     * @brief This is called once the kernel is ready to use the driver and call @see ConfigurationReason .
     */
    AcknowledgeReason,

    /**
     * @brief This is used after the driver is loaded and the kernel is ready to use the driver.
     *
     * For PCI drivers, @see RawPtr will be the PCI device address.
     */
    ConfigurationReason,

    /**
     * @brief This is used when the kernel wants to stop the driver.
     *
     * The memory allocated by the driver will be freed automatically.
     */
    StopReason,

    ProcessReason,
    InputReason,

    /* Kernel reserved callbacks. */
    /* ------------------------------------------------------- */
    /* Driver callbacks for basic usage. */

    /**
     * @brief This is used when the kernel sends data.
     *
     * - Network
     *   - Packet
     * - Audio
     *   - PCM Data
     */
    SendReason,

    /**
     * @brief This is used when the kernel wants to receive data.
     * Currently not used.
     */
    ReceiveReason,

    /**
     * @brief This is used to adjust driver settings.
     *
     * - Audio
     *   - Volume
     *   - PCM Encoding
     */
    AdjustReason,

    /**
     * @brief This is used when the kernel wants to fetch information about the driver.
     *
     * - Input
     *   - Mouse
     *     - Position
     *     - Buttons
     *   - Keyboard
     *     - Key
     */
    FetchReason,
};

union KernelCallback
{
    struct
    {
        CallbackReason Reason;
        void *RawPtr;
        unsigned long RawData;

        /** @brief When the kernel wants to send a packet. */
        struct
        {
            struct
            {
                unsigned char *Data;
                unsigned long Length;
            } Send;

            struct
            {
                char Name[128];
                unsigned long MAC;
            } Fetch;
        } NetworkCallback;

        /** @brief When the kernel wants to write to disk. */
        struct
        {
            struct
            {
                unsigned long Sector;
                unsigned long SectorCount;
                unsigned char Port;
                unsigned char *Buffer;
                bool Write;
            } RW;

            struct
            {
                unsigned char Ports;
                int BytesPerSector;
            } Fetch;
        } DiskCallback;

        /** @brief When the kernel wants to get mouse position / keyboard key */
        struct
        {
            struct
            {
                unsigned long X;
                unsigned long Y;
                unsigned long Z;
                struct
                {
                    bool Left;
                    bool Right;
                    bool Middle;
                } Buttons;
            } Mouse;
        } InputCallback;

        struct
        {
            struct
            {
                bool _Volume;
                bool _Encoding;
                bool _SampleRate;
                bool _Channels;

                /**
                 * @brief Adjust the volume.
                 *
                 * 0 - 100
                 */
                unsigned char Volume;

                /**
                 * @brief Adjust the encoding.
                 *
                 * 0 - None, use default
                 *
                 * 1 - Signed PCM 8-bit
                 * 2 - Unsigned PCM 8-bit
                 *
                 * 3 - Signed PCM 16-bit Little Endian
                 * 4 - Signed PCM 20-bit Little Endian
                 * 5 - Signed PCM 24-bit Little Endian
                 * 6 - Signed PCM 32-bit Little Endian
                 *
                 * 7 - Unsigned PCM 16-bit Little Endian
                 * 8 - Unsigned PCM 20-bit Little Endian
                 * 9 - Unsigned PCM 24-bit Little Endian
                 * 10 - Unsigned PCM 32-bit Little Endian
                 *
                 * 11 - Signed PCM 16-bit Big Endian
                 * 12 - Signed PCM 20-bit Big Endian
                 * 13 - Signed PCM 24-bit Big Endian
                 * 14 - Signed PCM 32-bit Big Endian
                 *
                 * 15 - Unsigned PCM 16-bit Big Endian
                 * 16 - Unsigned PCM 20-bit Big Endian
                 * 17 - Unsigned PCM 24-bit Big Endian
                 * 18 - Unsigned PCM 32-bit Big Endian
                 *
                 * 19 - Float PCM 32-bit Little Endian
                 * 20 - Float PCM 64-bit Little Endian
                 *
                 * 21 - Float PCM 32-bit Big Endian
                 * 22 - Float PCM 64-bit Big Endian
                 *
                 * 23 - PCM A-law
                 * 24 - PCM Mu-law
                 *
                 * ... - More
                 */
                unsigned short Encoding;

                /**
                 * @brief Adjust the sample rate.
                 *
                 * 0 - 8000 Hz
                 * 1 - 11025 Hz
                 * 2 - 16000 Hz
                 * 3 - 22050 Hz
                 * 4 - 32000 Hz
                 * 5 - 44100 Hz
                 * 6 - 48000 Hz
                 * 7 - 88200 Hz
                 * 8 - 96000 Hz
                 */
                unsigned char SampleRate;

                /**
                 * @brief Adjust the channels.
                 *
                 * 0 - Mono
                 * 1 - Stereo
                 */
                unsigned char Channels;
            } Adjust;

            struct
            {
                unsigned char *Data;
                unsigned long Length;
            } Send;

            struct
            {
                unsigned char Volume;
                unsigned short Encoding;
                unsigned char SampleRate;
                unsigned char Channels;
            } Fetch;
        } AudioCallback;

        struct
        {
            unsigned char Vector;
        } InterruptInfo;
    };
    unsigned long raw;
} __attribute__((packed));

union CPURegisters
{
    struct
    {
#if defined(__x86_64__) || defined(__amd64__)
        unsigned long r15;
        unsigned long r14;
        unsigned long r13;
        unsigned long r12;
        unsigned long r11;
        unsigned long r10;
        unsigned long r9;
        unsigned long r8;

        unsigned long rbp;
        unsigned long rdi;
        unsigned long rsi;
        unsigned long rdx;
        unsigned long rcx;
        unsigned long rbx;
        unsigned long rax;

        unsigned long InterruptNumber;
        unsigned long ErrorCode;
        unsigned long rip;
        unsigned long cs;
        unsigned long rflags;
        unsigned long rsp;
        unsigned long ss;
#elif defined(__i386__)
        unsigned int ebp;
        unsigned int edi;
        unsigned int esi;
        unsigned int edx;
        unsigned int ecx;
        unsigned int ebx;
        unsigned int eax;

        unsigned int InterruptNumber;
        unsigned int ErrorCode;
        unsigned int eip;
        unsigned int cs;
        unsigned int eflags;
        unsigned int esp;
        unsigned int ss;
#else
#warning "Unsupported architecture"
#endif
    };
    unsigned long raw;
} __attribute__((packed));

#endif // !__FENNIX_DRIVER_API_H__
