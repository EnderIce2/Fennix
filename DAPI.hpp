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
 * The driver API is a set of functions that the kernel provides to the drivers.
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
		__UINT64_TYPE__ Offset;
		__UINT32_TYPE__ DriverUID;
		char KernelDebug;
	} Info;

	struct KAPIMemory
	{
		__UINT64_TYPE__ PageSize;
		void *(*RequestPage)(__UINT64_TYPE__ Size);
		void (*FreePage)(void *Page, __UINT64_TYPE__ Size);
		void (*Map)(void *VirtualAddress, void *PhysicalAddress, __UINT64_TYPE__ Flags);
		void (*Unmap)(void *VirtualAddress);
	} Memory;

	struct KAPIPCI
	{
		char *(*GetDeviceName)(__UINT32_TYPE__ VendorID, __UINT32_TYPE__ DeviceID);
	} PCI;

	struct KAPIUtilities
	{
		void (*DebugPrint)(char *String, __UINT64_TYPE__ DriverUID);
		void (*DisplayPrint)(char *Value);
		void *(*memcpy)(void *Destination, void *Source, __UINT64_TYPE__ Size);
		void *(*memset)(void *Destination, int Value, __UINT64_TYPE__ Size);
		void (*Sleep)(__UINT64_TYPE__ Milliseconds);
		int (*sprintf)(char *Buffer, const char *Format, ...);
	} Util;

	struct KAPIDriverTalk
	{
		/** Connects to the network manager */
		struct
		{
			void (*SendPacket)(__UINT32_TYPE__ DriverID, __UINT8_TYPE__ *Data, __UINT16_TYPE__ Size);
			void (*ReceivePacket)(__UINT32_TYPE__ DriverID, __UINT8_TYPE__ *Data, __UINT16_TYPE__ Size);
		} Network;

		/** Connects to the disk manager */
		struct
		{
			struct
			{
				void (*ReadSector)(__UINT32_TYPE__ DriverID, __UINT64_TYPE__ Sector, __UINT8_TYPE__ *Data, __UINT32_TYPE__ SectorCount, __UINT8_TYPE__ Port);
				void (*WriteSector)(__UINT32_TYPE__ DriverID, __UINT64_TYPE__ Sector, __UINT8_TYPE__ *Data, __UINT32_TYPE__ SectorCount, __UINT8_TYPE__ Port);
			} AHCI;
		} Disk;
	} Command;

	struct KAPIDisplay
	{
		__UINT32_TYPE__ (*GetWidth)(void);
		__UINT32_TYPE__ (*GetHeight)(void);
		/* TODO: Add more */
	} Display;
} __attribute__((packed));

enum CallbackReason
{
	/**
	 * This is used to detect memory corruption, not used.
	 */
	UnknownReason,

	/**
	 * This is called once the kernel is ready to use the driver and call @see ConfigurationReason .
	 */
	AcknowledgeReason,

	/**
	 * This is used after the driver is loaded and the kernel is ready to use the driver.
	 *
	 * For PCI drivers, @see RawPtr will be the PCI device address.
	 */
	ConfigurationReason,

	/**
	 * This is used when the kernel wants to stop the driver.
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
	 * This is used when the kernel sends data.
	 *
	 * - Network
	 *   - Packet
	 * - Audio
	 *   - PCM Data
	 */
	SendReason,

	/**
	 * This is used when the kernel wants to receive data.
	 * Currently not used.
	 */
	ReceiveReason,

	/**
	 * This is used to adjust driver settings.
	 *
	 * - Audio
	 *   - Volume
	 *   - PCM Encoding
	 */
	AdjustReason,

	/**
	 * This is used when the kernel wants to query information about the driver.
	 *
	 * - Input
	 *   - Mouse
	 *     - Position
	 *     - Buttons
	 *   - Keyboard
	 *     - Key
	 */
	QueryReason,

	/**
	 * This is used when the kernel wants to wait for an event.
	 *
	 * - Input
	 *   - Mouse
	 *     - Position
	 *     - Buttons
	 *   - Keyboard
	 *     - Key
	 */
	PollWaitReason,
};

union KernelCallback
{
	struct
	{
		CallbackReason Reason;
		void *RawPtr;
		__UINT64_TYPE__ RawData;

		/** When the kernel wants to send a packet. */
		struct
		{
			struct
			{
				__UINT8_TYPE__ *Data;
				__UINT64_TYPE__ Length;
			} Send;

			struct
			{
				char Name[128];
				__UINT64_TYPE__ MAC;
			} Fetch;
		} NetworkCallback;

		/** When the kernel wants to write to disk. */
		struct
		{
			struct
			{
				__UINT64_TYPE__ Sector;
				__UINT64_TYPE__ SectorCount;
				__UINT8_TYPE__ Port;
				__UINT8_TYPE__ *Buffer;
				bool Write;
			} RW;

			struct
			{
				__UINT8_TYPE__ Ports;
				int BytesPerSector;
			} Fetch;
		} DiskCallback;

		/** When the kernel wants to get mouse position / keyboard key */
		struct
		{
			struct
			{
				__UINT64_TYPE__ X;
				__UINT64_TYPE__ Y;
				__UINT64_TYPE__ Z;
				struct
				{
					bool Left;
					bool Right;
					bool Middle;
				} Buttons;
			} Mouse;

			struct
			{
				/**
				 * The key.
				 *
				 * @note This is a scancode, not a character.
				 */
				__UINT8_TYPE__ Key;
			} Keyboard;
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
				 * Adjust the volume.
				 *
				 * 0 - 100
				 */
				__UINT8_TYPE__ Volume;

				/**
				 * Adjust the encoding.
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
				__UINT16_TYPE__ Encoding;

				/**
				 * Adjust the sample rate.
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
				__UINT8_TYPE__ SampleRate;

				/**
				 * Adjust the channels.
				 *
				 * 0 - Mono
				 * 1 - Stereo
				 */
				__UINT8_TYPE__ Channels;
			} Adjust;

			struct
			{
				__UINT8_TYPE__ *Data;
				__UINT64_TYPE__ Length;
			} Send;

			struct
			{
				__UINT8_TYPE__ Volume;
				__UINT16_TYPE__ Encoding;
				__UINT8_TYPE__ SampleRate;
				__UINT8_TYPE__ Channels;
			} Fetch;
		} AudioCallback;

		struct
		{
			__UINT8_TYPE__ Vector;
		} InterruptInfo;
	};
	__UINT64_TYPE__ raw;
} __attribute__((packed));

union CPURegisters
{
	struct
	{
#if defined(__x86_64__) || defined(__amd64__)
		__UINT64_TYPE__ r15;
		__UINT64_TYPE__ r14;
		__UINT64_TYPE__ r13;
		__UINT64_TYPE__ r12;
		__UINT64_TYPE__ r11;
		__UINT64_TYPE__ r10;
		__UINT64_TYPE__ r9;
		__UINT64_TYPE__ r8;

		__UINT64_TYPE__ rbp;
		__UINT64_TYPE__ rdi;
		__UINT64_TYPE__ rsi;
		__UINT64_TYPE__ rdx;
		__UINT64_TYPE__ rcx;
		__UINT64_TYPE__ rbx;
		__UINT64_TYPE__ rax;

		__UINT64_TYPE__ InterruptNumber;
		__UINT64_TYPE__ ErrorCode;
		__UINT64_TYPE__ rip;
		__UINT64_TYPE__ cs;
		__UINT64_TYPE__ rflags;
		__UINT64_TYPE__ rsp;
		__UINT64_TYPE__ ss;
#elif defined(__i386__)
		__UINT32_TYPE__ ebp;
		__UINT32_TYPE__ edi;
		__UINT32_TYPE__ esi;
		__UINT32_TYPE__ edx;
		__UINT32_TYPE__ ecx;
		__UINT32_TYPE__ ebx;
		__UINT32_TYPE__ eax;

		__UINT32_TYPE__ InterruptNumber;
		__UINT32_TYPE__ ErrorCode;
		__UINT32_TYPE__ eip;
		__UINT32_TYPE__ cs;
		__UINT32_TYPE__ eflags;
		__UINT32_TYPE__ esp;
		__UINT32_TYPE__ ss;
#else
#warning "Unsupported architecture"
#endif
	};
	__UINT64_TYPE__ raw;
} __attribute__((packed));

#endif // !__FENNIX_DRIVER_API_H__
